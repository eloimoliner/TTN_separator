/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ttn_separatorAudioProcessorEditor::Ttn_separatorAudioProcessorEditor (Ttn_separatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible (mGainSlider);
    addAndMakeVisible (mGainTSlider);
    addAndMakeVisible (mGainSSlider);
    addAndMakeVisible (mGainNSlider);
    addAndMakeVisible (mNfactorSlider);
    addAndMakeVisible (mltSlider);
    addAndMakeVisible (mlfSlider);
    setSize (600, 500);

}

Ttn_separatorAudioProcessorEditor::~Ttn_separatorAudioProcessorEditor()
{
}

//==============================================================================
void Ttn_separatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);   
    g.fillAll (juce::Colours::darkolivegreen);

    g.setFont(juce::Font("Comic Sans MS", 20.0f,juce::Font::plain));
    g.setColour (juce::Colours::black);

    auto box = getLocalBounds().reduced (20);
    const auto width = box.getWidth() / 4;
   const auto height = box.getHeight() / 2;
   auto boxup = box.removeFromTop(height);
    box = box.withTop (box.getBottom() - 40);

    boxup = boxup.withTop (boxup.getBottom() - 40);


    g.drawFittedText (TRANS ("Transient gain"), boxup.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Tonal gain"), boxup.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Noisy gain"), boxup.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Total Gain"), boxup.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Time filter"), box.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Freq. filter"), box.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Noisy factor"), box.removeFromLeft (width), juce::Justification::centred, 1);

}

void Ttn_separatorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..    auto box = getLocalBounds().reduced (20);
    auto box = getLocalBounds().reduced (20);
   const auto width = box.getWidth() / 4;
   const auto height = box.getHeight() / 2;
   auto boxup = box.removeFromTop(height);

    boxup.removeFromBottom (40);
    box.removeFromBottom (40);

    mGainTSlider.setBounds (boxup.removeFromLeft (width).reduced (10));
    mGainSSlider.setBounds (boxup.removeFromLeft (width).reduced (10));
    mGainNSlider.setBounds (boxup.removeFromLeft (width).reduced (10));
    mGainSlider.setBounds (boxup.removeFromLeft (width).reduced (10));
    mltSlider.setBounds (box.removeFromLeft (width).reduced (10));
    mlfSlider.setBounds (box.removeFromLeft (width).reduced (10));
    mNfactorSlider.setBounds (box.removeFromLeft (width).reduced (10));


}
