//
//  AudioProcessor.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 10/4/14.
//
//

#include "cinder/audio/Context.h"
#include "cinder/audio/Source.h"
#include <boost/filesystem/path.hpp>

#include "AudioProcessor.h"

#define MIN_FREQ 100
#define MAX_FREQ 4300

#define CAPTURE_INTERVAL 0.02f

#define BEAT_MAX_FREQ 250
#define BEAT_THRESHOLD 0.09f

#define FREQ_TO_BIN(freq) (size_t)((float)freq * mMonitorSpectralNode->getFftSize() / mMonitorSpectralNode->getSampleRate())
#define BIN_TO_FREQ(bin) bin * mMonitorSpectralNode->getSampleRate() / mMonitorSpectralNode->getFftSize()


AudioProcessor::AudioProcessor() {
}

AudioProcessor::~AudioProcessor() {
    if (mLastMagSpectrum) delete mLastMagSpectrum;
}

bool AudioProcessor::init(const ci::DataSourceRef dataSource) {
    auto ctx = ci::audio::Context::master();

    // Create the spectrum node
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives an increase in resolution of the resulting spectrum data
    auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
    mMonitorSpectralNode = ctx->makeNode( new ci::audio::MonitorSpectralNode(monitorFormat) );

    ctx->enable();

    try {
        ci::audio::SourceFileRef sourceFile(ci::audio::SourceFile::create(dataSource));

        mFilePlayerNode = ctx->makeNode( new ci::audio::FilePlayerNode );
        mFilePlayerNode->setSourceFile(sourceFile);

        mFilePlayerNode >> mMonitorSpectralNode;
        mFilePlayerNode >> ctx->getOutput();

        mFilePlayerNode->enable();

        return true;
    } catch (ci::audio::AudioFileExc e) {
        printf("Error loading audio: %s\n", e.what());
    }

    return false;
}

void AudioProcessor::start() {
    mFilePlayerNode->start();
}

void AudioProcessor::update(float time) {
//    if (time - mLastCaptureTime > CAPTURE_INTERVAL) {
        if (mLastMagSpectrum) delete mLastMagSpectrum;
        if (mMagSpectrum) mLastMagSpectrum = new std::vector<float>(*mMagSpectrum);

//        mLastCaptureTime = time;
//    }

    mMagSpectrum = &mMonitorSpectralNode->getMagSpectrum();
    mVolume      =  mMonitorSpectralNode->getVolume();

    mMinBin = FREQ_TO_BIN(MIN_FREQ);
    mMaxBin = FREQ_TO_BIN(MAX_FREQ);
}


size_t AudioProcessor::getNumBins() const {
    return mMaxBin - mMinBin + 1;
}

size_t AudioProcessor::getSampleRate() const {
    return mMonitorSpectralNode->getSampleRate();
}

float AudioProcessor::getAmpForBin(size_t bin) const {
    return mMagSpectrum->at(bin);
}

float AudioProcessor::getAmpForBins(size_t start, size_t len) const {
    size_t low = mMinBin + start, high = mMinBin + start + len;
    if (high > mMagSpectrum->size() - 1) high = mMagSpectrum->size() - 1;
    if (low > high) low = high;

    float total = 0;
    for (size_t i = low; i < high; ++i) {
        total += mMagSpectrum->at(i);
    }

    return total / len;
}

float AudioProcessor::getVolume() const {
    return mVolume;
}

float AudioProcessor::getTotalDiff() const {
    if (!mLastMagSpectrum) return 0.0f;
    
    float total = 0;
    for (size_t i = 0, len = mMagSpectrum->size(); i < len; ++i) {
        total += mMagSpectrum->at(i) - mLastMagSpectrum->at(i);
    }
    return total;
}

float AudioProcessor::getDiffBetween(float minFreq, float maxFreq) const {
    if (!mLastMagSpectrum) return 0.0f;

    float total = 0;
    size_t minBeatBin = FREQ_TO_BIN(minFreq); // can't be < 0
    size_t maxBeatBin = FREQ_TO_BIN(maxFreq);
    if (maxBeatBin > mMagSpectrum->size() - 1) maxBeatBin = mMagSpectrum->size() - 1;
    if (minBeatBin > maxBeatBin) minBeatBin = maxBeatBin;

    for (size_t i = minBeatBin; i <= maxBeatBin; ++i) {
        total += mMagSpectrum->at(i) - mLastMagSpectrum->at(i);
    }

    return total;
}

bool AudioProcessor::isBeat() {
    bool beat = getDiffBetween(0, BEAT_MAX_FREQ) > BEAT_THRESHOLD;

    bool lastState = mLastBeatState;
    mLastBeatState = beat;

    return lastState ? false : beat;
}

size_t AudioProcessor::getMaxFreq(float min, float max) const {
    size_t minBin = FREQ_TO_BIN(min); // can't be < 0
    size_t maxBin = FREQ_TO_BIN(max);
    if (maxBin > mMagSpectrum->size()) maxBin = mMagSpectrum->size();
    if (minBin > maxBin) minBin = maxBin;

    size_t highestBin;
    float highestAmp = 0;
    for (size_t i = minBin; i <= maxBin; ++i) {
        float amp = mMagSpectrum->at(i);
        if (amp > highestAmp) {
            highestBin = i;
            highestAmp = amp;
        }
        ++i;
    }

    return BIN_TO_FREQ(highestBin);
}
