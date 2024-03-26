/*
  ==============================================================================

    StateSpaceModelSimulation.cpp
    Created: 25 Mar 2024 8:51:42pm
    Author:  prmir

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StateSpaceModelSimulation.h"


using mat = juce::dsp::Matrix<float>;


StateSpaceModelSimulation::StateSpaceModelSimulation() :
    numChannels(0),
    m(0),
    n(0),
    r(0),
    timeSamples(0),
    A(1, 1),
    B(1, 1),
    C(1, 1),
    D(1, 1),
    timeRowVector(1, 1)
{
    A.clear();
    B.clear();
    C.clear();
    D.clear();
    timeRowVector.clear();
}

StateSpaceModelSimulation::~StateSpaceModelSimulation()
{
}

//sets A B C D

void StateSpaceModelSimulation::setInitStateVector(juce::AudioBuffer<float> &initialBuffer, int systemSize)
{
    //creates x0 where dim ~ [0:sysDim)
    //transform initialBuffer to matices vector where v[ch] == corresponding data
    jassert(initialBuffer.getNumSamples() == systemSize);

    std::vector<mat> initialStateMatrixMultiChannel;
    for (int channel = 0; channel < initialBuffer.getNumChannels(); ++channel)
    {
        mat singleInitChannelMatrix(systemSize, 1);
        singleInitChannelMatrix.clear();
        const float* in = initialBuffer.getReadPointer(channel);

        for (int i = 0; i < initialBuffer.getNumSamples(); ++i)
        {
            singleInitChannelMatrix(i, 0) = in[i];
        }
        initialStateMatrixMultiChannel.push_back(singleInitChannelMatrix);
    }

    this->x0 = initialStateMatrixMultiChannel;
}

void StateSpaceModelSimulation::setInitStateVector(std::vector<mat> &initialState, int systemSize)
{
    jassert(initialState.at(0).getNumRows() == systemSize);
    this->x0 = initialState;
}

//buffer to nChannel matrix vector
void StateSpaceModelSimulation::setInputSequence(juce::AudioBuffer<float>& buffer)
{
    //creates u[n] matrix u(1, samples)
    //transform audioBuffer to float vectors vector where v[ch] == corresponding data
    jassert(buffer.getNumChannels() == numChannels);

    std::vector<mat> inputSequenceMatrixVectorMultiChannel;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        mat singleChannelInputSequenceMatrix(1, numChannels);

        const float* in = buffer.getReadPointer(channel);
        std::vector<float> singleChannelArray;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            singleChannelInputSequenceMatrix(0, i) = in[i];
        }
        inputSequenceMatrixVectorMultiChannel.push_back(singleChannelInputSequenceMatrix);
    }

    this->u = inputSequenceMatrixVectorMultiChannel;
}



void StateSpaceModelSimulation::setInputSequence(std::vector<mat>& inputSequence)
{
    std::vector<mat> u_multiChannel;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        mat u_monoChannel(1, numChannels);
        auto in = inputSequence.at(channel);
        auto nrow = in.getNumRows();
        auto ncol = in.getNumColumns();
        jassert(nrow == 1);

        for (int t = 0; t < ncol; t++)
        {
            u_monoChannel(0, t) = in(0, t);
        }
        u_multiChannel.push_back(u_monoChannel);
    }
    this->u = u_multiChannel;
}


void StateSpaceModelSimulation::hardReset(mat A, mat B, mat C, mat D,
    juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& initialBuffer, int systemSize)
{
    this->A = A;
    this->B = B;
    this->C = C;
    this->D = D;

    setInitStateVector(initialBuffer, systemSize); //sets x0
    setInputSequence(buffer); //sets u
    
    numChannels = buffer.getNumChannels();
    timeSamples = buffer.getNumSamples();

    n = A.getNumRows(); 
    m = B.getNumColumns();
    r = C.getNumRows();

    jassert(n == systemSize);

    std::vector<mat> y_;
    std::vector<mat> x_;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto xMonoChannel = mat(r, timeSamples); xMonoChannel.clear();
        auto yMonoChannel = mat(r, timeSamples); yMonoChannel.clear();
        x_.push_back(xMonoChannel);
        y_.push_back(yMonoChannel);
    }
    this->y_sim = x_;
    this->x_sim = y_;


    this->timeRowVector = mat(1, timeSamples);  //[1 , t] array
    for (int t = 0; t < timeSamples; t++)
    {
        timeRowVector(0, t) = t + 1;
    }
}



//stateSpaceModel for mono channel
void StateSpaceModelSimulation::runSimulation(int channel)
{
    auto &x_ = x_sim.at(channel);
    auto &y_ = y_sim.at(channel);

    for (int j = 0; j < timeSamples; j++) //j is the "time" index.
    {
        auto nRow = y_.getNumColumns(); //systemSize

        if (j == 0)
        {
            for (int row = 0; row < nRow; row++) //essentially when u = 0
            {
                auto _x0 = x0.at(channel);
                x_(row, j) = _x0(row, j);
                y_(row, j) = (C * _x0)(row, j);
            }
        }
        else
        {
            for (int row = 0; row < nRow; row++)
            {
                x_(row, j) = (A * x_ + B * u.at(channel)(0, j))(row, j);
                y_(row, j) = (C * x_ + D * u.at(channel)(0, j))(row, j);
            }
        }
    }
}

std::vector<mat> StateSpaceModelSimulation::getSimulatedOutputMatrix()
{
    return y_sim;
}

std::vector<std::vector<float>> StateSpaceModelSimulation::convertBufferToVector(juce::AudioBuffer<float>& buffer)
{
    std::vector<std::vector<float>> multipleChannelVectors; //n-channel vector<float> arrays
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        std::vector<float> singleChannelVector;
        const float* in = buffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; i++)
        {
            singleChannelVector.push_back(in[i]);
        }
        multipleChannelVectors.push_back(singleChannelVector);

        return multipleChannelVectors;
    }
}


mat StateSpaceModelSimulation::convertArrayTo2dMatrix(float arr[], int row, int col)
{
    //jassert(sizeof(arr) / sizeof(float) == row * col);
    mat Out = mat(row, col, arr);
    return Out;
}







