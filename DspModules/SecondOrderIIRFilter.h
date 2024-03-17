/*
  ==============================================================================

    SecondOrderIIRFilter.h
    Created: 17 Mar 2024 1:40:44pm
    Author:  Decca (https://github.com/deccauvw)

  ==============================================================================
*/

#pragma once
#ifndef __FILTER_OF_SECOND_ORDER__
#define __FILTER_OF_SECOND_ORDER__
#include <JuceHeader.h>

//==============================================================================
/*
     A second order IIR (infinite inpulse response) filter.

     The filter will process the signal on every channel like this:

     out[k] = b0 * factorForB0[k] + b1 * z1[k] + b2 * z2[k]

     with
     factorForB0[k] = in[k] - a1 * z1[k] - a2 * z2[k]
     z1[k] = factorForB0[k-1]
     z2[k] = factorForB0[k-2]

     This structure is depicted in ITU-R BS.1770-2 Figure 3 as well as in
     111222_2nd_order_filter_structure.tif .
    */

class SecondOrderIIRFilter  : public juce::Component
{
public:
    //==============================================================================
    SecondOrderIIRFilter(   double b0_at48k_ = 1.0,
                            double b1_at48k_ = 0.0,
                            double b2_at48k_ = 0.0,
                            double a1_at48k_ = 0.0,
                            double a2_at48k_ = 0.0);
    virtual ~SecondOrderIIRFilter() override;

    //==============================================================================
    //call before the playback starts, to let the filter pre-prepare;
    virtual void prepareToPlay(double sampleRate, int numberOfChannels);

    //call after the playback has stopped,
    //to let the filter free up any resources it no longer needs.
    virtual void releaseResources();

    //renders the next block
    void processBlock(juce::AudioSampleBuffer& buffer);
    void reset();

protected:
    //==============================================================================
    //filter coeffs, valid for a sample rate of 48000Hz
    double b0_at48k, b1_at48k, b2_at48k, a1_at48k, a2_at48k;

    //filter coeffs. for the used sampleRate, they are set in prepare to play.
    double b0, b1, b2, a1, a2;




private:
    //==============================================================================

    //filter params that are set in the constructor and used in
    //prepareToPlay to cal. the filter coeffs.
    // 
    double Q, VH, VB, VL, arctanK;

    //audioChannelNums
    int numberOfChannels;

    //stores the prev. value of the variable factorForB2 for every audio Channel
    juce::HeapBlock<double> z1;

    //stores the prev. value of z1 for every audio channel.
    juce::HeapBlock<double> z2;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderIIRFilter)
};




#endif  // __FILTER_OF_SECOND_ORDER__