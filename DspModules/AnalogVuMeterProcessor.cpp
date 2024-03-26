/*
  ==============================================================================

    AnalogVuMeterProcessor.cpp
    Created: 19 Mar 2024 4:06:42pm
    Author:  orstphone@github.com
    statespace models simulation code ref :
    https://github.com/AleksandarHaber/Simulation-of-State-Space-Models-of-Dynamical-Systems-in-Cpp--Eigen-Matrix-Library-Tutorial/blob/master/SimulateSystem.cpp
  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalogVuMeterProcessor.h"
//#include "StateSpaceModelSimulation.h"
#include <cmath>
//static member constants

const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 120));

using mat = juce::dsp::Matrix<float>;

//==============================================================================
AnalogVuMeterProcessor::AnalogVuMeterProcessor() :
    ssm_v2i_x(1, 1),
    ssm_v2i_x0(1, 1),
    ssm_v2i_A(1, 1),
    ssm_v2i_B(1, 1),
    ssm_v2i_C(1, 1),
    ssm_v2i_D(1, 1),
    ssm_i2a_x(1, 1),
    ssm_i2a_x0(1, 1),
    ssm_i2a_A(1, 1),
    ssm_i2a_B(1, 1),
    ssm_i2a_C(1, 1),
    ssm_i2a_D(1, 1)
{
    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(48000.0, 2, 1024);
}

AnalogVuMeterProcessor::~AnalogVuMeterProcessor()
{
    z1.free();
    z2.free();
    z3.free();
    z4.free();
    w1.free();
    w2.free();
    w3.free();
    w4.free();
}

void AnalogVuMeterProcessor::prepareToPlay(double sampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock)
{
    spec.sampleRate =sampleRate;
    spec.numChannels = numberOfInputChannels;
    spec.maximumBlockSize = estimatedSamplesPerBlock;

    //create empty buffer following specs
    bufferForMeasurement.setSize(numberOfInputChannels, estimatedSamplesPerBlock);
    bufferForMeasurement.clear();


    //for state space equation : 4 past values for "next init x0"
    z1.calloc(numberOfInputChannels);
    z2.calloc(numberOfInputChannels);
    z3.calloc(numberOfInputChannels);
    z4.calloc(numberOfInputChannels);


    bufferForInitialStateSystemI.setSize(numberOfInputChannels, sysDim);
    for (int ch; ch < numberOfInputChannels; ++ch)
    {
        float* init = bufferForInitialStateSystemI.getWritePointer(ch);
        init[0] = z4[ch];
        init[1] = z3[ch];
        init[2] = z2[ch];
        init[3] = z1[ch];
    }

    bufferForInitialStateSystemII.setSize(numberOfInputChannels, sysDim);
    for (int ch; ch < numberOfInputChannels; ++ch)
    {
        float* init = bufferForInitialStateSystemII.getWritePointer(ch);
        init[0] = w4[ch];
        init[1] = w3[ch];
        init[2] = w2[ch];
        init[3] = w1[ch];
    }


    //matrices for system I (voltage to current)
    ssm_v2i_x = mat(sysDim, 1); ssm_v2i_x.clear(); //zeroize mono input
    ssm_v2i_x0 = mat(sysDim, 1); ssm_v2i_x0.clear(); //zeroize mono init cond
    ssm_v2i_A = ssms.convertArrayTo2dMatrix(v2i_A, sysDim, sysDim);
    ssm_v2i_B = ssms.convertArrayTo2dMatrix(v2i_B, 1, sysDim);
    ssm_v2i_C = ssms.convertArrayTo2dMatrix(v2i_C, sysDim, 1);
    ssm_v2i_D = ssms.convertArrayTo2dMatrix(v2i_D, 1, 1);

    //matrices for system II(current to galvanometer)
    ssm_i2a_x = mat(sysDim, 1); ssm_i2a_x.clear(); //zeroize mono input
    ssm_i2a_x0 = mat(sysDim, 1); ssm_i2a_x0.clear(); //zeroize mono init cond
    ssm_i2a_A = ssms.convertArrayTo2dMatrix(i2a_A, sysDim, sysDim);
    ssm_i2a_B = ssms.convertArrayTo2dMatrix(i2a_B, 1, sysDim);
    ssm_i2a_C = ssms.convertArrayTo2dMatrix(i2a_C, sysDim, 1);
    ssm_i2a_D = ssms.convertArrayTo2dMatrix(i2a_D, 1, 1);



    ssms_v2i.hardReset(ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_v2i_D, bufferForMeasurement, bufferForInitialStateSystemI, sysDim);

    ssms_i2a.hardReset(ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_v2i_D, bufferForMeasurement, bufferForInitialStateSystemII, sysDim);   
}


//virtually the main function
void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer, int systemSize) //_buffer == rectified one
{

    //initialize everything
    const int numberOfChannels = buffer.getNumChannels();
    const int numberOfSamples = buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = systemSize;

    //*********************************************************  System I Voltage To Current 
    ssms_v2i.setInitStateVector(bufferForInitialStateSystemI, sysDim);
    ssms_v2i.setInputSequence(buffer);

    for (int ch = 0; ch < numberOfChannels; ++ch)
    {
        //channel-wise simulation will be stored in the class
        ssms_v2i.runSimulation(ch);
    }

    //save prev 4 values for all channels
    auto& outputPostSystemI = ssms_v2i.getSimulatedOutputMatrix();

    keepPreviousStateForNextInitSystemI();

    DBG("System I ends here");

    //*********************************************************  System II Current to Needlepoint Angle
    //acquire result matrix vector y from SystemI 
    outputPostSystemI = ssms_v2i.getSimulatedOutputMatrix();

    //prep x and x0
    ssms_i2a.setInitStateVector(bufferForInitialStateSystemII, sysDim);
    ssms_i2a.setInputSequence(outputPostSystemI);

    for (int ch = 0; ch < numberOfChannels; ++ch)
    {
        //channel-wise simulation will be stored in the class
        ssms_i2a.runSimulation(ch);
    }

    outputPostSystemII = ssms_i2a.getSimulatedOutputMatrix();
    keepPreviousStateForNextInitSystemII(); //for next block; : prepareToPlay will be skipped next time.
    DBG("System II ends here");

}



void AnalogVuMeterProcessor::keepPreviousStateForNextInitSystemI()
{
    auto numChannels = bufferForInitialStateSystemI.getNumChannels();
    auto numSamples = bufferForInitialStateSystemI.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* init = bufferForInitialStateSystemI.getWritePointer(ch);
        init[0] = z4[ch];
        init[1] = z3[ch];
        init[2] = z2[ch];
        init[3] = z1[ch];
    }
}


void AnalogVuMeterProcessor::keepPreviousStateForNextInitSystemII()
{
    auto numChannels = bufferForInitialStateSystemII.getNumChannels();
    auto numSamples = bufferForInitialStateSystemII.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* init = bufferForInitialStateSystemII.getWritePointer(ch);
        init[0] = w4[ch];
        init[1] = w3[ch];
        init[2] = w2[ch];
        init[3] = w1[ch];
    }
}


//==============================================================================



void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{

    feedToSteadyStateEquation(buffer, sysDim);
    DBG("Single processBlock finished...");
}




void AnalogVuMeterProcessor::reset()
{
    auto numChannels = spec.numChannels;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        z1.clear(ch);
        z2.clear(ch);
        z3.clear(ch);
        z4.clear(ch);

        w1.clear(ch);
        w2.clear(ch);
        w3.clear(ch);
        w4.clear(ch);
    }

}

juce::AudioBuffer<float> AnalogVuMeterProcessor::getOutputBuffer()
{
    return bufferForMeasurement;
}
