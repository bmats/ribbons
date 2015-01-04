//
//  AudioProcessor.h
//  Ribbons
//
//  Created by Bryce Matsumori on 10/4/14.
//
//

#ifndef __Ribbons__AudioProcessor__
#define __Ribbons__AudioProcessor__

#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/OutputNode.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/DataSource.h"
#include <vector>

/**
 * Singleton which processes audio from a data source.
 */
class AudioProcessor {
public:
    explicit AudioProcessor();
    ~AudioProcessor();
    
    bool init(const ci::DataSourceRef dataSource);
    void start();
    void update(float time);
    void setPaused(bool paused);

    size_t getNumBins() const;
    size_t getSampleRate() const;
    float getAmpForBin(size_t bin) const;
    float getAmpForBins(size_t start, size_t len) const;
    float getVolume() const;
    float getTotalDiff() const;
    float getDiffBetween(float minFreq, float maxFreq) const;

    bool isBeat();
    size_t getMaxFreq(float min, float max) const;

private:
    ci::audio::FilePlayerNodeRef      mFilePlayerNode;
    ci::audio::MonitorSpectralNodeRef mMonitorSpectralNode;

    const std::vector<float> *mMagSpectrum = 0, *mLastMagSpectrum = 0;
    float mVolume, mLastCaptureTime = 0;

    bool mLastBeatState = false;

    size_t mMinBin, mMaxBin;
};

#endif /* defined(__Ribbons__AudioProcessor__) */
