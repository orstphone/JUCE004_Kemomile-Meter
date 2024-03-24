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
const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 120));


//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////
//////////////////////////////////Rectifier zone///////////////////////////////////


AnalogVuMeterRectifier::AnalogVuMeterRectifier() :
    numberOfChannels{ 0 },
    sampleRate{ 48000 }
{
    //determine the values if exists
    spec.maximumBlockSize = 0;
    spec.numChannels = 0;
    spec.sampleRate = 0;

}

AnalogVuMeterRectifier::~AnalogVuMeterRectifier()
{
    z1.free();
}





void AnalogVuMeterRectifier::prepareToPlay(double sampleRate, int numberOfChannels)
{
    this->sampleRate = sampleRate;
    this->numberOfChannels = numberOfChannels;
    
    spec.sampleRate = sampleRate;
    spec.numChannels = numberOfChannels;
    spec.maximumBlockSize = 0;
    //init. z1 and z2 and convolultion class
    z1.calloc(numberOfChannels);
    convolver.reset();
    //determine the filter coeffs if E : there is none unless Tustin.

}

void AnalogVuMeterRectifier::releaseResources()
{
}

void AnalogVuMeterRectifier::processBlock(juce::AudioBuffer<float>& buffer)
{
    spec.maximumBlockSize = buffer.getNumSamples();
    spec.numChannels = buffer.getNumChannels();
    vuMeterImpulseResponseBuffer = generateVuMeterIR(spec.numChannels, spec.maximumBlockSize);

    convolver.loadImpulseResponse(
        &vuMeterImpulseResponseBuffer,
        spec.sampleRate,
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::no,
        0,
        juce::dsp::Convolution::Normalise::no);



    convolver.prepare(spec);


    rectifiedBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
    rectifiedBuffer.clear();



    const int numOfChannels = juce::jmin(numberOfChannels, buffer.getNumChannels());
    for (int channel = 0; channel < numOfChannels; ++channel)
    {
        const float* samples = buffer.getReadPointer(channel);
        float* outSamples = rectifiedBuffer.getWritePointer(channel);



        //hard rectifying
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float u_ = samples[i]; //input voltage v(t)
            float absu_ = std::abs(u_); //rectified voltage v(t)
            outSamples[i] = absu_;
        }
        DBG("raw value after Rect = " + juce::String(outSamples[0]));

    }



        //introduce convolution to simulate capacitors STEREO
        juce::dsp::AudioBlock<float> block(rectifiedBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        convolver.process(context);


        
        
    
}

void AnalogVuMeterRectifier::reset()
{
    z1.clear(numberOfChannels);
    convolver.reset();
}




//custom built functions incl privates
juce::AudioBuffer<float> AnalogVuMeterRectifier::generateVuMeterIR(int numberOfChannels, int numberOfSamples)
{
    float Ts = 1.0f / spec.sampleRate;
    float C = AnalogVuMeterRectifier::capacitance;
    float R = AnalogVuMeterRectifier::resistance;

    auto irFormula = [C, R](float t) -> float {
        float power = (1.0f / C) * (-R - 1.0f / R) * float(t);
        float o = -exp(power) / C;
        return o;
        };


    juce::AudioBuffer<float> ir;
    ir.setSize(numberOfChannels, numberOfSamples);
    ir.clear();

    for(int ch = 0; ch < numberOfChannels; ++ch) {
        float* f = ir.getWritePointer(ch);

        for (int k = 0; k < numberOfSamples; k++) {
            f[k] = irFormula(float(k) * Ts);
        }
    }

    return ir;
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
    x_1next.free();
    x_2next.free();
    x_3next.free();
    x_4next.free();
}

void AnalogVuMeterProcessor::prepareToPlay(double sampleRate, int numberOfInputChannels, int estimatedSamplesPerBlock)
{
    spec.sampleRate =sampleRate;
    spec.numChannels = numberOfInputChannels;
    spec.maximumBlockSize = estimatedSamplesPerBlock;
    //this->expectedRequestRate = expectedRequestRate;



    ////////////////////////////////

    _buffer.setSize(numberOfInputChannels, estimatedSamplesPerBlock);
    _buffer.clear();
    bufferRectifier.prepareToPlay(sampleRate, numberOfInputChannels);

    //for ss - equation
    x_1next.calloc(numberOfInputChannels);
    x_2next.calloc(numberOfInputChannels);
    x_3next.calloc(numberOfInputChannels);
    x_4next.calloc(numberOfInputChannels);
}


