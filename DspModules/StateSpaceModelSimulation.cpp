/*
  ==============================================================================

    StateSpaceModelSimulation.cpp
    Created: 25 Mar 2024 8:51:42pm
    Author:  prmir

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StateSpaceModelSimulation.h"

//==============================================================================
using mat = juce::dsp::Matrix<float>;


StateSpaceModelSimulation::StateSpaceModelSimulation():
    m(0),
    n(0),
    r(0),
    timeSamples(0),
    A(1, 1),
    B(1, 1),
    C(1, 1),
    D(1, 1),
    x0(1, 1),
    inputSequence(1, 1),
    stateSequenceSimulated(1, 1),
    outputSequenceSimulated(1, 1),
    timeRowVector(1, 1)
{
    A.clear();
    B.clear();
    C.clear();
    D.clear();
    x0.clear();
    inputSequence.clear();
    stateSequenceSimulated.clear();
    outputSequenceSimulated.clear();
    timeRowVector.clear();
}

StateSpaceModelSimulation::~StateSpaceModelSimulation()
{
}

//stateSpaceModel for mono channel
void StateSpaceModelSimulation::setCoefficients(mat A, mat B, mat C, mat D, mat initState, mat inputSequenceMatrix)
{
    this->A = A;
    this->B = B;
    this->C = C;
    this->D = D;
    this->x0 = initState;
    this->inputSequence = inputSequenceMatrix;

    n = A.getNumRows();
    m = B.getNumColumns();
    r = C.getNumRows();
    timeSamples = inputSequenceMatrix.getNumColumns();

    auto outputSequenceSimulated = mat(r, timeSamples); outputSequenceSimulated.clear();
    auto stateSequenceSimulated = mat(n, timeSamples); stateSequenceSimulated.clear();
    auto timeRowVector = mat(1, timeSamples);
    //[1 , t] array
    for (int t = 0; t < timeSamples; t++)
    {
        timeRowVector(0, t) = t + 1;
    }
}

void StateSpaceModelSimulation::setInputSequence(mat initState, mat inputSequenceMatrix)
{
    this->inputSequence = inputSequenceMatrix;
    this->x0 = initState;
}


void StateSpaceModelSimulation::runSimulation()
{
    for (int j = 0; j < timeSamples; j++)
    {
        auto nRow = stateSequenceSimulated.getNumColumns();

        if (j == 0)
        {
            //StateSequenceSimulated.col(j) = x0;
            //.col(j) = C * x0;
            for (int row = 0; row < nRow; row++)
            {
                stateSequenceSimulated(row, j) = x0(row, j);
                outputSequenceSimulated(row, j) = (C * x0)(row, j);
            }
        }

        else
        {
            //simulatedOutputSequence.col(j) = A * simulatedStateSequence.col(j - 1) + B * inputSequence.col(j - 1);
            //simulatedOutputSequence.col(j) = C * simulatedStateSequence.col(j);
            for (int row = 0; row < nRow; row++)
            {
                stateSequenceSimulated(row, j) = (A * stateSequenceSimulated(row, j - 1) + B * inputSequence(row, j - 1))(row, j);
                outputSequenceSimulated(row, j) = (C * stateSequenceSimulated(row, j))(row, j);
            }
            
        }
    }
}

std::vector<mat> StateSpaceModelSimulation::convertBufferTo1dMatrix(juce::AudioBuffer<float>& buffer)
{
    std::vector<mat> matrices;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        mat singleChannelMatrix = mat(numSamples, 1);
        const float* in = buffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; i++)
        {
            singleChannelMatrix(i, 0) = in[i];
        }
        matrices.push_back(singleChannelMatrix);
    }

    return matrices;
}

mat StateSpaceModelSimulation::convertArrayTo2dMatrix(float arr[], int row, int col)
{
    jassert(sizeof(arr) / sizeof(float) == row * col);

    mat Out = mat(row, col, arr);
    return Out;
}




//==============================================================================









