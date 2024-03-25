/*
  ==============================================================================

    AnalogVuMeterProcessor.h
    Created: 19 Mar 2024 4:06:42pm
    Author:  prmir

  ==============================================================================
*/

#pragma once
#ifndef __ANALOG_VU_METER_PROCESSOR__
#define __ANALOG_VU_METER_PROCESSOR__
#include <JuceHeader.h>
#include <vector>
//==============================================================================


class AnalogVuMeterProcessor  :
    public juce::Component
{
public:
    AnalogVuMeterProcessor();
    ~AnalogVuMeterProcessor() override;

    void prepareToPlay(double  SampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock); 


    void getNextState(const juce::dsp::Matrix<float>& X, const float& u, juce::dsp::Matrix<float>& X_next, double sampleRate);

    float getOutput(juce::dsp::Matrix<float>& X, const float u);

    void feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer);

    void processBlock(juce::AudioBuffer<float> &buffer);

    void reset();

    juce::AudioBuffer<float> getOutputBuffer();


    void generateVuMeterIR(int numberOfChannels, int numberOfSamples);



private:
    //keeping og b

    //for rectifying
    float capacitance;
    float resistance;
    float outputMagnitudeCalibration;
    juce::dsp::Convolution convolver;
    juce::dsp::ProcessSpec rectifierSpec;
    juce::AudioBuffer<float> vuMeterImpulseResponseBuffer;


    //state space equation matrices
    const size_t sysDim = 4;
    float elements_ssmA[16] = {
        -19.84,      8.746,     -39.940,    -1232.0,
        169.6,       -100.0,      456.7,      14080.0,
        26.18,          0.0,    -24.41,      -150.6 ,
        0.0,            0.0,        1.0,        0.0
    };

    float elements_ssmB[4] = {
        0.2572,
        0,
        0,
        0
    };

    float elements_ssmC[4] = {
        1.696, 0, 4.567, 140.8
    };

    float elements_ssmD[1] = {
        0
    };

    juce::dsp::Matrix<float> ssmatrixX = juce::dsp::Matrix<float>(sysDim, 1);
    juce::dsp::Matrix<float> ssmatrixX_next = juce::dsp::Matrix<float>(sysDim, 1);
    juce::dsp::Matrix<float> ssmatrixA = juce::dsp::Matrix<float>(sysDim, sysDim, elements_ssmA);
    juce::dsp::Matrix<float> ssmatrixB = juce::dsp::Matrix<float>(sysDim, 1, elements_ssmB);
    juce::dsp::Matrix<float> ssmatrixC = juce::dsp::Matrix<float>(1, sysDim, elements_ssmC);
    juce::dsp::Matrix<float> ssmatrixD = juce::dsp::Matrix<float>(1, 1, elements_ssmD);
    juce::HeapBlock<float> x_1next;
    juce::HeapBlock<float> x_2next;
    juce::HeapBlock<float> x_3next;
    juce::HeapBlock<float> x_4next;


    //general stuff

    juce::dsp::ProcessSpec spec; //samplerate etc.
    //int expectedRequestRate;
    //juce::AudioBuffer<float> _buffer;
    juce::AudioBuffer<float> _statebuffer;
    juce::AudioBuffer<float> _statebufferNext;
    juce::AudioBuffer<float> _outputbuffer;

    float vuLevelArrayLeft;
    float vuLevelArrayRight;

    static const float minimalReturnValue; // virtual -INF

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogVuMeterProcessor)
};

#endif //__ANALOG_VU_METER_PROCESSOR__