void AnalogVuMeterProcessor::getNextState(
    const juce::dsp::Matrix<float>& X, const float& u,
    juce::dsp::Matrix<float>&  X_next,  double sampleRate) //perChannel
{   
    //single channel wise

    //x ->      currentStateVector
    //u ->      input "value" scalar
    //x_next->  nextStateVector
    //calculate x_next = x + dx/dt * dt
    //dt := 1/Ts
    auto dt = (float)(1.0 / sampleRate);

    X_next = ssmatrixA * X + ssmatrixB * u;

}


float AnalogVuMeterProcessor::getOutput(juce::dsp::Matrix<float>& X, const float u)
{
    //single channel wise
    auto y = ssmatrixC * X + ssmatrixD * u;
    return y(0, 0);
}

void AnalogVuMeterProcessor::feedToSteadyStateEquation(juce::AudioBuffer<float>& _buffer) //_buffer == rectified one
{
    //this must take rectified buffers "_buffer"

    const int numberOfChannels =_buffer.getNumChannels();
    const int numberOfSamples = _buffer.getNumSamples();
    const int sr = spec.sampleRate;
    const size_t sysDim = this->sysDim;

    _statebuffer.setSize(numberOfChannels, numberOfSamples);
    _statebuffer.clear();
    _outputbuffer.setSize(numberOfChannels, numberOfSamples);
    _outputbuffer.clear();
    _statebufferNext.setSize(numberOfChannels, numberOfSamples + sysDim); // + 3 for 3 prev datas
    _statebufferNext.clear();



    for (int channel = 0; channel < numberOfChannels; ++channel)
    {
        float* x_ = _statebuffer.getWritePointer(channel); //initial state vector.
        float* x_next = _statebufferNext.getWritePointer(channel);
        float* u_ = _buffer.getWritePointer(channel); //input sequence.
        float* y_ = _outputbuffer.getWritePointer(channel); //output sequence.

        double sr = spec.sampleRate;

        //assign previous 3 values
        //_statebufferNext.clear();

        x_[0] = x_1next[channel];
        x_[1] = x_2next[channel];
        x_[2] = x_3next[channel];
        x_[3] = x_4next[channel];
            //x_[4] is the "first" element.

        //simulate the system
        ssmatrixX = juce::dsp::Matrix<float>(sysDim, 1);
        ssmatrixX.clear(); // O-matrix
        ssmatrixX_next = juce::dsp::Matrix<float>(sysDim, 1, x_);
        

        for (int i = 0; i < numberOfSamples + sysDim - 1; ++i)
        {
            //calc next state -> vector<float>
            getNextState(ssmatrixX, u_[i], ssmatrixX_next, sr); //is x_next audiobuffer or float? -> it's buffer == float*

            float y = getOutput(ssmatrixX, u_[i]);

            y = y > minimalReturnValue ? y : minimalReturnValue;

            y_[i] = float(y); //this is the "meter movement"
            ssmatrixX = ssmatrixX_next;
            DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[i]));

            //_bufferLastValue.at(channel) = float(ssmat_Y(0, 0));
        }


        // remembering old sysDim amt. values before evaporating
        x_1next[channel] = x_next[numberOfSamples - 1];
        x_2next[channel] = x_next[numberOfSamples - 2];
        x_3next[channel] = x_next[numberOfSamples - 3];
        x_4next[channel] = x_next[numberOfSamples - 4];


        //DBG("raw value for VU @ ch " + juce::String(channel) + " == " + juce::String(y_[0]));
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
        }
        else {
            currentBlockIsSilent = false;
        }
    }

    //STEP 1:: rectify and damp the signal transient.
    bufferRectifier.processBlock(buffer);
    bufferRectifier.rectifiedBuffer.copyFrom(_buffer, 0); //_buffer is a rectified "buffer"

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

std::pair<float*, int> AnalogVuMeterProcessor::get_Outputbuffer(int channel)
{
    float* out = _outputbuffer.getWritePointer(channel);
    int numberOfSamples = _outputbuffer.getNumSamples();
    return std::make_pair(out, numberOfSamples);
}


void AnalogVuMeterProcessor::reset()
{
    x_1next.clear(spec.numChannels);
    x_2next.clear(spec.numChannels);
    x_3next.clear(spec.numChannels);
    x_4next.clear(spec.numChannels);
}


