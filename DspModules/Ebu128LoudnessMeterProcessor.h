/*
  ==============================================================================

    Ebu128LoudnessMeterProcessor.h
    Created: 14 Mar 2024 12:11:30am
    Author:  Decca (https://github.com/deccauvw)

    reference
    https://github.com/unusual-audio/loudness-meter/tree/main/Source/Vendor/Ebu128LoudnessMeter
  ==============================================================================
*/

#pragma once

#ifndef __EBU128_LOUDNESS_METER__
#define __EBU128_LOUDNESS_METER__

#include <JuceHeader.h>
#include "SecondOrderIIRFilter.h"
#include <map>
#include <vector>

using std::map;
using std::vector;

//==============================================================================
/*

 Measures the loudness of an audio stream.
 
 The loudness is measured according to the documents
 (List)
 - EBU - R 128
 - ITU 1770 Rev 2,3 and 4
 - EBU - Tech 3341 (EBU mode metering)
 - EBU - Tech 3342 (LRA, loudness range)
 - EBU - Tech 3343
*/


class Ebu128LoudnessMeterProcessor  : public juce::Component
{
public:
    Ebu128LoudnessMeterProcessor();
    ~Ebu128LoudnessMeterProcessor() override;

    //--------------audioProcessor methods-----------------
    //const String getName();
    /*
     @param sampleRate
     @param numberOfChannels
     @param estimatedSamplesPerBlock
     @param expectedRequestRate         
        Assumption about how many times
        a second the measurement values will be requested. Internally,
        this will be changed to a multiple of 10 because exactly every
        0.1 second a gating block needs to be measured (for the
        integrated loudness measurement).
     */

    void prepareToPlay(double SampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock, int expectedRequestRate);
    
    void processBlock(juce::AudioSampleBuffer& buffer);

    float getShortTermLoudness() const;

    float getMaximumShortTermLoudness() const;

    vector<float>& getMomentaryLoudnessForIndividualChannels();

    float getMomentaryLoudness() const;

    float getMaximumMomentaryLoudness() const;

    float getIntegratedLoudness() const;

    float getLoudnessRangeStart() const;

    float getLoudnessRangeEnd() const;

    float getLoudnessRange() const;
    //returns the time passed since the last reset in seconds.

    float getMeasurementDuration() const;

    void setFreezeLoudnessRangeOnSilence(bool freeze);

    void reset();



private:
    static int round(double d);
    juce::AudioSampleBuffer bufferForMeasurement;
    //buffer given to processBlock() will be copied to this buffer.
    //such that the filtering and sdquaring won't affect the audio output.

    SecondOrderIIRFilter preFilter;

    SecondOrderIIRFilter revisedLowFrequencyBCurveFilter;

    int numberOfBins;
    int numberOfSamplesPerBin;
    int numberOfSamplesInAllBins;
    int numberOfBinsToCover400ms;
    int numberOfSamplesIn400ms;

    int numberOfBinsToCover100ms;
    int numberOfBinsSinceLastGateMeasurementForI;
    
    int measurementDuration;
    //the duration of the current measurement : duration * .1 = the mes. duration in sec.

    vector<vector<double>>bin;
    int currentBin;
    int numberOfSamplesInTheCurrentBin;
    
    vector<double> averageOfTheLast3s;
    //the avr. of the filtered and squared samples of the last 3 seconds.
    //a value for each channel.

    vector<double> averafeOfTheLast400ms;

    vector<double> channelWeighting;

    vector<float> momentaryLoudnessForIndividualChannels;

    static const float minimalReturnValue;
    //if there's no signal at all, the methods getShortTermLoudness() and getMomentaryLoudness()
    //would perform a log(0) which is -NaN. to avoid this the return value of this methods will be set to
    //minimalReturnValue;

    static const double absoluteThreshold;
    //a gated window needs to be bigger than this value to 
    //contribute to the calculation of the relative threshold
    //abs threshold = gamma_a = -70 LUFS

    int numberOfBlocksToCalculateRelativeThreshold;
    double sumOfAllBlocksToCalculateRelativeThreshold;
    double relativeThreshold;

    int numberOfBlocksToCalculateRelativeThresholdLRA;
    double sumOfAllBlocksToCalculateRelativeThresholdLRA;
    double relativeThresholdLRA;

    static const double lowestBlockLoudnessToConsider;
    /** lower bound for the histograms(for I and LRA)
    * If a measured block has a value lower than this, it will not be 
    * considered in the calculation for I and LRA
    * without the possibility to increase the pre-measurement-gain at any point
    * after the measurement has started, this could've been set
    * to t he absThreshold = -70LUFS
    */

    map<int, int> histogramOfBlockLoudness;
    /**Storage for the loudnesses of all 400ms blocks since the last reset.
    * Because the relative threshold varies and all blocks with a loudness
    * bigger than the relative threshold are needed to calculate the gated loudness(integrated loudness),
    * it is mandatory to keep track of all block loudnesses.
    * 
    * adj. bins are set apart by 0.1LU wich seems to be sufficient.
    * key value = loudness * 10 (to get an int. value)
    */


    //the main loudness value of interest
    float integratedLoudness;
    float shortTermLoudness;
    float maximumShortTermLoudness;
    float momentaryLoudness;
    float maximumMomentaryLoudness;
    
    map<int, int> histogramOfBlockLoudnessLRA;
    /** Like histogramOfBlockLoudness, but for the measurement of the
    loudness range.

    The histogramOfBlockLoudness can't be used simultaneous for the
    loudness range, because the measurement blocks for the loudness
    range need to be of length 3s. Vs 400ms.
    */

    float loudnessRangeStart;
    float loudnessRangeEnd;

    bool freezeLoudnessRangeOnSilence;
    bool currentBlockIsSilent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ebu128LoudnessMeterProcessor)
};

#endif // __EBU128_LOUDNESS_METER__