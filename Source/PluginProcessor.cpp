/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::String Ttn_separatorAudioProcessor::paramGain     ("gain");
juce::String Ttn_separatorAudioProcessor::paramGainT    ("gaint");
juce::String Ttn_separatorAudioProcessor::paramGainS    ("gains");
juce::String Ttn_separatorAudioProcessor::paramGainN    ("gainn");
juce::String Ttn_separatorAudioProcessor::paramNfactor    ("nfactor");
juce::String Ttn_separatorAudioProcessor::paramlt    ("timelength");
juce::String Ttn_separatorAudioProcessor::paramlf    ("freqlength");


//==============================================================================
Ttn_separatorAudioProcessor::Ttn_separatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),   mState (*this, &mUndoManager, "TTN_Separator",
          {
              std::make_unique<juce::AudioParameterFloat>(paramGain,
                                                    TRANS ("Gain"),
                                                    juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f,3.0f),
                                                    gain.get(), "dB",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramGainT,
                                                    TRANS ("GainT"),
                                                    juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f, 3.0f),
                                                    gain_t.get(), "dB",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramGainS,
                                                    TRANS ("GainS"),
                                                    juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f, 3.0f),
                                                    gain_s.get(), "dB",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramGainN,
                                                    TRANS ("GainN"),
                                                    juce::NormalisableRange<float>(-100.0f, 6.0f, 0.1f,3.0f),
                                                    gain_n.get(), "dB",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 1) + " dB"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }), 
              std::make_unique<juce::AudioParameterFloat>(paramNfactor,
                                                    TRANS ("Nfactor"),
                                                    juce::NormalisableRange<float>(1.01f, 300.0f, 0.01f, 0.22f, false),
                                                    factor_n.get(), " ",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 2)+"   "; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramlt,
                                                    TRANS ("TFLength"),
                                                    juce::NormalisableRange<float>(24.0f, 200.0f, 0.1f, 1.0f, false),
                                                    filter_length_t.get(), " ms",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 2)+" ms"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); }),
              std::make_unique<juce::AudioParameterFloat>(paramlf,
                                                    TRANS ("FFLength"),
                                                    juce::NormalisableRange<float>(22.0f, 280.0f, 0.1f, 1.0f, false),
                                                    filter_length_f.get(), " Hz",
                                                    juce::AudioProcessorParameter::genericParameter,
                                                    [](float v, int) { return juce::String (v, 2)+" Hz"; },
                                                    [](const juce::String& t) { return t.dropLastCharacters (3).getFloatValue(); })
          }), fftjuce(11), mBuffer1(2,msizeB1), mBuffer3(2,msizeB3)


#endif
{ 
    mState.addParameterListener (paramGain, this);
    mState.addParameterListener (paramGainT, this);
    mState.addParameterListener (paramGainS, this);
    mState.addParameterListener (paramGainN, this);
    mState.addParameterListener (paramNfactor, this);
    mState.addParameterListener (paramlt, this);
    mState.addParameterListener (paramlf, this);


    //window initialized by definition (consider using a table)
    for (int i = 0; i < s_win ; ++i) {
        double value = 0.5 * (1 - juce::dsp::FastMathApproximations::cos(juce::MathConstants<double>::twoPi * i /((double)s_win-1)));
        mwindow[i]=value;
    }

    //rounded value (consider calculate it with the given window)
    mnormCoef = 1.5;

    //filterLength_t = 0.2; // in s
    //filterLength_f = 180; // in Hz

    //nMedianH = (int)2*floor(filterLength_t * mSampleRate /(2* (double)nhop))+1;
     //nMedianV = (int)2*floor(filterLength_f* s_win /(2*mSampleRate))+1;

     delay =nhop*ceil(nMedianHmax/2);
     //delay =10000;
    //juce::dsp::Matrix<double> mBuffer2L(msizeB2, nBins);
    //juce::dsp::Matrix<double> mBuffer2R(msizeB2, nBins);


}

Ttn_separatorAudioProcessor::~Ttn_separatorAudioProcessor()
{
}

