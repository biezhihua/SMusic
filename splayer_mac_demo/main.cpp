#include <SDLMediaPlayer.h>
#include <SDLAudioDevice.h>
#include <SDLVideoDevice.h>
#include <SDLMediaSync.h>
#include <message/MessageCenter.h>

SDLMediaPlayer *mediaPlayer = nullptr;

class MessageListener : public IMessageListener {
    const char *const TAG = "MessageListener";
public:
    void onMessage(Msg *msg) override {
        ALOGD(TAG, "%s what = %s arg1 = %d arg2 = %d", __func__,
              Msg::getMsgSimpleName(msg->what),
              msg->arg1,
              msg->arg2);
        if (mediaPlayer) {
            if (msg->what == Msg::MSG_REQUEST_PLAY_OR_PAUSE) {
                if (mediaPlayer->isPlaying()) {
                    mediaPlayer->pause();
                } else {
                    mediaPlayer->play();
                }
            } else if (msg->what == Msg::MSG_REQUEST_CREATE_OR_DESTROY) {

            } else if (msg->what == Msg::MSG_REQUEST_START_OR_STOP) {

            }
        } else {
            ALOGE(TAG, "%s media player is null", __func__);
        }
    }
};

int main() {
    mediaPlayer = SDLMediaPlayer::Builder{}
            .withMediaSync(new SDLMediaSync())
            .withAudioDevice(new SDLAudioDevice())
            .withVideoDevice(new SDLVideoDevice())
            .withMessageListener(new MessageListener())
            .withDebug(true)
            .build();
    mediaPlayer->create();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->start();
    mediaPlayer->stop();
    return mediaPlayer->eventLoop();
}
