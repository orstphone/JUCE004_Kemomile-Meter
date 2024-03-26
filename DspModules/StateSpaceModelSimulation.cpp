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
    timeRowVector(1, 1),
    x_simulated(1, 1),
    y_simulated(1, 1)
{
    A.clear();
    B.clear();
    C.clear();
    D.clear();
    x0.clear();
    timeRowVector.clear();
    x_simulated.clear();
    y_simulated.clear();
    y.resize(numChannels); //std::fill(y.begin(), y.end(), 0); //zeropad
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

void StateSpaceModelSimulation::setInitStateVector(std::vector<mat> initialState, int systemSize)
{
    jassert(initialState.at(0).getNumRows() == systemSize);
    this->x0 = initialState;
}

void StateSpaceModelSimulation::setInputSequence(juce::AudioBuffer<float>& buffer)
{
    //creates u[n]
    //transform audioBuffer to float vectors vector where v[ch] == corresponding data
    std::vector<std::vector<float>> inputSequenceMultiChannel;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        std::vector<float> singleChannelArray;
        const float* in = buffer.getReadPointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            singleChannelArray.push_back(in[i]);
        }
        inputSequenceMultiChannel.push_back(singleChannelArray);
    }

    this->u = inputSequenceMultiChannel;
}

void StateSpaceModelSimulation::setInputSequence(std::vector<std::vector<float>> inputSequence)
{
    //jassert(inputSequence.at(0).size() == this->u.at(0).size());
    this->u = inputSequence;
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
    

    n = A.getNumRows(); 
    m = B.getNumColumns();
    r = C.getNumRows();

    jassert(n == systemSize);

    timeSamples = buffer.getNumSamples();

    this->y_simulated = mat(r, timeSamples); this->y_simulated.clear();
    this->x_simulated = mat(n, timeSamples); this->x_simulated.clear();

    this->timeRowVector = mat(1, timeSamples);  //[1 , t] array
    for (int t = 0; t < timeSamples; t++)
    {
        timeRowVector(0, t) = t + 1;
    }
}


//stateSpaceModel for mono channel
void StateSpaceModelSimulation::runSimulation(int channel)
{
    std::vector<float> y_monoChannel;

    for (int j = 0; j < timeSamples; j++) //j is the "time" index.
    {
        auto nRow = y_simulated.getNumColumns(); //systemSize

        if (j == 0)
        {
            for (int row = 0; row < nRow; row++) //essentially when u = 0
            {
                auto _x0 = x0.at(channel);
                x_simulated(row, j) = _x0(row, j);
                y_simulated(row, j) = (C * _x0)(row, j);
                y_monoChannel.push_back(y_simulated(row, j));
            }
        }
        else
        {
            for (int row = 0; row < nRow; row++)
            {
                x_simulated(row, j) = (A * x_simulated + B * u.at(channel).at(j))(row, j);
                y_simulated(row, j) = (C * x_simulated + D * u.at(channel).at(j))(row, j);
                y_monoChannel.push_back(y_simulated(row, j));
            }
        }


    }
    y.at(channel).push_back(y_monoChannel);
}

mat StateSpaceModelSimulation::getOutputVectorVector()
{
    return y_simulated;
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
    jassert(sizeof(arr) / sizeof(float) == row * col);

    mat Out = mat(row, col, arr);
    return Out;
}