//juce::Array<double> Ttn_separatorAudioProcessor::hanning(juce::Array<double> windows)
//{
//    juce::Array<double,s_win> retornar
//    for (int i = 0; i <= s_win - 1; ++i) {
//        double valuecos=juce::dsp::FastMathApproximations::cos(juce::MathConstants<double>::twoPi * i /(s_win-1));
//        double value=0.5 *(1- valuecos)
//        windows.setUnchecked(i,value)
//    }
////w = .5*(1 - cos(2*pi*(0:M-1)'/(M-1)));
//}
//==============================================================================
const juce::String Ttn_separatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Ttn_separatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ttn_separatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ttn_separatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Ttn_separatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ttn_separatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Ttn_separatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ttn_separatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Ttn_separatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void Ttn_separatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Ttn_separatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampleRate = sampleRate;

    // sample buffer for 2 seconds + 2 buffers safety
    //mBuffer1.setSize (getTotalNumOutputChannels(), msizeB1, true, true);
    mBuffer1.clear();

    //mBuffer2L.clear();
    //mBuffer2R.clear();
    //SSL.clear();
    //SSR.clear();

    //mBuffer3.setSize (getTotalNumOutputChannels(), msizeB3, true, true);
    mBuffer3.clear();

    mWritePosB1 = 0;
    mReadPosB1 = 0;
    mWritePosB2 = 0;
    mReadPosB2 = 0;
    mWritePosB3 = 0;
    mReadPosB3 = -delay +msizeB3;

    mReadPosB3 %= msizeB3;

    newsamplesinB1 = 0;
    newhopsinB2 = 0;
    //kiss_fft_cpx *fgrainL = new kiss_fft_cpx[s_win/2+1];
    //kiss_fft_scalar *grainL = new kiss_fft_scalar[s_win];
    //kiss_fft_cpx *fgrainR = new kiss_fft_cpx[s_win/2+1];
    //kiss_fft_scalar *grainR = new kiss_fft_scalar[s_win];

    //grainL.fill(0);
    //grainR.fill(0);
}

