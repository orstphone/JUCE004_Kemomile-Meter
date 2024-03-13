/*
  ==============================================================================

    TruePeakProcessor.cpp
    Created: 13 Mar 2024 3:54:33pm
    Author:  Decca (https://github.com/deccauvw)

    reference document : 
    https://github.com/unusual-audio/loudness-meter
    // Polyphase peak calculation and coefficients are taken from
    // Annex 2 of https://www.itu.int/rec/R-REC-BS.1770/:
    // Guidelines for accurate measurement of ¡°true-peak¡± level,

  ==============================================================================
*/

#include <JuceHeader.h>
#include "TruePeakProcessor.h"


const float filterPhase0[] =
{
    0.0017089843750f, 0.0109863281250f, -0.0196533203125f, 0.0332031250000f,
    -0.0594482421875f, 0.1373291015625f, 0.9721679687500f, -0.1022949218750f,
    0.0476074218750f, -0.0266113281250f, 0.0148925781250f, -0.0083007812500f
};

const float filterPhase1[] =
{
    -0.0291748046875f, 0.0292968750000f, -0.0517578125000f, 0.0891113281250f,
    -0.1665039062500f, 0.4650878906250f, 0.7797851562500f, -0.2003173828125f,
    0.1015625000000f, -0.0582275390625f, 0.0330810546875f, -0.0189208984375f
};

const float filterPhase2[] =
{
    -0.0189208984375f, 0.0330810546875f, -0.0582275390625f, 0.1015625000000f,
    -0.2003173828125f, 0.7797851562500f, 0.4650878906250f, -0.1665039062500f,
    0.0891113281250f, -0.0517578125000f, 0.0292968750000f, -0.0291748046875f
};

const float filterPhase3[] =
{
    -0.0083007812500f, 0.0148925781250f, -0.0266113281250f, 0.0476074218750f,
    -0.1022949218750f, 0.9721679687500f, 0.1373291015625f, -0.0594482421875f,
    0.0332031250000f, -0.0196533203125f, 0.0109863281250f, 0.0017089843750f
};

const float* filterPhaseArray[] = { filterPhase0, filterPhase1, filterPhase2, filterPhase3 };
const int numCoeffs = sizeof(filterPhase0) / sizeof(float);

//==============================================================================
AudioProcessing::TruePeak::TruePeak()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}


AudioProcessing::TruePeak::~TruePeak()
{
}

void AudioProcessing::TruePeak::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("TruePeakProcessor", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void AudioProcessing::TruePeak::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

//======================================================================

float AudioProcessing::processTruePeak(const juce::AudioBuffer<float>& buffer)
{
    float truePeakValue = 0.0f;

    AudioProcessing::TruePeak truePeak;
    int offset = 0;
    TruePeak::LinearValue value = truePeak.process(buffer);

    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        if (truePeakValue < value.m_channelArray[i])
            truePeakValue = value.m_channelArray[i];
    }

    return juce::Decibels::gainToDecibels(truePeakValue, -INFINITY);
}


void AudioProcessing::polyphase4(const juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& result)
{
    int bufferSize = buffer.getNumSamples();
    result.setSize(buffer.getNumSamples(), 4 * buffer.getNumSamples() + numCoeffs, false, false, true);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* input = buffer.getArrayOfReadPointers()[ch];
        float* res = result.getArrayOfWritePointers()[ch];

        for (int i = 0; i < bufferSize; ++i)
        {
            res[4 * i + 0] = polyphase4ComputeSum(input, i, buffer.getNumSamples(), filterPhase0, numCoeffs);
            res[4 * i + 1] = polyphase4ComputeSum(input, i, buffer.getNumSamples(), filterPhase1, numCoeffs);
            res[4 * i + 2] = polyphase4ComputeSum(input, i, buffer.getNumSamples(), filterPhase2, numCoeffs);
            res[4 * i + 3] = polyphase4ComputeSum(input, i, buffer.getNumSamples(), filterPhase3, numCoeffs);

        }
    }
}


float AudioProcessing::polyphase4ComputeSum(const float* input, int offset, int maxOffset, const float* coefficients, int numCoeff)
{
    float sum = 0.0f;
    for(int j = 0 ; j < numCoeff; ++j)
    {
        int index = offset - j;
        if ((index >= 0) && (index < maxOffset))
        {
            sum += (input[index] * coefficients[j]);
        }
    }
    return sum;
}



AudioProcessing::TruePeak::LinearValue AudioProcessing::TruePeak::process(const juce::AudioBuffer<float>& buffer)
{

    if (TruePeak::m_inputs.getNumSamples() > numCoeffs)
    {
        juce::AudioBuffer<float> tempBuffer;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            TruePeak::m_inputs.copyFrom(ch, 0, buffer.getArrayOfReadPointers()[ch], TruePeak::m_inputs.getNumSamples() - numCoeffs, numCoeffs);
        }
        //resize
        if (TruePeak::m_inputs.getNumSamples() != numCoeffs + buffer.getNumSamples())
            TruePeak::m_inputs.setSize(buffer.getNumChannels(), numCoeffs + buffer.getNumSamples(), true, false, true);
    }
    else
    {
        //setSize clear buffer content too
        TruePeak::m_inputs.setSize(buffer.getNumChannels(), numCoeffs + buffer.getNumSamples(), false, true);
    }

    //copy buffer to inputs with numcoeffs ofs
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        TruePeak::m_inputs.copyFrom(ch, numCoeffs, buffer, ch, 0, buffer.getNumSamples());
    }
    return processPolyphase4AbsMax(TruePeak::m_inputs);
}


void AudioProcessing::TruePeak::reset()
{
    TruePeak::m_inputs.setSize(0, 0);
}


AudioProcessing::TruePeak::LinearValue AudioProcessing::TruePeak::processPolyphase4AbsMax(const juce::AudioBuffer<float>& buffer)
{
    LinearValue value;
    int sampleSize = buffer.getNumSamples();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* input = buffer.getArrayOfReadPointers()[ch];
        for (int i = 0; i < sampleSize; ++i)
        {
            for (int j = 0; j < 4; ++j)//num of polyphs filters
            {
                float absSample = fabs(polyphase4ComputeSum(input, i, buffer.getNumSamples(), filterPhaseArray[j], numCoeffs));
                if (absSample > value.m_channelArray[ch])
                    value.m_channelArray[ch] = absSample;
            }
        }
    }

    return value;
}