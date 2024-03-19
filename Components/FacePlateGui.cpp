/*
  ==============================================================================

    FacePlateGui.cpp
    Created: 19 Mar 2024 2:04:36am
    Author:  Decca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FacePlateGui.h"

#include <vector>

//==============================================================================
Gui::FacePlateGui::FacePlateGui()
{
}

Gui::FacePlateGui::~FacePlateGui()
{
}

void Gui::FacePlateGui::loadAssets()
{
     imageFaceplateCanvas = juce::ImageCache::getFromMemory(BinaryData::FaceplateCanvas_png, BinaryData::FaceplateCanvas_pngSize);
     imageGlassSheenReflection = juce::ImageCache::getFromMemory(BinaryData::glassSheen_png, BinaryData::glassSheen_pngSize);
     imageNumericPrintedValuesVu = juce::ImageCache::getFromMemory(BinaryData::text_Numerics_png, BinaryData::text_Numerics_pngSize);
     imageNumericPrintedLr = juce::ImageCache::getFromMemory(BinaryData::text_LR_png, BinaryData::text_LR_pngSize);
     imageNumericPrintedVu = juce::ImageCache::getFromMemory(BinaryData::text_VU_png, BinaryData::text_VU_pngSize);

     int numberOfAssets = 8;
     for (int i = 0; i < numberOfAssets; i++)
     {

     }

}



void Gui::FacePlateGui::paint (juce::Graphics& g)
{

}

void Gui::FacePlateGui::paintOverChildren(juce::Graphics& g)
{
    //g.drawImage(imageComponentImageFacePlateBody.getImage(), getLocalBounds().toFloat());
    //g.drawImage(imageComponentImageFaceDisplayBody.getImage(), getLocalBounds().toFloat());
    //g.drawImage(imageComponentImageFaceGuardRail.getImage(), getLocalBounds().toFloat());
    //g.drawImage(imageComponentImageHorizontalStripDisplay.getImage(), getLocalBounds().toFloat());
    //g.drawImage(imageComponentImageGlassSheenReflection.getImage(), getLocalBounds().toFloat());
    //g.drawImage(imageComponentImageNumericPrintedValuesVu.getImage(), getLocalBounds().toFloat());
}

void Gui::FacePlateGui::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    //imageComponentImageFacePlateBody.setBounds(0, 0, 1012, 230);
    //imageComponentImageFaceDisplayBody.setBounds(88, 0, 527, 230);
    //imageComponentImageFaceGuardRail.setBounds(10, 17, 33, 196);
    //imageComponentImageHorizontalStripDisplay.setBounds(657, 174, 167, 23);
    //imageComponentImageGlassSheenReflection.setBounds(137, 0, 186, 230);
    //imageComponentImageNumericPrintedValuesVu.setBounds(114, 105, 468, 21);
}

juce::Image Gui::FacePlateGui::getFaceplate() const
{
    return imageFaceplateCanvas; 
}

juce::Image Gui::FacePlateGui::getChannelInfos() const
{
    return imageNumericPrintedLr;
}

juce::Image Gui::FacePlateGui::getMetricsVu() const
{
    return imageNumericPrintedValuesVu;
}

juce::Image Gui::FacePlateGui::getIconVu() const
{
    return imageNumericPrintedVu;
}