void Ttn_separatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Ttn_separatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Ttn_separatorAudioProcessor::processBlock (juce::AudioSampleBuffer& buffer, juce::MidiBuffer& midiMessages)
{
   const float local_gain = juce::Decibels::decibelsToGain (gain.get());
   const float local_gain_t = juce::Decibels::decibelsToGain (gain_t.get());
   const float local_gain_s = juce::Decibels::decibelsToGain (gain_s.get());
   const float local_gain_n = juce::Decibels::decibelsToGain (gain_n.get());
   const float local_factor_n = factor_n.get();
    nMedianH = (int)2*floor(filter_length_t.get()/1000 * mSampleRate /(2* (double)nhop))+1; //only impair, correct for pair
   nMedianV = (int)2*floor(filter_length_f.get()* s_win /(2*mSampleRate))+1;

        // write original to buffer1
        writeToBuffer1 (buffer, 0, mWritePosB1);
        writeToBuffer1 (buffer, 1, mWritePosB1);
        mWritePosB1 += buffer.getNumSamples();
        mWritePosB1 %= msizeB1;

        newsamplesinB1 += buffer.getNumSamples();


        while (newsamplesinB1 >= nhop)
        {
            readFromBuffer1( 0, mReadPosB1);
            readFromBuffer1( 1, mReadPosB1);

            //if (abs(grainL[0]) > 0.1) {
            //    DBG("hello");
            //}
            //windowing
            for (int i = 0; i < s_win; ++i) {
                grainL[i] *= mwindow[i];
                grainR[i] *= mwindow[i];
            }
            //fftshift
            for (int i = 0; i < s_win / 2; ++i) {
                std::complex<float> tmpl = grainL[i];
                std::complex<float> tmpr = grainR[i];

                int k = (int)s_win / 2 + i;
                grainL[i] = grainL[k];
                grainR[i] = grainR[k];
                grainL[k] = tmpl;
                grainR[k] = tmpr;
            }
            //fft
            //kiss_fftr(cfg, grainL, fgrainL); //increase speed
            fftjuce.perform(&grainL[0], &fgrainL[0], false);
            fftjuce.perform(&grainR[0], &fgrainR[0], false);

            //free(cfg);
            //kiss_fftr(cfg, grainR, fgrainR);
            //free(cfg);

            writeToBuffer2(0, mWritePosB2);
            writeToBuffer2 (1, mWritePosB2);

            newhopsinB2++;
            mWritePosB2++;
            mWritePosB2 %= msizeB2;

            mReadPosB1 += nhop;
            mReadPosB1 %= msizeB1;
            newsamplesinB1 -= nhop;
        }

        while(newhopsinB2 >= ceil(nMedianHmax / 2)) {

            readFromBuffer2(0, mReadPosB2);
            readFromBuffer2(1, mReadPosB2);
            mReadPosB2++;
            mReadPosB2 %= msizeB2;
            domymedianfilteringshit();
            //fussy membership functions
            for (int l = nBins; l--;) {
                //RtL = pow(SvL[l], 2) / (pow(SvL[l], 2) + pow(ShL[l], 2));
                //RsL = 1 - RtL;
                //RtR = pow(SvR[l], 2) / (pow(SvR[l], 2) + pow(ShR[l], 2));
                //RsR = 1 - RtR; 
                RsL = pow(ShL[l], 2) / (pow(SvL[l], 2) + pow(ShL[l], 2));
                if (isnan(RsL)) RsL = 0.5;
                RtL = 1 - RsL;
                RsR = pow(ShR[l], 2) / (pow(SvR[l], 2) + pow(ShR[l], 2));
                if (isnan(RsR)) RsR = 0.5;
                RtR = 1 - RsR;

                RnL = 1 - pow(abs(RtL - RsL), 1/local_factor_n);
                RnR = 1 - pow(abs(RtR - RsR), 1/local_factor_n);
                //RnL = 1 - sqrt(abs(RtL - RsL));
                //RnR = 1 - sqrt(abs(RtR - RsR));
                //apply them, considering removing the Rt, rs and rn arrays
                xtL = currentgrainL[l] * (RtL - RnL / 2);
                xtR = currentgrainR[l] * (RtR - RnR / 2);

                xsL = currentgrainL[l] * (RsL - RnL / 2);
                xsR = currentgrainR[l] * (RsR - RnR / 2);


                xnL = currentgrainL[l] * RnL;
                xnR = currentgrainR[l] * RnR;

                totalL[l] = local_gain * (local_gain_t * xtL + local_gain_s * xsL + local_gain_n * xnL);
                totalR[l] = local_gain * (local_gain_t * xtR + local_gain_s * xsR + local_gain_n * xnR);

                //totalL[l] = currentgrainL[l];
                //totalR[l] = currentgrainR[l];
                if (l > 0 && l < nBins-1) {
                    totalL[s_win - l] = conj(totalL[l]);
                    totalR[s_win - l] = conj(totalR[l]);

                }
            }
            //ffft
            //kiss_fftri(icfg, totalL, timetotalL);
            fftjuce.perform(&totalL[0], &cpx_timetotalL[0], true);
            fftjuce.perform(&totalR[0], &cpx_timetotalR[0], true);
            //free(icfg);
            //kiss_fftri(icfg, totalR, timetotalR);
            //free(icfg);
            //fftshift
            for (int i = 0; i < s_win / 2; i++ ) {

                int k = (int)s_win / 2 + i;
                timetotalL[i] = cpx_timetotalL[k].real();
                timetotalR[i] = cpx_timetotalR[k].real();
                timetotalL[k] = cpx_timetotalL[i].real();
                timetotalR[k] = cpx_timetotalR[i].real();
            }
            //windowing
            for (int i = 0; i< s_win; i++ ) {
                //timetotalL[i] *= mwindow[i]/(mnormCoef*sqrt(s_win));
                timetotalL[i] *= mwindow[i]/(mnormCoef);
                //timetotalR[i] *= mwindow[i]/(mnormCoef*sqrt(s_win));
                timetotalR[i] *= mwindow[i]/(mnormCoef);
            }
            //write in buffer 3

            writeToBuffer3(0,mWritePosB3);
            writeToBuffer3(1,mWritePosB3);
            newhopsinB2--;
            mWritePosB3 += nhop;
            mWritePosB3 %= msizeB3;

    }

        //read from buffer 3 (and erase)
        readFromBuffer3(buffer, 0, mReadPosB3);
        readFromBuffer3(buffer, 1, mReadPosB3);
        mReadPosB3 += buffer.getNumSamples();
        mReadPosB3 %= msizeB3;

    
}
void Ttn_separatorAudioProcessor::readFromBuffer3(juce::AudioSampleBuffer& buffer, const int channel, const int readPos) {
    //boutreadend=mymod(boutread+bufsize-1,sizebufout); 
    int b3readend = readPos + buffer.getNumSamples() - 1;
    b3readend %= msizeB3;

        if (b3readend > readPos)
        {
            buffer.copyFrom(channel, 0, mBuffer3.getReadPointer (channel, readPos), buffer.getNumSamples());
            mBuffer3.clear(channel, readPos,  buffer.getNumSamples());
        }
        else
        {
            const auto midPos = msizeB3 - readPos;

            buffer.copyFrom(channel, 0, mBuffer3.getReadPointer (channel, readPos), midPos);
            buffer.copyFrom(channel, midPos, mBuffer3.getReadPointer (channel, 0), buffer.getNumSamples()-midPos);
            mBuffer3.clear(channel, readPos,  midPos);
            mBuffer3.clear(channel, 0,  buffer.getNumSamples()-midPos);

        }
}

