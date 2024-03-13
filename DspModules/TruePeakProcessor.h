/*
  ==============================================================================

    TruePeakProcessor.h
    Created: 13 Mar 2024 3:54:33pm
    Author:  prmir

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
constexpr int LUFS_TP_MAX_NB_CHANNELS = 2;

//==============================================================================
/*
*/
class AudioProcessing
{
public:
    //applies TP processing on fly
    static float processTruePeak(const juce::AudioBuffer<float>& buffer);

    //True Peak class calculates True Peak linear Volume for Buffer


    class TruePeak : public juce::Component
    {
    public:
        TruePeak();
        ~TruePeak() override;

        struct LinearValue
        {
            float m_channelArray[LUFS_TP_MAX_NB_CHANNELS];

            LinearValue()
            {
                for (int i = 0; i < LUFS_TP_MAX_NB_CHANNELS; ++i)
                    m_channelArray[i] = 0;
            }

            float getMax() const
            {
                float value = 0;
                for (int i = 0; i < LUFS_TP_MAX_NB_CHANNELS; ++i)
                {
                    if (value < m_channelArray[i])
                        value = m_channelArray[i];
                }
                return value;
            }
            
        };


        //process : since this method needs numCoeffs values more than buffer size,
        //numCoeffs values from prev. process call are used at beginning of buffer
        LinearValue process(const juce::AudioBuffer<float>& buffer);

        //resets internal buffers
        void reset();


        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        LinearValue processPolyphase4AbsMax(const juce::AudioBuffer<float>& buffer);
        juce::AudioSampleBuffer m_inputs; //getPolyphase4AbsMax processes this buffer
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TruePeak)
    };
private:
    static void polyphase4(const juce::AudioBuffer<float>& source, juce::AudioBuffer<float>& result);
    static float polyphase4ComputeSum(const float* input, int offset, int maxOffset, const float* coefficients, int numCoeff);
};
