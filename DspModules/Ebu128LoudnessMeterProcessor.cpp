/*
  ==============================================================================

    Ebu128LoudnessMeterProcessor.cpp
    Created: 14 Mar 2024 12:11:30am
    Author:  Decca (https://github.com/deccauvw)

    reference
    https://github.com/unusual-audio/loudness-meter/tree/main/Source/Vendor/Ebu128LoudnessMeter
  ==============================================================================
*/

#include <JuceHeader.h>
#include "Ebu128LoudnessMeterProcessor.h"

    //static member constants
const float Ebu128LoudnessMeterProcessor::minimalReturnValue = -300.0f;
const double Ebu128LoudnessMeterProcessor::absoluteThreshold = 70.0;

    //spec for the histograms
const double Ebu128LoudnessMeterProcessor::lowestBlockLoudnessToConsider = -100.0; //LUFS



//==============================================================================
Ebu128LoudnessMeterProcessor::Ebu128LoudnessMeterProcessor()
    : bufferForMeasurement(2, 2048),
    // Initialise the buffer with some common values.
    // Also initialise the two filters with the coefficients for a sample
    // rate of 44100 Hz. These values are given in the ITU-R BS.1770-2.
    preFilter(
        1.53512485958697,  // b0
        -2.69169618940638, // b1
        1.19839281085285,  // b2
        -1.69065929318241, // a1
        0.73248077421585), // a2
    revisedLowFrequencyBCurveFilter(
        1.0,               // b0
        -2.0,              // b1
        1.0,               // b2
        -1.99004745483398, // a1
        0.99007225036621), // a2
    numberOfBins(0),
    numberOfSamplesPerBin(0),
    numberOfSamplesInAllBins(0),
    numberOfBinsToCover400ms(0),
    numberOfSamplesIn400ms(0),
    numberOfBinsToCover100ms(0),
    numberOfBinsSinceLastGateMeasurementForI(1),
    // millisecondsSinceLastGateMeasurementForLRA (0),
    measurementDuration(0),
    numberOfBlocksToCalculateRelativeThreshold(0),
    sumOfAllBlocksToCalculateRelativeThreshold(0.0),
    relativeThreshold(absoluteThreshold),
    numberOfBlocksToCalculateRelativeThresholdLRA(0),
    sumOfAllBlocksToCalculateRelativeThresholdLRA(0.0),
    relativeThresholdLRA(absoluteThreshold),
    integratedLoudness(minimalReturnValue),
    shortTermLoudness(minimalReturnValue),
    maximumShortTermLoudness(minimalReturnValue),
    momentaryLoudness(minimalReturnValue),
    maximumMomentaryLoudness(minimalReturnValue),
    loudnessRangeStart(minimalReturnValue),
    loudnessRangeEnd(minimalReturnValue),
    freezeLoudnessRangeOnSilence(false),
    currentBlockIsSilent(false)
{
    DBG("The longest possible measurement until a buffer overflow = "
        + juce::String(INT_MAX / 10. / 3600. / 365.) + " years");

    // If this class is used without caution and processBlock
    // is called before prepareToPlay, divisions by zero
    // might occure. E.g. if numberOfSamplesInAllBins = 0.
    //
    // To prevent this, prepareToPlay is called here with
    // some arbitrary arguments.
    prepareToPlay(44100.0, 2, 512, 20);
}


Ebu128LoudnessMeterProcessor::~Ebu128LoudnessMeterProcessor()
{
}

