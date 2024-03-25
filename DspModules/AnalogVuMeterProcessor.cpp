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
    x_1next.free();
    x_2next.free();
    x_3next.free();
    x_4next.free();
}

void AnalogVuMeterProcessor::prepareToPlay(double sampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock)
{
    spec.sampleRate =sampleRate;
    spec.numChannels = numberOfInputChannels;
    spec.maximumBlockSize = estimatedSamplesPerBlock;



    //for state space equation : 4 past values for "next init x0"
    x_1next.calloc(numberOfInputChannels);
    x_2next.calloc(numberOfInputChannels);
    x_3next.calloc(numberOfInputChannels);
    x_4next.calloc(numberOfInputChannels);

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


    for (int ch = 0; ch < spec.numChannels; ++ch)
    {
        //set matrices for system I - single channel #ch
        StateSpaceModelSimulation ssms_v2i;
        ssms_v2i.setCoefficients(ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_v2i_D, ssm_v2i_x0, ssm_v2i_x);
        
        //set matrix for system II - single channel #ch
        StateSpaceModelSimulation ssms_i2a;
        ssms_i2a.setCoefficients(ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_v2i_D, ssm_v2i_x0, ssm_v2i_x);

        ssmsVector_v2i.push_back(ssms_v2i);
        ssmsVector_i2a.push_back(ssms_i2a);
    }


}



void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer, std::vector<StateSpaceModelSimulation> &ssmsVector) //_buffer == rectified one
{

    //initialize everything
    const int numberOfChannels = buffer.getNumChannels();
    const int numberOfSamples = buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = this->sysDim;


    //                      **System I Rectifying **

    auto xs = ssms.convertBufferTo1dMatrix(buffer); //x is the input sequence
    auto x0s = ssms.convertBufferTo1dMatrix

    for (int channel = 0; channel < numberOfChannels ; ++channel) //stereo only first
    {
        mat x = xs.at(channel);
        mat x0 = 
        auto ssms = &ssmsVector.at(channel);
        
        ssms->setInputSequence(x0, x);
        //DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[0]));
    }

}



//==============================================================================



void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{

    //hard rectifying using matrices from system I
    bufferForMeasurement = buffer;
    feedToSteadyStateEquation(bufferForMeasurement, ssmsVector_v2i);
    DBG("End of System I.");

    feedToSteadyStateEquation(bufferForMeasurement, ssmsVector_i2a);
    DBG("End of System II.");

}




void AnalogVuMeterProcessor::reset()
{
    x_1next.clear(spec.numChannels);
    x_2next.clear(spec.numChannels);
    x_3next.clear(spec.numChannels);
    x_4next.clear(spec.numChannels);
}

juce::AudioBuffer<float> AnalogVuMeterProcessor::getOutputBuffer()
{
    return bufferForMeasurement;
}
