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
//#include "juce_dsp\maths\juce_Matrix.h"


using std::vector;
//==============================================================================
/*
* 
*/


class AnalogVuMeterRectifier : public juce::Component
{
public:
    AnalogVuMeterRectifier();
    virtual ~AnalogVuMeterRectifier() override;

    virtual void prepareToPlay(double sampleRate, int numberOfChannels);

    virtual void releaseResources();

    //renders the next block
    void processBlock(juce::AudioBuffer<float>& buffer);

    void reset();

    juce::AudioBuffer<float> rectifiedBuffer;

private:
    double capacitance = 22e-6;
    double resistance = 650;
    float outputManitudeCalibration = 1;
    //filter params that are set in the constr. and used in prepareToPlay
    //for steady state equation

    //SR and audioChannelNums
    int numberOfChannels;
    double sampleRate;

    //stores the prev. value of the variables
    juce::HeapBlock<float> z1;


};


//==============================================================================


class AnalogVuMeterProcessor  :
    public juce::Component
{
public:
    AnalogVuMeterProcessor();
    ~AnalogVuMeterProcessor() override;

    void prepareToPlay(double  SampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock);
    
    template <typename T>
    void getNextState(const T* x, const T u, T* x_next, double sampleRate);

    template <typename T>
    T getOutput(const T* x, const T u);

    void feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer);

    void processBlock(juce::AudioBuffer<float> buffer);

    float* get_Outputbuffer(int channel) const;

    void reset();

protected:

private:
    //state space equation stuff
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

    juce::dsp::Matrix<float> ssmatrixA = juce::dsp::Matrix<float>(sysDim, sysDim, elements_ssmA);
    juce::dsp::Matrix<float> ssmatrixB = juce::dsp::Matrix<float>(sysDim, 1, elements_ssmB);
    juce::dsp::Matrix<float> ssmatrixC = juce::dsp::Matrix<float>(1, sysDim, elements_ssmC);
    juce::dsp::Matrix<float> ssmatrixD = juce::dsp::Matrix<float>(1, 1, elements_ssmD);
    juce::HeapBlock<float> x_1next;
    juce::HeapBlock<float> x_2next;
    juce::HeapBlock<float> x_3next;


    //general stuff

    juce::dsp::ProcessSpec spec; //samplerate etc.
    //int expectedRequestRate;
    AnalogVuMeterRectifier bufferRectifier;
    juce::AudioBuffer<float> _buffer;
    juce::AudioBuffer<float> _statebuffer;
    juce::AudioBuffer<float> _statebufferNext;
    juce::AudioBuffer<float> _outputbuffer;

    float vuLevelArrayLeft;
    float vuLevelArrayRight;

    int vuMeterWindowSize = 300;
    bool currentBlockIsSilent;

    static const float minimalReturnValue; // virtual -INF

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogVuMeterProcessor)
};


//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////



#endif //__ANALOG_VU_METER_PROCESSOR__