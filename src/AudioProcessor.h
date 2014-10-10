//
//  FrequencyProcessor.h
//  Ribbons
//
//  Created by Bryce Matsumori on 10/4/14.
//
//

#ifndef __Ribbons__FrequencyProcessor__
#define __Ribbons__FrequencyProcessor__

#include "cinder/audio/SamplePlayerNode.h"
#include "cinder/audio/OutputNode.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/DataSource.h"
#include <vector>

class FrequencyProcessor {
public:
    explicit FrequencyProcessor();

    void setSourceFile(ci::DataSourceRef dataSource);

    void start();
    void update();

private:
    ci::audio::FilePlayerNodeRef mFilePlayerNode;
    ci::audio::OutputDeviceNodeRef mOutputDeviceNode;
    ci::audio::MonitorSpectralNodeRef mMonitorSpectralNode;

    std::vector<float> mMagSpectrum;
    float mVolume;
};

#endif /* defined(__Ribbons__FrequencyProcessor__) */
