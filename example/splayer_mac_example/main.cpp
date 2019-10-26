#include <SDLMediaPlayer.h>
#include <SDLAudioDevice.h>
#include <SDLVideoDevice.h>
#include <message/MessageCenter.h>

SDLMediaPlayer *mediaPlayer = nullptr;

class MessageListener : public IMessageListener {
    const char *const TAG = "MessageListener";
public:
    void onMessage(Msg *msg) override {
        if (DEBUG) {
            ALOGD(TAG, "%s what = %s arg1I = %d arg2I = %d", __func__,
                  Msg::getMsgSimpleName(msg->what),
                  msg->arg1I,
                  msg->arg2I);
        }
        if (mediaPlayer) {
            switch (msg->what) {
                case Msg::MSG_REQUEST_PLAY_OR_PAUSE:
                    if (mediaPlayer->isPlaying()) {
                        mediaPlayer->pause();
                    } else {
                        mediaPlayer->play();
                    }
                    break;
                case Msg::MSG_REQUEST_DESTROY:
                    if (mediaPlayer) {
                        mediaPlayer->destroy();
                    }
                    break;
                case Msg::MSG_REQUEST_START:
                    break;
                case MSG_REQUEST_SEEK_SDL:
                    int increment = msg->arg1I;
                    if (mediaPlayer) {
                        mediaPlayer->seekTo(increment);
                    }
                    break;
            }
        } else {
            if (DEBUG) {
                ALOGE(TAG, "%s media player is null", __func__);
            }
        }
    }
};

int main() {
    mediaPlayer = SDLMediaPlayer::Builder{}
            .withAudioDevice(new SDLAudioDevice())
            .withVideoDevice(new SDLVideoDevice())
            .withMessageListener(new MessageListener())
            .withDebug(true)
            .build();
    mediaPlayer->create();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
//    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/The.Walking.Dead.S04E15.2013.BluRay.720p.x264.AC3-CMCT.mkv");
    mediaPlayer->start();
    mediaPlayer->eventLoop();
    return SUCCESS;
}
