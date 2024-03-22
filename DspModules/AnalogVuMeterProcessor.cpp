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
const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 40));


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
        float* outSamples = rectifiedBuffer.getWritePointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float x_ = samples[i]; //input voltage v(t)
            float absx_ = std::abs(x_); //rectified voltage v(t)

            float i_c = (1.0 - 1.0 / resistance) * absx_ + (1.0 / (sampleRate * capacitance)) * z1[channel];


            z1[channel] = x_;
            outSamples[i] = i_c;
            //DBG("raw value after Rect = " + juce::String(i_c));
        }
    }
}

void AnalogVuMeterRectifier::reset()
{
    z1.clear(numberOfChannels);
}

//////////////////////////////////VuMeterProcessorZone///////////////////////////////////
//////////////////////////////////VuMeterProcessorZone///////////////////////////////////
//////////////////////////////////VuMeterProcessorZone///////////////////////////////////
//////////////////////////////////VuMeterProcessorZone///////////////////////////////////


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
    _buffer.clear();
    bufferRectifier.prepareToPlay(sampleRate, numberOfInputChannels);

    //for ss - equation
    x_1next.calloc(numberOfInputChannels);
    x_2next.calloc(numberOfInputChannels);
    x_3next.calloc(numberOfInputChannels);
}

template <typename T>
void AnalogVuMeterProcessor::getNextState(const T* x, const T u, T* x_next,  double sampleRate) //perChannel
{   
    //single channel wise

    //calculate x_next = x + dx/dt * dt
    //dt := 1/Ts
    auto dt = (float)(1.0 / sampleRate);

    for (size_t i = 0; i < sysDim; ++i)
    {
        float sum = 0.0;
        for (size_t j = 0; j < sysDim; ++j)
        {
            sum += ssmatrixA(i, j) * x[j];
        }

        sum += ssmatrixB(i, 0) * u; //add input contribution
        x_next[i] = (x[i] + sum * dt);
    }


}

template<typename T>
T AnalogVuMeterProcessor::getOutput(const T* x, const T u)
{
    //single channel wise
    T y = 0.0;

    for (size_t i = 0; i < sysDim; ++i)
    {
        y += ssmatrixC(0, i) * x[i] + ssmatrixD(0, 0) * u;
    }

    return y;
}

void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& _buffer) //_buffer == rectified one
{
    //this must take rectified buffers "_buffer"

    const int numberOfChannels =_buffer.getNumChannels();
    const int numberOfSamples = _buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = this->sysDim;

    _statebuffer.setSize(numberOfChannels, numberOfSamples + sysDim - 1);
    _statebuffer.clear();
    _outputbuffer.setSize(numberOfChannels, numberOfSamples);
    _outputbuffer.clear();
    _statebufferNext.setSize(1, numberOfSamples + sysDim - 1);
    _statebufferNext.clear();



    for (int channel = 0; channel < numberOfChannels; ++channel)
    {
        float* x_ = _statebuffer.getWritePointer(channel); //initial state.
        float* u_ = _buffer.getWritePointer(channel); //input sequence.
        float* y_ = _outputbuffer.getWritePointer(channel); //output sequence.
        double sr = spec.sampleRate;

        //assign previous 3 values
        x_[numberOfSamples - 1] = x_1next[channel];
        x_[numberOfSamples - 2] = x_2next[channel];
        x_[numberOfSamples - 3] = x_3next[channel];


        //simulate the system
        _statebufferNext.clear();
        float* x_next = _statebufferNext.getWritePointer(0);

        for (int i = 0; i < numberOfSamples + sysDim - 1; ++i)
        {
            //calc next state -> vector<float>
            getNextState(x_, u_[i], x_next, sr); //is x_next audiobuffer or float? -> it's buffer == float*

            float y = getOutput(x_, u_[i]);

            y_[i] = float(y); //this is the "meter movement"

            //_bufferLastValue.at(channel) = float(ssmat_Y(0, 0));
        }


        // remembering old 3 values before evaporating
        x_1next[channel] = x_next[numberOfSamples - 1];
        x_2next[channel] = x_next[numberOfSamples - 2];
        x_3next[channel] = x_next[numberOfSamples - 3];


        DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[0]));
    }

    //_outputbuffer will be assigned from somewhewr.
    //the buffer is declared in the header as private member.
}

void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float> buffer)
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
    _buffer = bufferRectifier.rectifiedBuffer; //_buffer is a rectified "buffer"

    //STEP 2:: set the number of channels to prevent EXC_BAD_ACCESS
    //when the number of channels in the buffer suddenly changes w/o calling
    //prepareToPlay() in advance.

    const int numberOfChannels = juce::jmin(
        _buffer.getNumChannels(), static_cast<int>(spec.numChannels));
    
    //STEP 3:: accum. the samples and use the steady state equation.
    feedToSteadyStateEquation(_buffer);

    //STEP 4:: throw every data in _outputbuffer to the "Vu Meter"
    //from here _outputBuffer will handle "needlepoint" of the meter.


    //end of sequence
}

float* AnalogVuMeterProcessor::get_Outputbuffer(int channel) const
{
    float* out = _outputbuffer.getWritePointer(channel);
    return out;
}


void AnalogVuMeterProcessor::reset()
{
    x_1next.clear(spec.numChannels);
    x_2next.clear(spec.numChannels);
    x_3next.clear(spec.numChannels);
}


//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================

