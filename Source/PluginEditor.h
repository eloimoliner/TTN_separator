/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Ttn_separatorAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Ttn_separatorAudioProcessorEditor (Ttn_separatorAudioProcessor&);
    ~Ttn_separatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Ttn_separatorAudioProcessor& audioProcessor;
    juce::Slider mGainSlider      { juce::Slider::LinearVertical,  juce::Slider::TextBoxBelow };
    juce::Slider mGainTSlider      { juce::Slider::LinearVertical,  juce::Slider::TextBoxBelow };
    juce::Slider mGainSSlider  { juce::Slider::LinearVertical,  juce::Slider::TextBoxBelow };
    juce::Slider mGainNSlider  { juce::Slider::LinearVertical,  juce::Slider::TextBoxBelow };
    juce::Slider mNfactorSlider  { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };
    juce::Slider mltSlider  { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };
    juce::Slider mlfSlider  { juce::Slider::RotaryHorizontalVerticalDrag,  juce::Slider::TextBoxBelow };


    juce::AudioProcessorValueTreeState::SliderAttachment mGainAttachment      { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramGain,     mGainSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mGainTAttachment      { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramGainT,     mGainTSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mGainSAttachment  { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramGainS, mGainSSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mGainNAttachment  { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramGainN, mGainNSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mNfactorAttachment  { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramNfactor, mNfactorSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mltAttachment  { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramlt, mltSlider };
    juce::AudioProcessorValueTreeState::SliderAttachment mlfAttachment  { audioProcessor.getValueTreeState(), Ttn_separatorAudioProcessor::paramlf, mlfSlider };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ttn_separatorAudioProcessorEditor)
};
