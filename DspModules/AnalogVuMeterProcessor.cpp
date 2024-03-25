/*
  ==============================================================================

    AnalogVuMeterProcessor.cpp
    Created: 19 Mar 2024 4:06:42pm
    Author:  orstphone@github.com

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalogVuMeterProcessor.h"

#include <cmath>
//static member constants

const float AnalogVuMeterProcessor::minimalReturnValue = static_cast<float>(std::pow(10, - 120));



//==============================================================================
AnalogVuMeterProcessor::AnalogVuMeterProcessor() :
    resistance (660),
    capacitance(22e-6),
    outputMagnitudeCalibration(1)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    //
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(48000.0, 2, 2048);

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
    convolver.reset();


    //for state space equation
    x_1next.calloc(numberOfInputChannels);
    x_2next.calloc(numberOfInputChannels);
    x_3next.calloc(numberOfInputChannels);
    x_4next.calloc(numberOfInputChannels);


    //generating IRs
    //generateVuMeterIR(numberOfInputChannels, estimatedSamplesPerBlock);
    //DBG("generated IR successfully");


    //convolver.loadImpulseResponse(
    //    &vuMeterImpulseResponseBuffer,
    //    sampleRate,
    //    juce::dsp::Convolution::Stereo::yes,
    //    juce::dsp::Convolution::Trim::no,
    //    estimatedSamplesPerBlock, //"0" if to request the og IR size
    //    juce::dsp::Convolution::Normalise::no);


    //DBG("Loaded IR");
    //DBG("VU IRBufferSize " + juce::String(vuMeterImpulseResponseBuffer.getNumSamples()));
    //DBG("VU IRCHannelSize " + juce::String(vuMeterImpulseResponseBuffer.getNumChannels()));
    //DBG("SR = " + juce::String(spec.sampleRate));
    //DBG("NC = " + juce::String(spec.numChannels));
    //DBG("BS = " + juce::String(spec.maximumBlockSize));


    //convolver.prepare(AnalogVuMeterProcessor::spec); //this is malfunctioning
    DBG("Conv prep bypassed");


}


//==============================================================================


//custom built functions incl privates
void AnalogVuMeterProcessor::generateVuMeterIR(int numberOfChannels, int numberOfSamples)
{
    float sr = static_cast<float>(this->spec.sampleRate);
    float Ts = 1.0f / sr;
    float C = AnalogVuMeterProcessor::capacitance;
    float R = AnalogVuMeterProcessor::resistance;

    DBG("SR == " + juce::String(sr));
    DBG("Ts == " + juce::String(Ts));
    DBG("C_ == " + juce::String(C));
    DBG("R_ == " + juce::String(R));

    auto irFormula = [C, R](float t) -> float {
        float power = (1.0f / C) * (-R - 1.0f / R) * float(t);
        float o = -exp(power) / C;
        return o;
        };

    DBG("irFormula generated");

    vuMeterImpulseResponseBuffer.setSize(numberOfChannels, numberOfSamples);
    vuMeterImpulseResponseBuffer.clear();

    for (int ch = 0; ch < numberOfChannels; ++ch) {
        float* f = vuMeterImpulseResponseBuffer.getWritePointer(ch);

        for (int k = 0; k < numberOfSamples; k++) {
            f[k] = irFormula(float(k) * Ts);
        }
    }

    DBG("returning the IR buffer : ir size == " + juce::String(vuMeterImpulseResponseBuffer.getNumSamples()));
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



//==============================================================================



void AnalogVuMeterProcessor::processBlock(juce::AudioBuffer<float>& buffer)
{

    //hard rectifying
    for (size_t ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* samples = buffer.getWritePointer(ch);

        for (size_t i = 0; i < buffer.getNumSamples(); ++i)
        {
            samples[i] = std::abs(samples[i]); //rectifying input
        }

        DBG("1st raw value every block after Rect() == " + juce::String(samples[0]));
    }
    DBG("rectifying done.");


    //convolution
    //juce::dsp::AudioBlock<float> block(buffer);
    //juce::dsp::ProcessContextReplacing<float> context(block);
    //convolver.process(context);

    DBG("convolution bypassed.");


    //STEP 2:: set the number of channels to prevent EXC_BAD_ACCESS
    //when the number of channels in the buffer suddenly changes w/o calling
    //prepareToPlay() in advance.

 
    //STEP 3:: accum. the samples and use the steady state equation.
    feedToSteadyStateEquation(buffer);
    DBG("SSE done. applied to _buffer directly.");

    //STEP 4:: throw every data in _outputbuffer to the "Vu Meter"
    //from here _outputbuffer will determine "needlepoint" of the meter. : "i(t)"
    //end of sequence
}




void AnalogVuMeterProcessor::reset()
{
    x_1next.clear(spec.numChannels);
    x_2next.clear(spec.numChannels);
    x_3next.clear(spec.numChannels);
    x_4next.clear(spec.numChannels);
}

juce::AudioBuffer<float> AnalogVuMeterProcessor::getOutputBuffer()
{
    return _outputbuffer;
}
