/*
  ==============================================================================

    A simple editor for the delay plugin

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
TapeDelayAudioProcessorEditor::TapeDelayAudioProcessorEditor (TapeDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible (mGainSlider);
    addAndMakeVisible (mTimeSlider);
    addAndMakeVisible (mFeedbackSlider);

    setSize (400, 250);
}

TapeDelayAudioProcessorEditor::~TapeDelayAudioProcessorEditor()
{
}

//==============================================================================
void TapeDelayAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgoldenrod);

    g.setColour (juce::Colours::silver);
    g.setFont   (24.0f);

    auto box = getLocalBounds().reduced (20);
    box = box.withTop (box.getBottom() - 40);

    const auto width = box.getWidth() / 3;
    g.drawFittedText (TRANS ("Dry Gain"), box.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Time"), box.removeFromLeft (width), juce::Justification::centred, 1);
    g.drawFittedText (TRANS ("Feedback"), box.removeFromLeft (width), juce::Justification::centred, 1);
}

void TapeDelayAudioProcessorEditor::resized()
{
    auto box = getLocalBounds().reduced (20);
    box.removeFromBottom (40);

    const auto width = box.getWidth() / 3;
    mGainSlider.setBounds (box.removeFromLeft (width).reduced (10));
    mTimeSlider.setBounds (box.removeFromLeft (width).reduced (10));
    mFeedbackSlider.setBounds (box.removeFromLeft (width).reduced (10));
}
