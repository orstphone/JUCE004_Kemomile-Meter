/*
  ==============================================================================

    StateSpaceModelSimulation.h
    Created: 25 Mar 2024 8:51:42pm
    Author:  prmir

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*

    A, B, C, D state space matrices
    x0 : initialState
    inputSequence = buffer :

    n = A.rows ~ input dimension
    m = B.cols ~ state dimension
    r = C.rows ~ output dimension
    timeSamples = inputSequence.cols ~ number of time samples

*/

using mat = juce::dsp::Matrix<float>;

class StateSpaceModelSimulation  : public juce::Component
{
public:
    StateSpaceModelSimulation();

    ~StateSpaceModelSimulation() override;

    //==============================================================================

    void setCoefficients(mat A, mat B, mat C, mat D, mat initState, mat inputSequenceMatrix);

    void setInputSequence(mat initState, mat inputSequenceMatrix);
    //std::tuple<mat, mat, mat> getStateOutputTime();
    
    //void saveData();

    //mat openData();

    //void openFromFile();
    // 
    //stateSpaceModel for mono channel
    void runSimulation();

    std::vector<mat> convertBufferTo1dMatrix(juce::AudioBuffer<float>& buffer);

    mat convertArrayTo2dMatrix(float arr[], int row, int col);


private:
    int m, n, r, timeSamples;
    mat A, B, C, D;
    mat x0;
    mat inputSequence;
    mat stateSequenceSimulated;
    mat outputSequenceSimulated;
    mat timeRowVector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateSpaceModelSimulation)
};