void Ebu128LoudnessMeterProcessor::prepareToPlay(
    double sampleRate,
    int numberOfInputChannels,
    int estimatedSamplesPerBlock,
    int expectedRequestRate)
{
    //resize teh buffer
    bufferForMeasurement.setSize(numberOfInputChannels, estimatedSamplesPerBlock);

    //set up the two filters for the K-Filtering.
    preFilter.prepareToPlay(sampleRate, numberOfInputChannels);
    revisedLowFrequencyBCurveFilter.prepareToPlay(sampleRate, numberOfInputChannels);

    //modify the expectedRequestRate if needed.
    //it needs to be at least 10 and a multiple of 10 because
    //exactly every .1 second a gating block needs to be measured
    //for the integrated loudness measurement.

    if (expectedRequestRate < 10)
        expectedRequestRate = 10;
    else
    {
        expectedRequestRate = (((expectedRequestRate - 1) / 10) + 1) * 10;
        //examples
        //19 -> 20
        //20 -> 20
        //21 -> 30
    }

    //it also needs to be a divisor of the SampleRate for accurate
    //MS values (the I-loudness wouldn't be affected by this inaccuracy.
    while (int(sampleRate) % expectedRequestRate != 0)
    {
        expectedRequestRate += 10;
        if (expectedRequestRate > sampleRate / 2)
        {
            expectedRequestRate = 10;
            DBG("Not possible to make expectedRequestRate a multiple of 10 and a divisor of the samplerate");
            break;
        }
    }

    DBG("expectedRequestRate = " + juce::String(expectedRequestRate));

    //figure out how many b ins are needed.
    const int timeOfAccumulationForShortTerm = 3; //sec
        //needed for the short term loudness measurement.
    
    numberOfBins = expectedRequestRate * timeOfAccumulationForShortTerm;
    numberOfSamplesPerBin = int(sampleRate / expectedRequestRate);
    numberOfSamplesInAllBins = numberOfBins * numberOfSamplesPerBin;

    numberOfBinsToCover100ms = int(0.1 * expectedRequestRate);
    DBG("NumberOfBinsToCover100ms " + juce::String(numberOfBinsToCover100ms));
    numberOfBinsToCover400ms = int(0.4 * expectedRequestRate);
    DBG("numberOfBinsToCover400ms = " + juce::String(numberOfBinsToCover400ms));
    numberOfSamplesIn400ms = numberOfBinsToCover400ms * numberOfSamplesPerBin;

    currentBin = 0;
    numberOfSamplesInTheCurrentBin = 0;
    numberOfBinsSinceLastGateMeasurementForI = 1;
    //millisecondsSinceLastGateMeasurementForLRA = 0;
    measurementDuration = 0;

    //init. the bins.
    bin.assign(numberOfInputChannels, vector<double>(numberOfBins, 0.0));

    averageOfTheLast3s.assign(numberOfInputChannels, 0.0);
    averageOfTheLast400ms.assign(numberOfInputChannels, 0.0);

    //init. the channel weighting.
    channelWeighting.clear();
    for (int k = 0; k != numberOfInputChannels; ++k)
    {
        if (k == 3 || k == 4)
        {
            //the left and right surround channels have a heigher weight
            //because they seem louder to the human ear.
            channelWeighting.push_back(1.41);
        }
        else
        {
            channelWeighting.push_back(1.0);
        }

    }
        //momentary loudness for the individual channels.
       momentaryLoudnessForIndividualChannels.assign(numberOfInputChannels, minimalReturnValue);
    reset();
}

