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
#include "juce_dsp/maths/juce_Matrix.h"

using std::vector;
//==============================================================================
/*
* 
*/



class AnalogVuMeterRectifier;

class AnalogVuMeterProcessor  : public juce::Component
{
public:
    AnalogVuMeterProcessor();
    ~AnalogVuMeterProcessor() override;

    void prepareToPlay(double  SampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock, int expectedRequestRate);
    
    void feedToSteadyStateEquation(juce::AudioSampleBuffer& buffer);

    void processBlock(juce::AudioSampleBuffer& buffer);
    
    void getVuLevel() const; //mono
    vector<float>& getVuLevelForIndividualChannels();

    void reset();
    



    //>>>>>>>>>>>>>>>
    void paint (juce::Graphics&) override;
    void resized() override;

protected:



private:
    juce::dsp::ProcessSpec spec; //samplerate etc.
    int expectedRequestRate;
    AnalogVuMeterRectifier bufferRectifier;

    static int round(double d);
    juce::AudioSampleBuffer bufferForMeasurement;

    int numberOfBins;
    int numberOfSamplesPerBin;
    int numberOfSamplesInAllBins;

    //the duration of the current measurement :
    // duration * .1 = the mes. duration in sec.
    int measurementDuration;

    vector<vector<double>>bin;
    int currentBin;
    int numberOfSamplesInTheCurrentBin;

    vector<float> vuLevels;
    bool currentBlockIsSilent;

    static const float minimalReturnValue; // virtual -INF

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogVuMeterProcessor)
};


//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////


class AnalogVuMeterRectifier : public juce::Component
{
public:
    AnalogVuMeterRectifier();
    virtual ~AnalogVuMeterRectifier() override;

    virtual void prepareToPlay(double sampleRate, int numberOfChannels);

    virtual void releaseResources();

    //renders the next block
    void processBlock(juce::AudioSampleBuffer& buffer);

    void reset();

protected:
    //essential coeffs and variables valid for a sample rate of 48000Hz
    double capacitance = 22e-6;
    
    //essential coeffs and variables reserved to be set in prepareToPlay

private:


    //filter params that are set in the constr. and used in prepareToPlay
    //for steady state equation
    size_t systemDim = 4;

    vector<double> elements_ssmA = {
        -19.84,      8.746,     -39.940,    -1232.0,
        169.6,       -100.0,      456.7,      14080.0,
        26.16,          0.0,    -24.41,      -150.6 ,
        0.0,            0.0,        1.0,        0.0
    };
    juce::dsp::Matrix<double> ssmatrixA(systemDim, systemDim, juce::ReferenceCountedArray<double>(elements_ssmA.data(), elements_ssmA.size()));
    juce::dsp::Matrix<double> ssmatrixB;
    juce::dsp::Matrix<double> ssmatrixC;
    juce::dsp::Matrix<double> ssmatrixD;



    //SR and audioChannelNums
    int numberOfChannels;
    double sampleRate;

    //stores the prev. value of the variables
    juce::HeapBlock<double> z1;


};

#endif //__ANALOG_VU_METER_PROCESSOR__