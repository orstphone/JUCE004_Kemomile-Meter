/*
  ==============================================================================

    RectifierProcessor.h
    Created: 14 Mar 2024 12:11:30am
    Author:  Decca (https://github.com/deccauvw)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
constexpr int RECTIFIER_MAX_NB_CHANNELS = 2;

//==============================================================================
/*
*/
namespace DspModules
{
    juce::AudioSampleBuffer prev_inputs;
    void drawProcessedBuffer(const juce::AudioBuffer<float>& buffer);
    //replace input buffer with processed buffer


    class RectifierProcessor : public juce::Component
    {
    public:
        RectifierProcessor();
        ~RectifierProcessor() override;

        struct LinearValue
        {
            float m_channelArray[RECTIFIER_MAX_NB_CHANNELS];

            LinearValue()
            {
                for (int i = 0; i < RECTIFIER_MAX_NB_CHANNELS; ++i)
                    m_channelArray[i] = 0;              
            }

            float Rectify() const
            {
                float value = 0.0;
                for (int i = 0; i < RECTIFIER_MAX_NB_CHANNELS; ++i)
                {
                    if (0 < m_channelArray[i])
                        value = -m_channelArray[i];
                    else
                        value = m_channelArray[i];
                }
                return value;
            }
        };

   
        LinearValue process(const juce::AudioBuffer<float>& buffer);
        void reset(); // reset  internal buffers

        //======================================================================

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        LinearValue RectifyAndSmoothout(const juce::AudioBuffer<float>& buffer);
        juce::AudioSampleBuffer m_inputs; //sAR process this buffer
        

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RectifierProcessor)
    };
//private:


}