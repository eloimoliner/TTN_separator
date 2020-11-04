/*
  ==============================================================================

    A simple editor for the delay plugin

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class TapeDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TapeDelayAudioProcessorEditor (TapeDelayAudioProcessor&);
    ~TapeDelayAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TapeDelayAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDelayAudioProcessorEditor)

    juce::Slider mGainSlider      { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };
    juce::Slider mTimeSlider      { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };
    juce::Slider mFeedbackSlider  { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };

    juce::AudioProcessorValueTreeState::SliderAttachment mGainAttachment      { processor.getValueTreeState(), TapeDelayAudioProcessor::paramGain,     mGainSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mTimeAttachment      { processor.getValueTreeState(), TapeDelayAudioProcessor::paramTime,     mTimeSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mFeedbackAttachment  { processor.getValueTreeState(), TapeDelayAudioProcessor::paramFeedback, mFeedbackSlider };
};