void Ebu128LoudnessMeterProcessor::processBlock(juce::AudioSampleBuffer& buffer)
{
    //copy the buffer, such that all upcoming calc. won't affect
    //the audio output. we want the oudio output to be exactly the same as the input

    bufferForMeasurement = buffer; // this copies the audio to another memory location using memcpy.

    if (freezeLoudnessRangeOnSilence)
    {
        //detect if the block is silent-----------------------------

        const float silenceThreshold = std::pow(10, 0.1 * -120);

        const float magnitude = buffer.getMagnitude(0, buffer.getNumSamples());
        if (magnitude < silenceThreshold)
        {
            currentBlockIsSilent = true;
            DBG("silence detected.");
        }
        else
        {
            currentBlockIsSilent = false;
        }

    }
    //STEP 1 : K-Weighted Filter-----------------------------

    //apply the pre-filter
    //used to account for the acoustic effects on the head.
    //this is the first part of the "K-weighted" filtering.
    preFilter.processBlock(bufferForMeasurement);

    //apply the RLB filter (simple highpass filter)
    //this is the second part of the K-weighted filtering.
    //accordance to ITU-R BS.1770-2
    revisedLowFrequencyBCurveFilter.processBlock(bufferForMeasurement);


    //STEP 2 : Mean Square-----------------------------
    for (int k = 0; k != bufferForMeasurement.getNumChannels(); ++k)
    {
        float* theKthChannelData = bufferForMeasurement.getWritePointer(k);

        for (int i = 0; i != bufferForMeasurement.getNumSamples(); ++i)
        {
            theKthChannelData[i] = theKthChannelData[i] * theKthChannelData[i];
        }
    }

    //intermezzo : set the number of channels.
    //to prevent EXC_BAD_ACCESS when the number of channels in the buffer
    //suddenly changes without calling prepareToPlay() in advance.

    const int numberOfChannels = juce::jmin(
        bufferForMeasurement.getNumChannels(),
        int(bin.size()),
        int(averageOfTheLast400ms.size()),
        juce::jmin(int(averageOfTheLast3s.size()), int(channelWeighting.size()))
    );

    jassert(bufferForMeasurement.getNumChannels() == int(bin.size()));
    jassert(bufferForMeasurement.getNumChannels() == int(averageOfTheLast400ms.size()));
    jassert(bufferForMeasurement.getNumChannels() == int(averageOfTheLast3s.size()));
    jassert(bufferForMeasurement.getNumChannels() == int(channelWeighting.size()));


    //STEP 3 : Accumulate the samples and put the sums into the right bins.---------

        //if the new samples from the bufferForMeasurement can all be added to the same bin...
    if (numberOfSamplesInTheCurrentBin + bufferForMeasurement.getNumSamples() < numberOfSamplesPerBin)
    {
        for (int k = 0; k != numberOfChannels; ++k)
        {
            float* bufferOfChannelK = bufferForMeasurement.getWritePointer(k);
            double& theBinToSumTo = bin[k][currentBin];

            for (int i = 0; i != bufferForMeasurement.getNumSamples(); ++i)
            {
                theBinToSumTo += bufferOfChannelK[i];
            }
        }
        numberOfSamplesInTheCurrentBin += bufferForMeasurement.getNumSamples();
    }
    else        //if the new samples are split up between the two or more bins.
    {
        int positionInBuffer = 0;
        bool bufferStillContainsSamples = true;
        while (bufferStillContainsSamples)
        {
            //Fig. out if the remaining samples in the buffer can all be
            //accumulated to the current bin.
            const int numberOfSamplesLeftInTheBuffer = bufferForMeasurement.getNumSamples() - positionInBuffer;
            int numberOfSamplesToPutIntoTheCurrentBin;

            if (numberOfSamplesLeftInTheBuffer < numberOfSamplesPerBin - numberOfSamplesInTheCurrentBin)
            {
                //case 1: Partially fill a bin by using all the samples left in the buffer
                //if all the samples from the buffer can be added to the current bin.
                numberOfSamplesToPutIntoTheCurrentBin = numberOfSamplesLeftInTheBuffer;
                bufferStillContainsSamples = false;
            }
            else
            {
                //case 2: Completely fill a bin
                //most likely the buffer will still contain some samples for the next bin.
                //accumulate samples to the current bin until it is full.
                numberOfSamplesToPutIntoTheCurrentBin = numberOfSamplesPerBin - numberOfSamplesInTheCurrentBin;
            }

            //Add the samples to the bin.
            for (int k = 0; k != numberOfChannels; ++k)
            {
                float* bufferOfChannelK = bufferForMeasurement.getWritePointer(k);
                double& theBinToSumTo = bin[k][currentBin];
                for (int i = positionInBuffer; i != positionInBuffer + numberOfSamplesToPutIntoTheCurrentBin; ++i)
                {
                    theBinToSumTo += bufferOfChannelK[i];
                }
            }
            numberOfSamplesInTheCurrentBin += numberOfSamplesToPutIntoTheCurrentBin;

            //if there are some samples left in the buffer
            //...A in has just been completely filled (case 2 above)..
            if (bufferStillContainsSamples)
            {
                positionInBuffer = positionInBuffer + numberOfSamplesToPutIntoTheCurrentBin;
                //We've completely filled the bin
                //this is the moment the larger sums need to be updated
                for (int k = 0; k != numberOfChannels; ++k)
                {
                    double sumOfAllBins = 0.0;
                    //which covers the last 3s
                    for (int b = 0; b != numberOfBins; ++b)
                        sumOfAllBins += bin[k][b];

                    averageOfTheLast3s[k] = sumOfAllBins / numberOfSamplesInAllBins;

                    //short term loudness//===================
                    {
                        double weightedSum = 0.0;
                        for (int k = 0; k != int(averageOfTheLast400ms.size()); ++k)
                            weightedSum += channelWeighting[k] * averageOfTheLast400ms[k];

                        if (weightedSum > 0.0)
                            //this refers to equation 2 in ITU-R BS.1770-2
                            momentaryLoudness = juce::jmax(float(-0.691 + 10. * std::log10(weightedSum)), minimalReturnValue);
                        else
                            //since returning a value of -NaN most probably would lead to malfunction
                            momentaryLoudness = minimalReturnValue;

                        //Maximum
                        if (momentaryLoudness > maximumMomentaryLoudness)
                            maximumMomentaryLoudness = momentaryLoudness;
                    }
                }

                //INTEGRATED LOUDNESS ==============================================================
                //for the ingegrated loudness mes. we h ave to observe a 
                //gating window of length 400ms every 100ms.
                //we call this window gating block, according to BS.1770-3
                if (numberOfBinsSinceLastGateMeasurementForI != numberOfBinsToCover100ms)
                    ++numberOfBinsSinceLastGateMeasurementForI;
                else
                {
                    //every 100ms this section is reached
                    //the next time the condition above is checked, one bin has already been filled.
                    //therefore this is set to 1( not to 0).
                    numberOfBinsSinceLastGateMeasurementForI = 1;

                    ++measurementDuration;

                    //fig. out if the current 400ms gated window(loudnessOfCurrentBlock =) l_j > /gamma_a
                    //see ITU-R BS.1770-3 equation

                    //calc. the weighted sum of current block
                    // (in 120725_integrated_loudness_revisited.tif, I call
                    // this s_j)
                    double weightedSumOfCurrentBlock = 0.0;
                    for (int k = 0; k != numberOfChannels; ++k)
                    {
                        weightedSumOfCurrentBlock += channelWeighting[k] * averageOfTheLast400ms[k];
                    }

                    //calc. the j'th gating block loudness l_j
                    const double loudnessOfCurrentBlock = -0.691 + 10. * std::log10(weightedSumOfCurrentBlock);

                    if (loudnessOfCurrentBlock > absoluteThreshold)
                    {
                        //recalc. the relative threshold --
                        ++numberOfBlocksToCalculateRelativeThreshold;
                        sumOfAllBlocksToCalculateRelativeThreshold += weightedSumOfCurrentBlock;

                        //According to the definition of the relative.
                        //threshold in ITU-R BS.1770-3, page 6.
                        relativeThreshold = -10.691 + 10.0 * std::log10(sumOfAllBlocksToCalculateRelativeThreshold / numberOfBlocksToCalculateRelativeThreshold);
                    }

                    //add the loudness of the current block to the histogram
                    if (loudnessOfCurrentBlock > lowestBlockLoudnessToConsider)
                    {
                        histogramOfBlockLoudness[round(loudnessOfCurrentBlock * 10.0)] += 1;
                        //with the + 0.5 the value is rounded to the closest bin.
                        //with + 0.5 : -22.26 ->
                    }

                    //determine the integrated loudness -------------------------------------
                    //it's here instead inside of the getIntegratedLoudness() function.
                    //because here it's only calculated 10 times a second.
                    //getIntegratedLoudness is called at the refreshrate of the GUI,
                    //which is higher (20/s)

                    if (histogramOfBlockLoudness.size() > 0)
                    {
                        const double biggestLoudnessInHistogram = (--histogramOfBlockLoudness.end())->first * 0.1;
                        //DEB("biggestLoudnessInHistogram = " + String(biggestLoudnessInHistogram))
                        if (relativeThreshold < biggestLoudnessInHistogram)
                        {
                            int closestBinAboveRelativeThresholdKey = int(relativeThreshold * 10.0);
                            while (histogramOfBlockLoudness.find(closestBinAboveRelativeThresholdKey) == histogramOfBlockLoudness.end())
                            {//if not found
                                closestBinAboveRelativeThresholdKey++; //go 0.1LU higher
                            }

                            int nrOfAllBlocks = 0;
                            double sumForIntegratedLoudness = 0.0;

                            for (map<int, int>::iterator currentBin = histogramOfBlockLoudness.find(closestBinAboveRelativeThresholdKey);
                                currentBin != histogramOfBlockLoudness.end();
                                ++currentBin)
                            {
                                const int nrOfBlocksInBin = currentBin->second;
                                nrOfAllBlocks += nrOfBlocksInBin;

                                const double weightedSumOfCurrentBin = pow(10.0, (currentBin->first * 0.1 + 0.691) * 0.1);
                                sumForIntegratedLoudness += nrOfBlocksInBin * weightedSumOfCurrentBin;
                            }

                            if (nrOfAllBlocks > 0) //nrOfAllBlocks > 0 => sumForIntegratedLoudness > 0.0
                            {
                                integratedLoudness = float(-0.691 + 10. * std::log10(sumForIntegratedLoudness / nrOfAllBlocks));
                            }
                            else
                            {
                                integratedLoudness = minimalReturnValue;
                            }
                        }
                    }
                    // Loudness range
                    // ==============
                    // According to the specification, at least every 1000ms
                    // a new 3s long LRA block needs to be started.
                    //
                    // Here, an interval of 100ms is used.
                    // This makes measurement results equal (or very similar)
                    // to ffmpeg/ebur128 and Nugen VisLM2.

                    // if (millisecondsSinceLastGateMeasurementForLRA != 500)
                    //     millisecondsSinceLastGateMeasurementForLRA += 100;
                    // else

                        // Every second this section is reached.
                        // This results in an overlap of the 3s gates of exactly
                        // 2/3, the minimum requirement.

                    //    millisecondsSinceLastGateMeasurementForLRA = 100;


                        // This is very similar to the above code for the integrated loudness.
                        // (But distinct enough to not put it into a single function/object.)

                        // Calculate the weighted sum of the current block,
                        // (in 120725_integrated_loudness_revisited.tif, I call
                        // this s_j)
                        // Using an analysis-window of 3 seconds, as specified in
                        // EBU 3342-2011.
                    double weightedSumOfCurrentBlockLRA = 0.0;

                    for (int k = 0; k != numberOfChannels; ++k)
                    {
                        weightedSumOfCurrentBlockLRA += channelWeighting[k] * averageOfTheLast3s[k];
                    }

                    //calc. the j'th gating block loudness l_j
                    const double loudnessOfCurrentBlockLRA = -0.691 + 10.0 * std::log10(weightedSumOfCurrentBlockLRA);

                    if (loudnessOfCurrentBlockLRA > absoluteThreshold)
                    {
                        //recalc. the relative thres. for LRA
                        sumOfAllBlocksToCalculateRelativeThresholdLRA += weightedSumOfCurrentBlockLRA;

                        //according to the def. of the relative
                        //thres in ITU-R BS.1770-3, page 6
                        //-20LU as described in EBU 3342-2011
                        relativeThresholdLRA = -20.691 + 10.0 * std::log10(sumOfAllBlocksToCalculateRelativeThresholdLRA / numberOfBlocksToCalculateRelativeThresholdLRA);
                    }

                    //determine the loudness range---
                    //
                    //it is here instead inside of the getter functions.
                    //because here it's only calc.'d once a second.
                    //the getter functions are called at the refreshrate of the GUI*,
                    //which is higher (e.g. 20/sec)

                    if (histogramOfBlockLoudnessLRA.size() > 0) {
                        const double biggestLoudnessInHistogramLRA = (--histogramOfBlockLoudnessLRA.end())->first * 0.1;
                        //DEB("biggestLoudnessInHistogramLRA = " + String(biggestLoudnessInHistogramLRA))
                        if (relativeThresholdLRA < biggestLoudnessInHistogramLRA)
                        {
                            int closestBinAboveRelativeThresholdKeyLRA = int(relativeThresholdLRA * 10.0);
                            while (histogramOfBlockLoudnessLRA.find(closestBinAboveRelativeThresholdKeyLRA) == histogramOfBlockLoudnessLRA.end())
                            {
                                closestBinAboveRelativeThresholdKeyLRA++;
                            }

                            //fig.out the number of blocks above the relativeThresholdLRA----------------------
                            int numberOfBlocksLRA = 0;

                            for (map<int, int>::iterator currentBinLRA = histogramOfBlockLoudnessLRA.find(closestBinAboveRelativeThresholdKeyLRA);
                                currentBinLRA != histogramOfBlockLoudnessLRA.end();
                                ++currentBinLRA)
                            {
                                const int nrOfBlocksInBinLRA = currentBinLRA->second;
                                numberOfBlocksLRA += nrOfBlocksInBinLRA;
                            }

                            //fig. out the lower bound(start) of the loudness range.
                            //=================
                            map<int, int>::iterator startBinLRA = histogramOfBlockLoudnessLRA.find(closestBinAboveRelativeThresholdKeyLRA);
                            int numberOfBlocksBelowStartBinLRA = startBinLRA->second;

                            while (double(numberOfBlocksBelowStartBinLRA) < 0.10 * double(numberOfBlocksLRA))
                            {
                                ++startBinLRA;
                                numberOfBlocksBelowStartBinLRA += startBinLRA->second;
                            }
                            //DEB("numberOfBlocks = " + String(numberOfBlocksLRA))
                            //DEB("numberOfBlocksBelowStartBinLRA = " + String(numberOfBlocksBelowStartBinLRA))
                            ++startBinLRA;

                            if (!(freezeLoudnessRangeOnSilence && currentBlockIsSilent))
                                loudnessRangeStart = startBinLRA->first * 0.1;
                            //DEB("LRA starts at " + String(loudnessRangeStart))
                        //else:
                        //holding the loudnessRangeStartOnSilence
                        //helps reading it after the end of an audio
                        //region or if the daw has justr been stopped.
                        //the mes. does not get interrupted by this! it's only a temp. freeze.

                        //fig out the upper bound(end) of the loudness range.
                            map<int, int>::iterator endBinLRA = --(histogramOfBlockLoudnessLRA.end());
                            int numberOfBlocksAboveEndBinLRA = endBinLRA->second;

                            while (double(numberOfBlocksAboveEndBinLRA) < 0.05 * double(numberOfBlocksLRA))
                            {
                                --endBinLRA;
                                numberOfBlocksAboveEndBinLRA += endBinLRA->second;
                            }

                            if (!(freezeLoudnessRangeOnSilence && currentBlockIsSilent))
                                loudnessRangeEnd = endBinLRA->first * 0.1;
                            //DEB("LRA ends at " + String(loudnessRangeEnd))
                            // Else:
                            // Holding the loudnessRangeEnd on silence
                            // helps reading it after the end of an audio
                            // region or if the DAW has just been stopped.
                            // The measurement does not get interrupted by
                            // this! It's only a temporary freeze.

                            // DEB("LRA = " + String (loudnessRangeEnd - loudnessRangeStart))
                        }
                    }
                }
            }

            //move on to the next bin
            currentBin = (currentBin + 1) % numberOfBins;
            //set it to zero
            for (int k = 0; k != numberOfChannels; ++k)
            {
                bin[k][currentBin] = 0.0;
            }
            numberOfSamplesInTheCurrentBin = 0;
        }

    }
}