void Ttn_separatorAudioProcessor::writeToBuffer3(
    const int channel,
    const int writePos) {

    int writeend = writePos + s_win - 1;
    writeend %= msizeB3;

    if (channel == 0) {
        if (writeend > writePos)
        {
            mBuffer3.addFrom(0, writePos, timetotalL, s_win);
        }
        else
        {
            const auto midPos = msizeB3 - writePos;
            mBuffer3.addFrom(0, writePos, timetotalL, midPos);
            mBuffer3.addFrom(0, 0, timetotalL + midPos, s_win - midPos);
        }
    }
    else
    {
        if (writeend > writePos)
        {
            mBuffer3.addFrom(1, writePos, timetotalR, s_win);
        }
        else
        {
            const auto midPos = msizeB3 - writePos;
            mBuffer3.addFrom(1, writePos, timetotalR, midPos);
            mBuffer3.addFrom(1, 0, timetotalR + midPos, s_win - midPos);
        }

    }
}
void Ttn_separatorAudioProcessor::writeToBuffer2(
    const int channel,
    const int writePos) {

    if (channel == 0) {
        for (int i = nBins; i--;) {
            mBuffer2L[writePos][i] = fgrainL[i] ;
            //mBuffer2L[writePos][i] = fgrainL[i] / (float)sqrt(s_win);
        }
    }
    else
    {
        for (int i = nBins; i--;) {
            mBuffer2R[writePos][i] = fgrainR[i] ;
            mBuffer2R[writePos][i] = fgrainR[i] ;
            //mBuffer2R[writePos][i] = fgrainR[i] /(float)sqrt(s_win);
            //mBuffer2R[writePos][i] = fgrainR[i] /(float)sqrt(s_win);
        }

    }


}
void Ttn_separatorAudioProcessor::writeToBuffer1 (juce::AudioSampleBuffer& buffer,
                                                  const int channel, const int writePos)
{
    if (writePos + buffer.getNumSamples() <= msizeB1)
    {
        //for (int i = 0; i < buffer.getNumSamples(); i++)
         //   mBuffer1[i][channel] =buffer.getSample(channel, i);
          mBuffer1.copyFrom(channel, writePos, buffer.getReadPointer (channel), buffer.getNumSamples());
    }
    else
    {
        const auto midPos = msizeB1 - writePos;
        //int j = 0;
        //for (int i = writePos; i < midPos; i++){
         //   mBuffer1[i][channel] = buffer.getSample(channel, j);
        //j++;
    //}        for (int i = 0; i < (buffer.getNumSamples() - midPos); i++){
    //        mBuffer1[i][channel] = buffer.getSample(channel, j);
    //    j++;
    //}

            mBuffer1.copyFrom(channel, writePos, buffer.getReadPointer (channel),         midPos);
            mBuffer1.copyFrom(channel,0,        buffer.getReadPointer (channel, midPos), buffer.getNumSamples() - midPos);
    }
}

