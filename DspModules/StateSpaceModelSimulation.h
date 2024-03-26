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
    initstate : `x0' (dim, 1)
    stateSequenceSimulated : simulated state sequence `x` (dim, samples)
    outp.SequenceSimulated : simulated outpt sequence `y` (samples)

*/

using mat = juce::dsp::Matrix<float>;

class StateSpaceModelSimulation  : public juce::Component
{
public:
    StateSpaceModelSimulation();

    ~StateSpaceModelSimulation() override;

    //==============================================================================


    //sets x0 and u
    void setInitStateVector(juce::AudioBuffer<float> &initialBuffer, int systemSize);
    
    void setInitStateVector(std::vector<mat> initialState, int systemSize);

    void setInputSequence(juce::AudioBuffer<float>& buffer);

    void setInputSequence(std::vector<std::vector<float>> inputSequence);

    //sets A B C D and 
    void hardReset(mat A, mat B, mat C, mat D,
        juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& initialBuffer, int systemSize);



    //stateSpaceModel for mono channel
    void runSimulation(int channel);

    void concatenateOutput();

    mat getOutputVectorVector();

    //audioBuffer to n channel vector 
    std::vector<std::vector<float>> convertBufferToVector(juce::AudioBuffer<float>& buffer);

    //float arr[] for juce::dsp::Matrix constructor input argument.
    mat convertArrayTo2dMatrix(float arr[], int row, int col); //for everything else



private:



    int numChannels;
    int m, n, r, timeSamples;
    mat A, B, C, D;
    std::vector<mat> x0; //initstate vector for multichannel matrix input
    std::vector<std::vector<float>> u; //input sequence vectors for multi channel scalar input

    mat x_simulated; //statevector simulated
    mat y_simulated; //ouputmatrix simulated
    std::vector<std::vector<float>> y; //outputVector simulated
    mat timeRowVector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateSpaceModelSimulation)
};
