//
//  FrequencyProcessor.cpp
//  Ribbons
//
//  Created by Bryce Matsumori on 10/4/14.
//
//

#include "cinder/audio/Context.h"
#include "cinder/audio/Source.h"
#include <boost/filesystem/path.hpp>

#include "FrequencyProcessor.h"

FrequencyProcessor::FrequencyProcessor() {
    auto ctx = ci::audio::Context::master();

    mOutputDeviceNode = ctx->createOutputDeviceNode();

    // Create the spectrum node
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
    mMonitorSpectralNode = ctx->makeNode( new ci::audio::MonitorSpectralNode(monitorFormat) );

    ctx->enable();
        
//    mTrack = ci::audio::Output::addTrack( mAudioSource, false );
//    mTrack->enablePcmBuffering( true );
//    mTrack->play();
}

void FrequencyProcessor::setSourceFile(ci::DataSourceRef dataSource) {
//    mAudioSource = ci::audio::load( ci::app::loadResource( RES_AUDIO ) );
    ci::audio::SourceFileRef sourceFile(ci::audio::SourceFile::create(dataSource));

    mFilePlayerNode->setSourceFile(sourceFile);

    mFilePlayerNode >> mMonitorSpectralNode;
    mFilePlayerNode >> mOutputDeviceNode;

    mFilePlayerNode->enable();
}

void FrequencyProcessor::start() {
    mFilePlayerNode->start();
}

void FrequencyProcessor::update() {
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    mVolume      = mMonitorSpectralNode->getVolume();

    size_t nBins = mMagSpectrum.size();
    // TODO: limit frequencies processed (ex. high not heard)
}