void Ttn_separatorAudioProcessor::readFromBuffer2 ( const int channel,const int readPos)
{
            int grain_1=readPos-floor(nMedianH/2); //-1 removed in C
            if (grain_1 < 0)
                grain_1 += msizeB2;

            grain_1%=msizeB2;

            int grain_end = readPos + floor(nMedianH/2) ;
            grain_end %= msizeB2;

        if (grain_end > grain_1) {
            if (channel == 0) {
                int j = 0;
                for (int i = grain_1; i <= grain_end; ++i) {
                    for (int k=nBins;k--;){
                        SSL[j][k] = std::abs(mBuffer2L[i][k]);
                        //sqrt(pow(mBuffer2L[i][k].r, 2) + pow(mBuffer2L[i][k].i, 2));
                        if (i==readPos)
                            currentgrainL[k] = mBuffer2L[i][k];
                    }
                    j++;
                }
            }
            else {
                int j = 0;
                for (int i = grain_1; i <= grain_end; ++i) {
                    for (int k=nBins;k--;){
                        //SSR[j][k] = sqrt( pow(mBuffer2R[i][k].r,2)+pow(mBuffer2R[i][k].i,2));
                        SSR[j][k] = std::abs(mBuffer2R[i][k]);
                        if (i==readPos)
                            currentgrainR[k] = mBuffer2R[i][k];
                    }
                    j++;
                }
            }
        }
        else {
            if (channel == 0) {

            int j = 0;
            for (int i = grain_1; i < msizeB2; ++i) {
                    for (int k=nBins;k--;){
                        //SSL[j][k] = sqrt( pow(mBuffer2L[i][k].r,2)+pow(mBuffer2L[i][k].i,2));
                        SSL[j][k] = std::abs(mBuffer2L[i][k]);
                        if (i==readPos)
                            currentgrainL[k] = mBuffer2L[i][k];
                    }
                j++;
            }
            for (int i = 0; i <= grain_end && j<s_win; ++i) {
                    for (int k=nBins;k--;){
                        //SSL[j][k] = sqrt( pow(mBuffer2L[i][k].r,2)+pow(mBuffer2L[i][k].i,2));
                        SSL[j][k] = std::abs(mBuffer2L[i][k]);
                        if (i==readPos)
                            currentgrainL[k] = mBuffer2L[i][k];
                    }
                j++;
            }
            }
            else
            {
            int j = 0;
            for (int i = grain_1; i < msizeB2; ++i) {
                    for (int k=nBins;k--;){
                        //SSR[j][k] = sqrt( pow(mBuffer2R[i][k].r,2)+pow(mBuffer2R[i][k].i,2));
                        SSR[j][k] = std::abs(mBuffer2R[i][k]);
                        if (i==readPos)
                            currentgrainR[k] = mBuffer2R[i][k];
                    }
                j++;
            }
            for (int i = 0; i <= grain_end && j<nMedianHmax; ++i) {
                    for (int k=nBins;k--;){
                        //SSR[j][k] = sqrt( pow(mBuffer2R[i][k].r,2)+pow(mBuffer2R[i][k].i,2));
                        SSR[j][k] = std::abs(mBuffer2R[i][k]);
                        if (i==readPos)
                            currentgrainR[k] = mBuffer2R[i][k];
                    }
                j++;
            }

            }

        }
}


