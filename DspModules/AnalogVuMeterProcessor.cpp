/*
  ==============================================================================

    AnalogVuMeterProcessor.cpp
    Created: 19 Mar 2024 4:06:42pm
    Author:  prmir

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalogVuMeterProcessor.h"
#include <cmath>
//static member constants
const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 90));



//==============================================================================
AnalogVuMeterProcessor::AnalogVuMeterProcessor() :
    _buffer(2, 2048),
    currentBlockIsSilent(false)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    DBG("AnalogVuMeterProcessor Spawned");

    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    //
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(48000.0, 2, 512);
}

AnalogVuMeterProcessor::~AnalogVuMeterProcessor()
{
}

void AnalogVuMeterProcessor::prepareToPlay(double sampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock)
{
    spec.sampleRate =sampleRate;
    spec.numChannels = numberOfInputChannels;
    spec.maximumBlockSize = estimatedSamplesPerBlock;
    //this->expectedRequestRate = expectedRequestRate;


    _buffer.setSize(numberOfInputChannels, estimatedSamplesPerBlock);
    bufferRectifier.prepareToPlay(sampleRate, numberOfInputChannels);

    //for ssequation
    x_0next.calloc(numberOfInputChannels);
    x_1next.calloc(numberOfInputChannels);
    x_2next.calloc(numberOfInputChannels);
    x_3next.calloc(numberOfInputChannels);
}

void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& buffer)
{
    //this will take rectified buffer "_buffer"

    const int numberOfChannels = juce::jmin(buffer.getNumChannels(), static_cast<int>(spec.numChannels));
    const int numberOfSamples = buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = this->sysDim;

    _buffer = buffer; //buffer for measurement.
    _memorybuffer.setSize(numberOfChannels, numberOfSamples + sysDim - 1);
    _memorybuffer.clear();
    _outputbuffer.setSize(numberOfChannels, numberOfSamples);
    _outputbuffer.clear();

    for (int channel = 0; channel < numberOfChannels; ++channel)
    {
        float* x_ = _memorybuffer.getWritePointer(channel); //memory for the operation
        float* u_ = _buffer.getWritePointer(channel); //copy of the input audiobuffer
        float* y_ = _outputbuffer.getWritePointer(channel);
        x_[0] = x_0next[channel];
        x_[1] = x_1next[channel];
        x_[2] = x_2next[channel];
        x_[3] = x_3next[channel]; //from previous iteration

        for (int i = 0; i < _memorybuffer.getNumSamples(); ++i)
        {
            float x_0 = x_[i];
            float x_1 = x_[i] - x_[i - 1];
            float x_2 = x_[i] - 2 * x_[i - 1] + x_[i - 2];
            float x_3 = x_[i] - 3 * x_[i - 1] + 3 * x_[i - 2] - 1 * x_[i - 3];
            //float x_4 = x_[i] - 4 * x_[i - 1] + 6 * x_[i - 2] - 4 * x_[i - 3] + x_[i - 4];
            float elements_X[4] = { x_0, x_1, x_2, x_3 };
            //float elementsdX[4] = { x_1, x_2, x_3, x_4 };
            float elements_U[4] = { u_[i + sysDim]};
            float elements_Y[4] = { 0 };
            juce::dsp::Matrix<float> ssmat_X = juce::dsp::Matrix<float>(sysDim, 1, elements_X);
            juce::dsp::Matrix<float> ssmat_Xnext = juce::dsp::Matrix<float>(sysDim, 1);
            juce::dsp::Matrix<float> ssmat_U = juce::dsp::Matrix<float>(1, 1, elements_U);
            juce::dsp::Matrix<float> ssmat_Y = juce::dsp::Matrix<float>(1, 1, elements_Y);

            //matrix op

            ssmat_Xnext = ssmatrixA * ssmat_X + ssmatrixB * ssmat_U; //4 elms will be stored in _memorybuffer
            ssmat_Y = ssmatrixC * ssmat_X + ssmatrixD * ssmat_U; //output value
            x_0next[channel] = ssmat_Xnext(0, 0);
            x_1next[channel] = ssmat_Xnext(1, 0);
            x_2next[channel] = ssmat_Xnext(2, 0);
            x_3next[channel] = ssmat_Xnext(3, 0);

            y_[i] = float(ssmat_Y(0, 0)); //this is the "meter movement"

            //DBG("raw value for VU = " + juce::String(y_[i]));
            //_bufferLastValue.at(channel) = float(ssmat_Y(0, 0));
        }
    }
}

void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{


    if (currentBlockIsSilent)
    {
        //if the block is silence
        const float silenceThreshold = std::pow(10, 0.1 * -45);
        const float magnitude = buffer.getMagnitude(0, buffer.getNumSamples());

        if (magnitude < silenceThreshold)
        {
            currentBlockIsSilent = true;
            DBG("silence True");
        }
        else {
            currentBlockIsSilent = false;
            DBG("silence False");
        }
    }

    //STEP 1:: rectify and damp the signal transient.
    bufferRectifier.processBlock(buffer);
    _buffer = bufferRectifier.rectifiedBuffer;

    //STEP 2:: set the number of channels to prevent EXC_BAD_ACCESS
    //when the number of channels in the buffer suddenly changes w/o calling
    //prepareToPlay() in advance.

    const int numberOfChannels = juce::jmin(
        _buffer.getNumChannels(), static_cast<int>(spec.numChannels));
    
    //STEP 3:: accum. the samples and use the steady state equation.
    feedToSteadyStateEquation(_buffer);

    //STEP 4:: throw every data in _outputbuffer to the "Vu Meter"
    vuLevelArrayLeft = getVuLevelForIndividualChannels(0);
    vuLevelArrayRight = getVuLevelForIndividualChannels(1);
    //end of sequence
}


void AnalogVuMeterProcessor::reset()
{
    x_0next.clear(spec.numChannels);
    x_1next.clear(spec.numChannels);
    x_2next.clear(spec.numChannels);
    x_3next.clear(spec.numChannels);
}


//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================
vector<float> AnalogVuMeterProcessor::getVuLevelForIndividualChannels(int channel)
{
    //DBG("getVuLevelForIndividualChannels. called");
    
    vector<float> outputArr;
    jassert(channel < spec.numChannels);

    float* in = _outputbuffer.getWritePointer(channel);
    for (int i = 0; i < _outputbuffer.getNumSamples(); i++)
    {
        float in_ = juce::jmax(in[i], minimalReturnValue);
        in_ = juce::Decibels::gainToDecibels(in_);
        outputArr.push_back(in_);
    }

    //DBG("outputting buffer into float sequence done");

    return outputArr;
}


//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////


AnalogVuMeterRectifier::AnalogVuMeterRectifier() :
    numberOfChannels{ 0 },
    sampleRate{ 48000 }
{
    //determine the values if exists
}

AnalogVuMeterRectifier::~AnalogVuMeterRectifier()
{
}

void AnalogVuMeterRectifier::prepareToPlay(double sampleRate, int numberOfChannels)
{
    DBG("prepareToPlay @ Rectifier called");
    this->sampleRate = sampleRate;
    this->numberOfChannels = numberOfChannels;

    //init. z1 and z2
    z1.calloc(numberOfChannels);

    //determine the filter coeffs if E : there is none unless Tustin.


}

void AnalogVuMeterRectifier::releaseResources()
{
}

void AnalogVuMeterRectifier::processBlock(juce::AudioBuffer<float>& buffer)
{
    rectifiedBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
    rectifiedBuffer.clear();

    const int numOfChannels = juce::jmin(numberOfChannels, buffer.getNumChannels());
    for (int channel = 0; channel < numOfChannels; ++channel)
    {
        const float* samples = buffer.getReadPointer(channel);
        float* outSamples = buffer.getWritePointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float x_ = samples[i]; //input voltage v(t)
            float absx_ = std::abs(x_); //rectified voltage v(t)

            float i_c = (1 + 1.0 / resistance + 1.0 / sampleRate * capacitance) * absx_ - z1[channel];


            z1[channel] = x_;
            outSamples[i] = i_c;
            DBG("raw value after Rect = " + juce::String(i_c));
        }
    }
}

void AnalogVuMeterRectifier::reset()
{
    z1.clear(numberOfChannels);
}
