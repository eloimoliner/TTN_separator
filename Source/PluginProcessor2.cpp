/*
  ==============================================================================

    A simple delay example with time and feedback knobs

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


juce::String TapeDelayAudioProcessor::paramGain     ("gain");
juce::String TapeDelayAudioProcessor::paramTime     ("time");
juce::String TapeDelayAudioProcessor::paramFeedback ("feedback");


//==============================================================================
TapeDelayAudioProcessor::TapeDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
   mState (*this, &mUndoManager, "FFTapeDelay",
          {
              std::make_unique<juce::AudioParameterFloat>(paramGain,
                                                    TRANS ("Input Gain"),
                                                    juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f, std::log (0.5f) / std::log (100.0f / 106.0f)),
                                                    mGain.get(), "dB",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramTime,
                                                    TRANS ("Delay TIme"),    juce::NormalisableRange<float>(0.0, 2000.0, 1.0),
                                                    mTime.get(), "ms",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (juce::roundToInt (v)) + " ms"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramFeedback,
                                                    TRANS ("Feedback Gain"), juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f, std::log (0.5f) / std::log (100.0f / 106.0f)),
                                                    mFeedback.get(), "dB", juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); })
          })

#endif

{
    mState.addParameterListener (paramGain, this);
    mState.addParameterListener (paramTime, this);
    mState.addParameterListener (paramFeedback, this);
}

TapeDelayAudioProcessor::~TapeDelayAudioProcessor()
{
}

//==============================================================================
void TapeDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    mSampleRate = sampleRate;

    // sample buffer for 2 seconds + 2 buffers safety
    mDelayBuffer.setSize (getTotalNumOutputChannels(), 2.0 * (samplesPerBlock + sampleRate), false, false);
    mDelayBuffer.clear();

    mExpectedReadPos = -1;
}

void TapeDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void TapeDelayAudioProcessor::parameterChanged (const juce::String &parameterID, float newValue)
{
    if (parameterID == paramGain) {
        mGain = newValue;
    }
    else if (parameterID == paramTime) {
        mTime = newValue;
    }
    else if (parameterID == paramFeedback) {
        mFeedback = newValue;
    }
}

bool TapeDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // we only support stereo and mono
    if (layouts.getMainInputChannels() == 0 || layouts.getMainInputChannels() > 2)
        return false;

    if (layouts.getMainOutputChannels() == 0 || layouts.getMainOutputChannels() > 2)
        return false;

    // we don't allow the narrowing the number of channels
    if (layouts.getMainInputChannels() > layouts.getMainOutputChannels())
        return false;

    return true;
}

void TapeDelayAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages)
{
    if (Bus* inputBus = getBus (true, 0))
    {
        const float gain = juce::Decibels::decibelsToGain (mGain.get());
        const float time = mTime.get();
        const float feedback = juce::Decibels::decibelsToGain (mFeedback.get());

        // write original to delay
        for (int i=0; i < mDelayBuffer.getNumChannels(); ++i)
        {
            const int inputChannelNum = inputBus->getChannelIndexInProcessBlockBuffer (std::min (i, inputBus->getNumberOfChannels()));
            writeToDelayBuffer (buffer, inputChannelNum, i, mWritePos, 1.0f, 1.0f, true);
        }

        // adapt dry gain
        buffer.applyGainRamp (0, buffer.getNumSamples(), mLastInputGain, gain);
        mLastInputGain = gain;

        // read delayed signal
        auto readPos = juce::roundToInt (mWritePos - (mSampleRate * time / 1000.0));
        if (readPos < 0)
            readPos += mDelayBuffer.getNumSamples();

        if (Bus* outputBus = getBus (false, 0))
        {
            // if has run before
            if (mExpectedReadPos >= 0)
            {
                // fade out if readPos is off
                auto endGain = (readPos == mExpectedReadPos) ? 1.0f : 0.0f;
                for (int i=0; i<outputBus->getNumberOfChannels(); ++i)
                {
                    const int outputChannelNum = outputBus->getChannelIndexInProcessBlockBuffer (i);
                    readFromDelayBuffer (buffer, i, outputChannelNum, mExpectedReadPos, 1.0, endGain, false);
                }
            }

            // fade in at new position
            if (readPos != mExpectedReadPos)
            {
                for (int i=0; i<outputBus->getNumberOfChannels(); ++i)
                {
                    const int outputChannelNum = outputBus->getChannelIndexInProcessBlockBuffer (i);
                    readFromDelayBuffer (buffer, i, outputChannelNum, readPos, 0.0, 1.0, false);
                }
            }
        }

        // add feedback to delay
        for (int i=0; i<inputBus->getNumberOfChannels(); ++i)
        {
            const int outputChannelNum = inputBus->getChannelIndexInProcessBlockBuffer (i);
            writeToDelayBuffer (buffer, outputChannelNum, i, mWritePos, mLastFeedbackGain, feedback, false);
        }
        mLastFeedbackGain = feedback;

        // advance positions
        mWritePos += buffer.getNumSamples();
        if (mWritePos >= mDelayBuffer.getNumSamples())
            mWritePos -= mDelayBuffer.getNumSamples();

        mExpectedReadPos = readPos + buffer.getNumSamples();
        if (mExpectedReadPos >= mDelayBuffer.getNumSamples())
            mExpectedReadPos -= mDelayBuffer.getNumSamples();
    }
}

void TapeDelayAudioProcessor::writeToDelayBuffer (juce::AudioSampleBuffer& buffer,
                                                  const int channelIn, const int channelOut,
                                                  const int writePos, float startGain, float endGain, bool replacing)
{
    if (writePos + buffer.getNumSamples() <= mDelayBuffer.getNumSamples())
    {
        if (replacing)
            mDelayBuffer.copyFromWithRamp (channelOut, writePos, buffer.getReadPointer (channelIn), buffer.getNumSamples(), startGain, endGain);
        else
            mDelayBuffer.addFromWithRamp (channelOut, writePos, buffer.getReadPointer (channelIn), buffer.getNumSamples(), startGain, endGain);
    }
    else
    {
        const auto midPos  = mDelayBuffer.getNumSamples() - writePos;
        const auto midGain = juce::jmap (float (midPos) / buffer.getNumSamples(), startGain, endGain);
        if (replacing)
        {
            mDelayBuffer.copyFromWithRamp (channelOut, writePos, buffer.getReadPointer (channelIn),         midPos, startGain, midGain);
            mDelayBuffer.copyFromWithRamp (channelOut, 0,        buffer.getReadPointer (channelIn, midPos), buffer.getNumSamples() - midPos, midGain, endGain);
        }
        else
        {
            mDelayBuffer.addFromWithRamp (channelOut, writePos, buffer.getReadPointer (channelIn),         midPos, mLastInputGain, midGain);
            mDelayBuffer.addFromWithRamp (channelOut, 0,        buffer.getReadPointer (channelIn, midPos), buffer.getNumSamples() - midPos, midGain, endGain);
        }
    }
}

void TapeDelayAudioProcessor::readFromDelayBuffer (juce::AudioSampleBuffer& buffer,
                                                   const int channelIn, const int channelOut,
                                                   const int readPos,
                                                   float startGain, float endGain,
                                                   bool replacing)
{
    if (readPos + buffer.getNumSamples() <= mDelayBuffer.getNumSamples())
    {
        if (replacing)
            buffer.copyFromWithRamp (channelOut, 0, mDelayBuffer.getReadPointer (channelIn, readPos), buffer.getNumSamples(), startGain, endGain);
        else
            buffer.addFromWithRamp (channelOut, 0, mDelayBuffer.getReadPointer (channelIn, readPos), buffer.getNumSamples(), startGain, endGain);
    }
    else
    {
        const auto midPos  = mDelayBuffer.getNumSamples() - readPos;
        const auto midGain = juce::jmap (float (midPos) / buffer.getNumSamples(), startGain, endGain);
        if (replacing)
        {
            buffer.copyFromWithRamp (channelOut, 0,      mDelayBuffer.getReadPointer (channelIn, readPos), midPos, startGain, midGain);
            buffer.copyFromWithRamp (channelOut, midPos, mDelayBuffer.getReadPointer (channelIn), buffer.getNumSamples() - midPos, midGain, endGain);
        }
        else
        {
            buffer.addFromWithRamp (channelOut, 0,      mDelayBuffer.getReadPointer (channelIn, readPos), midPos, startGain, midGain);
            buffer.addFromWithRamp (channelOut, midPos, mDelayBuffer.getReadPointer (channelIn), buffer.getNumSamples() - midPos, midGain, endGain);
        }
    }
}

juce::AudioProcessorValueTreeState& TapeDelayAudioProcessor::getValueTreeState()
{
    return mState;
}

//==============================================================================
bool TapeDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TapeDelayAudioProcessor::createEditor()
{
    return new TapeDelayAudioProcessorEditor (*this);
}

//==============================================================================
void TapeDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    juce::MemoryOutputStream stream(destData, false);
    mState.state.writeToStream (stream);
}

void TapeDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ValueTree tree = juce::ValueTree::readFromData (data, sizeInBytes);
    if (tree.isValid()) {
        mState.state = tree;
    }
}

//==============================================================================
const juce::String TapeDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapeDelayAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TapeDelayAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

double TapeDelayAudioProcessor::getTailLengthSeconds() const
{
    return 2.0;
}

int TapeDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int TapeDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapeDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapeDelayAudioProcessor::getProgramName (int index)
{
    return juce::String();
}

void TapeDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeDelayAudioProcessor();
}