void Ttn_separatorAudioProcessor::domymedianfilteringshit() {
    //OJO!!!
//[timeframes, nbins]= size(SS);
//midframe=ceil(timeframes/2);
//

    for (int l = 0; l<nBins; l++ ) {

        darrel = {0};
        darrer = {0};
        vdarrel = {0};
        vdarrer = {0 };
        for (int j = 0; j < nMedianH; ++j) {
        darrel[j]=SSL[j][l];
        darrer[j]=SSR[j][l];
        }
        std::sort(darrel.begin(), darrel.end()); //increase speed
        std::sort(darrer.begin(), darrer.end());
        int w = nMedianHmax - nMedianH;
        ShL[l] = darrel[w + ceil(nMedianH / 2)];
        ShR[l] = darrer[w + ceil(nMedianH / 2)];

        for (int m = 0; m < nMedianV ; ++m) {
            int value = l + m -floor(nMedianV/2);
            if (value < 0 || value>= nBins ) {
                vdarrel[m] = 0;
                vdarrer[m] = 0;
            }
            else {
                vdarrel[m] = SSL[(int)ceil(nMedianH / 2)] [value];
                vdarrer[m] = SSR[(int)ceil(nMedianH / 2)] [value];
            }

        }
        int w2 = nMedianVmax - nMedianV;
        std::sort(vdarrel.begin(), vdarrel.end()); //increase speed
        std::sort(vdarrer.begin(), vdarrer.end()); //increase speed
        SvL[l] = vdarrel[w2 + ceil(nMedianV / 2)];
        SvR[l] = vdarrer[w2 + ceil(nMedianV / 2)];


    }
//Sh=median(SS); %calculation in matrix (use a for loop in C)
//
//midS=SS(midframe,:);
//Sv=medfilt1(midS,nMedianV);%implement manually

}
void Ttn_separatorAudioProcessor::readFromBuffer1 ( const int channel,const int readPos)
{
            int grain_1=readPos+nhop-s_win +msizeB1; //-1 removed in C
            grain_1%=msizeB1;

            int grain_end = readPos + nhop-1 ;
            grain_end %= msizeB1;


            if (channel == 0) {
                if (grain_end > grain_1) {
                    int i = 0;
                    //grainL = mBuffer1.getReadPointer(channel, grain_1);
                    for (int j = grain_1; j < grain_end; ++j) {
                        grainL[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }
                }
                else {
                    int i = 0;
                    for (int j = grain_1; j < msizeB1; ++j) {
                        grainL[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }
                    for (int j = 0; j <= grain_end && i < s_win; ++j) {
                        grainL[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }

                }
            }
            else {

                if (grain_end > grain_1) {
                    int i = 0;
                    for (int j = grain_1; j < grain_end; ++j) {
                        grainR[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }
                }
                else {
                    int i = 0;
                    for (int j = grain_1; j < msizeB1; ++j) {
                        grainR[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }
                    for (int j = 0; j <= grain_end && i < s_win; ++j) {
                        grainR[i] = mBuffer1.getSample(channel, j);
                        i++;
                    }

                }
            }
}
//==============================================================================
bool Ttn_separatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Ttn_separatorAudioProcessor::createEditor()
{
    return new Ttn_separatorAudioProcessorEditor (*this);
}

//==============================================================================
void Ttn_separatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}
void  Ttn_separatorAudioProcessor::parameterChanged (const juce::String &parameterID, float newValue)
{
    if (parameterID == paramGain) {
        gain = newValue;
    }
    else if (parameterID == paramGainT) {
        gain_t = newValue;
    }
    else if (parameterID == paramGainS) {
        gain_s = newValue;
    }
    else if (parameterID == paramGainN) {
        gain_n = newValue;
    }
    else if (parameterID == paramNfactor) {
        factor_n = newValue;
    } 
    else if (parameterID == paramlt) {
        filter_length_t = newValue;
    }
    else if (parameterID == paramlf) {
        filter_length_f = newValue;
    }

}

juce::AudioProcessorValueTreeState& Ttn_separatorAudioProcessor::getValueTreeState()
{
    return mState;
}



void Ttn_separatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ttn_separatorAudioProcessor();
}

