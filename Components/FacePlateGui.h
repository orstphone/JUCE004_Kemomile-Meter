/*
  ==============================================================================

    FacePlateGui.h
    Created: 19 Mar 2024 2:04:36am
    Author:  Decca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
namespace Gui {
    class FacePlateGui : public juce::Component
    {
    public:
        FacePlateGui();
        ~FacePlateGui() override;

        void loadAssets();


        void paint(juce::Graphics& g) override;
        void paintOverChildren(juce::Graphics& g) override;
        void resized() override;


        ///assets getters     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        juce::Image getFaceplate() const;
        juce::Image getChannelInfos() const;
        juce::Image getMetricsVu() const;
        juce::Image getIconVu() const;



    private:
        juce::Image imageFaceplateCanvas;
        juce::Image imageGlassSheenReflection;
        juce::Image imageNumericPrintedValuesVu;
        juce::Image imageNumericPrintedLr;
        juce::Image imageNumericPrintedVu;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FacePlateGui)
    };

}