//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================
//getters and setters========================================================================================




float Ebu128LoudnessMeterProcessor::getMaximumShortTermLoudness() const
{
    return shortTermLoudness;
}

float Ebu128LoudnessMeterProcessor::getMaximumShortTermLoudness() const
{
    return maximumShortTermLoudness;
}

vector<float>& Ebu128LoudnessMeterProcessor::getMomentaryLoudnessForIndividualChannels()
{
    //calculate the momentary loudness
    for (int k = 0; k != int(momentaryLoudnessForIndividualChannels.size()); ++k)
    {
        float kthChannelMomentaryLoudness = minimalReturnValue;

        if (averageOfTheLast400ms[k] > 0.0f)
        {
            //this revers to equation 2 in itu-r bs.1770-2
            kthChannelMomentaryLoudness = juce::jmax(float(-0.691 + 10. * std::log10(averageOfTheLast400ms[k])), minimalReturnValue);
        }

        momentaryLoudnessForIndividualChannels[k] = kthChannelMomentaryLoudness;
    }

    return momentaryLoudnessForIndividualChannels;
}

float Ebu128LoudnessMeterProcessor::getMomentaryLoudness() const
{
    return momentaryLoudness;
}

float Ebu128LoudnessMeterProcessor::getMaximumMomentaryLoudness() const
{
    return maximumMomentaryLoudness;
}

