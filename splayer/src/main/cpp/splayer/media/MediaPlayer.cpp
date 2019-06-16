
#include "MediaPlayer.h"


MediaPlayer::MediaPlayer() {
    ALOGD(__func__);
    pMutex = new Mutex();
    pPlay = new FFPlay();
}

MediaPlayer::~MediaPlayer() {
    ALOGD(__func__);
    delete pPlay;
    delete pMutex;
}

int MediaPlayer::create() {
    ALOGD(__func__);
    if (pPlay) {
        // 设置图像输出表面
        VOut *vOut = createSurface();
        pPlay->setVOut(vOut);
        if (!vOut) {
            return EXIT_FAILURE;
        }

        // 设置数据输入管道
        Pipeline *pipeline = createPipeline();
        if (pipeline) {
            pipeline->setOpaque(pipeline->createOpaque());
            pPlay->setPipeline(pipeline);
        } else {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int MediaPlayer::start() {
    ALOGD(__func__);
    return EXIT_FAILURE;
}

int MediaPlayer::stop() {
    ALOGD(__func__);
    return EXIT_FAILURE;
}

int MediaPlayer::pause() {
    ALOGD(__func__);
    return EXIT_FAILURE;
}

int MediaPlayer::reset() {
    ALOGD(__func__);
    return EXIT_FAILURE;
}

int MediaPlayer::destroy() {
    ALOGD(__func__);
    return EXIT_FAILURE;
}


