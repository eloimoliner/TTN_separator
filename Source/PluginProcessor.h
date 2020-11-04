/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class Ttn_separatorAudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener

{
public:
    //==============================================================================
    Ttn_separatorAudioProcessor();
    ~Ttn_separatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioSampleBuffer&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void readFromBuffer1(const int channel, const int readPos);
    void readFromBuffer3(juce::AudioSampleBuffer& buffer, const int channel, const int readPos);
    void writeToBuffer2(const int channel, const int readPos);
    void writeToBuffer3(const int channel, const int readPos);


    void readFromBuffer2(const int channel, const int readPos);
    void writeToBuffer1(juce::AudioSampleBuffer& buffer,
        const int channel,
        const int writePos);

    juce::AudioProcessorValueTreeState& getValueTreeState();

    void domymedianfilteringshit();


    void parameterChanged (const juce::String &parameterID, float newValue) override;

    //double filterLength_t;
    //double filterLength_f;
    /*
    double gain_t=1;
    double gain_s=1;
    double gain_n=1;
    double gain=1;*/

    static juce::String paramGain;
    static juce::String paramGainT;
    static juce::String paramGainS;
    static juce::String paramGainN;
    static juce::String paramNfactor;
    static juce::String paramlt;
    static juce::String paramlf;

private:
    //============================================================================== 
    juce::Atomic<float>   gain     {   0.0f };
    juce::Atomic<float>   gain_t     { 0.0f };
    juce::Atomic<float>   gain_s     { 0.0f };
    juce::Atomic<float>   gain_n     { 0.0f };
    juce::Atomic<float>   factor_n     { 2.0f };
    juce::Atomic<float>   filter_length_t     { 200.0f };
    juce::Atomic<float>   filter_length_f     { 180.0f };

    juce::UndoManager                  mUndoManager;


    int delay = 0;
    int    mWritePosB1 = 0;
    int    mReadPosB1 = -1;

    int    mWritePosB2 = 0;
    int    mReadPosB2 = -1;

    int    mWritePosB3 = 0;
    int    mReadPosB3 = -1;

    int newsamplesinB1 = 0;
    int newhopsinB2 = 0;

    int s_win = 2048;
    int nhop = 512;

    const static int nBins = 1025;

    double mSampleRate = 44100;

    const static int nMedianHmax = 17;
    int nMedianH = 17;
    int nMedianV = 9;
    const static int nMedianVmax = 13;

    std::array<float, 2048> mwindow;
    //std::array<kiss_fft_cpx,2048> grainL;
    //std::array<kiss_fft_cpx,2048> grainR;

    double mnormCoef = 1.5;


    const static int msizeB1 = 10000;
    const static int msizeB2 = 30;
    const static int msizeB3 = 10000;

    juce::AudioSampleBuffer            mBuffer1; //change to array
    //float mBuffer1[msizeB1][2];
    //float mBuffer3[msizeB3][2];

    juce::AudioSampleBuffer            mBuffer3; //change to array
    //std::complex<float> mBuffer2[msizeB2][nBins] = { 0 };
    std::complex<float> mBuffer2L[msizeB2][nBins] = { 0 };
    std::complex<float> mBuffer2R[msizeB2][nBins] = { 0 };
    //juce::dsp::Matrix<kiss_fft_cpx> mBuffer2L; //change to array
    //juce::dsp::Matrix<kiss_fft_cpx> mBuffer2R; //change to array

    float SSL[nMedianHmax][nBins] = { 0 };
    float SSR[nMedianHmax][nBins] = { 0 };
    //juce::dsp::Matrix<double> SSL;
    //juce::dtsp::Matrix<double> SSR;

    //kiss_fftr_cfg cfg = kiss_fftr_alloc(2048, false, 0, 0);
    //kiss_fftr_cfg icfg = kiss_fftr_alloc(2048, true, 0, 0);

    juce::dsp::FFT fftjuce;

    std::complex<float> grainL[2048]= { 0 };
    std::complex<float> grainR[2048]= { 0 };
    //std::complex<float> grain[2048] = { 0 };
    std::complex<float> fgrainL[2048] = { 0 };
    std::complex<float> fgrainR[2048] = { 0 };
    //std::complex<float> fgrainR[2048] = { 0 };
    //kiss_fft_scalar grainL[2048] = { 0 };
    //kiss_fft_scalar grainR[2048] = { 0 };
    //kiss_fft_cpx fgrainL[1025] = { 0 };
    //kiss_fft_cpx fgrainR[1025] = { 0 };

    //kiss_fft_cpx currentgrainL[1025] = { 0 };
    //kiss_fft_cpx currentgrainR[1025] = { 0 };
    //std::complex<float> currentgrain[1025][2] = { 0 };
   std::complex<float> currentgrainL[1025] = { 0 };
   std::complex<float> currentgrainR[1025] = { 0 };

    std::complex<float> xtL = { 0,0 };
    std::complex<float> xsL = { 0,0 };
    std::complex<float> xnL = { 0,0 };
    std::complex<float> xtR = { 0,0 };
    std::complex<float> xsR = { 0,0 };
    std::complex<float> xnR = { 0,0 };
    //kiss_fft_cpx xtL = { 0,0 };
    //kiss_fft_cpx xsL = { 0,0 };
    //kiss_fft_cpx xnL = { 0,0 };
    //kiss_fft_cpx xtR = { 0,0 };
    //kiss_fft_cpx xsR = { 0,0 };
    //kiss_fft_cpx xnR = { 0,0 };

    std::complex<float> totalL[2048] = { 0 };
    std::complex<float> totalR[2048] = { 0 };

    std::complex<float> cpx_timetotalL[2048] = { 0 };
    std::complex<float> cpx_timetotalR[2048] = { 0 };
    float timetotalL[2048] = { 0 };
    float timetotalR[2048] = { 0 };

    float ShL[1025] = { 0 };
    float ShR[1025] = { 0 };
    float SvL[1025] = { 0 };
    float SvR[1025] = { 0 };

    float RtL = 0;
    float RtR = 0;
    float RsL = 0;
    float RsR = 0;
    float RnL = 0;
    float RnR = 0;

    std::array<float, nMedianHmax> darrel = { 0};
    std::array<float, nMedianHmax> darrer = { 0};
    std::array<float, nMedianVmax> vdarrel = {0};
    std::array<float, nMedianVmax> vdarrer = {0};


    juce::AudioProcessorValueTreeState mState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ttn_separatorAudioProcessor)
};