float Ebu128LoudnessMeterProcessor::getIntegratedLoudness() const
{
    return integratedLoudness;
}

float Ebu128LoudnessMeterProcessor::getLoudnessRangeStart() const
{
    return loudnessRangeStart;
}

float Ebu128LoudnessMeterProcessor::getLoudnessRangeEnd() const
{
    return loudnessRangeEnd;
}

float Ebu128LoudnessMeterProcessor::getLoudnessRange() const
{
    return loudnessRangeEnd - loudnessRangeStart;
}

float Ebu128LoudnessMeterProcessor::getMeasurementDuration() const
{
    return measurementDuration * 0.1f;
}

void Ebu128LoudnessMeterProcessor::setFreezeLoudnessRangeOnSilence(bool freeze)
{
    freezeLoudnessRangeOnSilence = freeze;
}

void Ebu128LoudnessMeterProcessor::reset()
{
    //the bins
    //it is important to use assign() : replace all values and not resize (only set new elm. to the provided value)
    bin.assign(bin.size(), vector<double>(numberOfBins, 0.0));

    //To ensure the returned momentary and short term loudness are at its
    //minimum, even if no audio is processed at the moment.
    averageOfTheLast3s.assign(averageOfTheLast400ms.size(), 0.0);
    averageOfTheLast400ms.assign(averageOfTheLast400ms.size(), 0.0);

    measurementDuration = 0;

    //momentary loudness for the indivual tracks.
    momentaryLoudnessForIndividualChannels.assign(momentaryLoudnessForIndividualChannels.size(), minimalReturnValue);

    //integrated loudness
    numberOfBinsSinceLastGateMeasurementForI = 1;
    numberOfBlocksToCalculateRelativeThreshold = 0;
    sumOfAllBlocksToCalculateRelativeThreshold = 0.0;
    relativeThreshold = absoluteThreshold;

    histogramOfBlockLoudness.clear();
    integratedLoudness = minimalReturnValue;

    //loudnessRange
    numberOfBlocksToCalculateRelativeThresholdLRA = 0;
    sumOfAllBlocksToCalculateRelativeThresholdLRA = 0.0;
    relativeThresholdLRA = absoluteThreshold;

    histogramOfBlockLoudnessLRA.clear();
    loudnessRangeStart = minimalReturnValue;
    loudnessRangeEnd = minimalReturnValue;

    //shortTermLoudness
    shortTermLoudness = minimalReturnValue;
    maximumShortTermLoudness = minimalReturnValue;

    //momentary loudness
    momentaryLoudness = minimalReturnValue;
    maximumMomentaryLoudness = minimalReturnValue;
}

int Ebu128LoudnessMeterProcessor::round(double d)
{
    //for a negative d, int(d) will choose the next higher number number
    return (d > 0.0) ? int(d + 0.5) : int(d - 0.5);
}