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
#include "StateSpaceModelSimulation.h"
//==============================================================================

class AnalogVuMeterProcessor  : public juce::Component
{
public:
    AnalogVuMeterProcessor();
    ~AnalogVuMeterProcessor() override;

    void prepareToPlay(double  SampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock); 

    using mat = juce::dsp::Matrix<float>;

    void feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer, int systemSize);
 
    void keepPreviousStateForNextInitSystemI();
    void keepPreviousStateForNextInitSystemII();


    void processBlock(juce::AudioBuffer<float> &buffer);

    void reset();

    juce::AudioBuffer<float> getOutputBuffer();


private:
    //for rectifying
    const size_t sysDim = 4;

    //state space model simulation classes for stereo
    StateSpaceModelSimulation ssms;
    StateSpaceModelSimulation ssms_v2i; //for n-channels
    StateSpaceModelSimulation ssms_i2a; //for n-channels

    //StateSpaceModelSimulation ssmsLeft_v2i;
    //StateSpaceModelSimulation ssmsRight_v2i;
    //StateSpaceModelSimulation ssmsLeft_i2a;
    //StateSpaceModelSimulation ssmsRight_i2a;


    //state space equation matrices for VOLT -> CUR : System I
    float v2i_A[16] = {
        -10.0,     0.0,     0.0,    -10.0,
        -9.671e+6,       -45.45,      -9.671e+6,      9.671,
       -21.28,          0.0,    -21.28,      -2.128e-5,
        -0.01,            0.0,        1.0,       -0.21
    };

    float v2i_B[4] = {
        10.0,
        9.671e+6,
        21.28,
        0.01
    };

    float v2i_C[4] = {
        8.674e-19, - 8.674e-19,   8.078e-28, -1.654e-24
    };

    float v2i_D[1] = {
        -8.674e-19
    };

    mat ssm_v2i_x, ssm_v2i_x0;
    mat ssm_v2i_A;
    mat ssm_v2i_B;
    mat ssm_v2i_C;
    mat ssm_v2i_D;


    //state space equation matrices for CUR -> ANGLE : system II
    float i2a_A[16] = {
        -19.84,      8.746,     -39.940,    -1232.0,
        169.6,       -100.0,      456.7,      14080.0,
        26.18,          0.0,    -24.41,      -150.6 ,
        0.0,            0.0,        1.0,        0.0
    };

    float i2a_B[4] = {
        0.2572,
        0,
        0,
        0
    };

    float i2a_C[4] = {
        1.696, 0, 4.567, 140.8
    };

    float i2a_D[1] = {
        0
    };

    mat ssm_i2a_x, ssm_i2a_x0;
    mat ssm_i2a_A;
    mat ssm_i2a_B;
    mat ssm_i2a_C;
    mat ssm_i2a_D;

    juce::HeapBlock<float> z1;
    juce::HeapBlock<float> z2;
    juce::HeapBlock<float> z3;
    juce::HeapBlock<float> z4;

    juce::HeapBlock<float> w1;
    juce::HeapBlock<float> w2;
    juce::HeapBlock<float> w3;
    juce::HeapBlock<float> w4;


    //==============================================================================

    juce::AudioBuffer<float> bufferForMeasurement;
    std::vector<mat> outputPostSystemI;
    std::vector<mat> outputPostSystemII;

    juce::AudioBuffer<float> bufferForInitialStateSystemI; //previous 4 samples
    juce::AudioBuffer<float> bufferForInitialStateSystemII; //previous 4 samples
    juce::dsp::ProcessSpec spec; //samplerate etc.
    float vuLevelArrayLeft;
    float vuLevelArrayRight;

    static const float minimalReturnValue; // virtual -INF

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogVuMeterProcessor)
};

#endif //__ANALOG_VU_METER_PROCESSOR__