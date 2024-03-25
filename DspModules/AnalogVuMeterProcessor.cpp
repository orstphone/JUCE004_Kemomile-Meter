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

#include <cmath>
//static member constants

const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 120));



//==============================================================================
AnalogVuMeterProcessor::AnalogVuMeterProcessor()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    //
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(48000.0, 2, 2048);

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
    //ssm_v2i_x will be initialized later
    ssm_v2i_A = ssms.convertArrayTo2dMatrix(v2i_A, sysDim, sysDim);
    ssm_v2i_B = ssms.convertArrayTo2dMatrix(v2i_B, 1, sysDim);
    ssm_v2i_C = ssms.convertArrayTo2dMatrix(v2i_C, sysDim, 1);
    ssm_v2i_D = ssms.convertArrayTo2dMatrix(v2i_D, 1, 1);

    //matrices for system II(current to galvanometer)
    //ssm_i2a_x will be initialized later
    ssm_i2a_A = ssms.convertArrayTo2dMatrix(i2a_A, sysDim, sysDim);
    ssm_i2a_B = ssms.convertArrayTo2dMatrix(i2a_B, 1, sysDim);
    ssm_i2a_C = ssms.convertArrayTo2dMatrix(i2a_C, sysDim, 1);
    ssm_i2a_D = ssms.convertArrayTo2dMatrix(i2a_D, 1, 1);

    //set matrix for system I
    ssmsLeft_v2i.setCoefficients(ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_V2i_D,

}


//==============================================================================
//custom built functions incl privates




void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& _buffer, mat& A, mat& B, mat& C, mat& D) //_buffer == rectified one
{

    //initialize everything
    const int numberOfChannels =_buffer.getNumChannels();
    const int numberOfSamples = _buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = this->sysDim;


    //                      **System I Rectifying **

    auto matrices = 

    for (int channel = 0; channel < numberOfChannels; ++channel)
    {
        float* x_ = _statebuffer.getWritePointer(channel); //initial state vector.
        float* x_next = _statebufferNext.getWritePointer(channel);
        float* u_ = _buffer.getWritePointer(channel); //input sequence.
        float* y_ = _outputbuffer.getWritePointer(channel); //output sequence.

        double sr = spec.sampleRate;

        //assign previous 3 values
        //_statebufferNext.clear();

        x_[0] = x_1next[channel];
        x_[1] = x_2next[channel];
        x_[2] = x_3next[channel];
        x_[3] = x_4next[channel];
            //x_[4] is the "first" element.

        //simulate the system
        ssm_i2a_X = juce::dsp::Matrix<float>(sysDim, 1);
        ssm_i2a_X.clear(); // O-matrix
        ssm_i2a_X_next = juce::dsp::Matrix<float>(sysDim, 1, x_);
        

        for (int i = 0; i < numberOfSamples + sysDim - 1; ++i)
        {
            //calc next state -> vector<float>
            getNextState(ssm_i2a_X, u_[i], ssm_i2a_X_next, sr); //is x_next audiobuffer or float? -> it's buffer == float*

            float y = getOutput(ssm_i2a_X, u_[i]);

            y = y > minimalReturnValue ? y : minimalReturnValue;

            y_[i] = float(y); //this is the "meter movement"
            ssm_i2a_X = ssm_i2a_X_next;
            DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[i]));

            //_bufferLastValue.at(channel) = float(ssmat_Y(0, 0));
        }


        // remembering old sysDim amt. values before evaporating
        x_1next[channel] = x_next[numberOfSamples - 1];
        x_2next[channel] = x_next[numberOfSamples - 2];
        x_3next[channel] = x_next[numberOfSamples - 3];
        x_4next[channel] = x_next[numberOfSamples - 4];


        //DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[0]));
    }

    //_outputbuffer will be assigned from somewhewr.
    //the buffer is declared in the header as private member.
}



//==============================================================================



void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{

    //hard rectifying using matrices from system I

    feedToSteadyStateEquation(buffer, ssm_v2i_A, ssm_v2i_B, ssm_v2i_C, ssm_v2i_D);
    DBG("rectifying done.");


    //STEP 2:: set the number of channels to prevent EXC_BAD_ACCESS
    //when the number of channels in the buffer suddenly changes w/o calling
    //prepareToPlay() in advance.

 
    //STEP 3:: accum. the samples and use the steady state equation.
    feedToSteadyStateEquation(buffer, ssm_i2a_A, ssm_i2a_B, ssm_i2a_C, ssm_i2a_D);
    DBG("SSE done. applied to _buffer directly.");

    //STEP 4:: throw every data in _outputbuffer to the "Vu Meter"
    //from here _outputbuffer will determine "needlepoint" of the meter. : "i(t)"
    //end of sequence
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
    return _outputbuffer;
}
