/*
  ==============================================================================

    A simple delay example with time and feedback knobs

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class TapeDelayAudioProcessor  :  public juce::AudioProcessor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    TapeDelayAudioProcessor();
    ~TapeDelayAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void parameterChanged (const juce::String &parameterID, float newValue) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override;

    void writeToDelayBuffer (juce::AudioSampleBuffer& buffer,
                             const int channelIn, const int channelOut,
                             const int writePos,
                             float startGain, float endGain,
                             bool replacing);

    void readFromDelayBuffer (juce::AudioSampleBuffer& buffer,
                              const int channelIn, const int channelOut,
                              const int readPos,
                              float startGain, float endGain,
                              bool replacing);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState();

    static juce::String paramGain;
    static juce::String paramTime;
    static juce::String paramFeedback;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeDelayAudioProcessor)

    juce::Atomic<float>   mGain     {   0.0f };
    juce::Atomic<float>   mTime     { 200.0f };
    juce::Atomic<float>   mFeedback {  -6.0f };

    juce::UndoManager                  mUndoManager;
    juce::AudioProcessorValueTreeState mState;

    juce::AudioSampleBuffer            mDelayBuffer;

    float mLastInputGain    = 0.0f;
    float mLastFeedbackGain = 0.0f;

    int    mWritePos        = 0;
    int    mExpectedReadPos = -1;
    double mSampleRate      = 0;
};
