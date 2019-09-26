#include <SDLMediaPlayer.h>
#include <SDLAudioDevice.h>
#include <SDLVideoDevice.h>
#include <message/MessageCenter.h>

SDLMediaPlayer *mediaPlayer = nullptr;

class MessageListener : public IMessageListener {
    const char *const TAG = "MessageListener";
public:
    void onMessage(Msg *msg) override {
        if (DEBUG)
            ALOGD(TAG, "%s what = %s arg1 = %d arg2 = %d", __func__,
                  Msg::getMsgSimpleName(msg->what),
                  msg->arg1,
                  msg->arg2);
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
                case Msg::MSG_REQUEST_SEEK:
                    int increment = msg->arg1;
                    if (mediaPlayer) {
                        mediaPlayer->seekTo(increment);
                    }
                    break;
            }
        } else {
            if (DEBUG) ALOGE(TAG, "%s media player is null", __func__);
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
    mediaPlayer->start();
    mediaPlayer->eventLoop();
    return SUCCESS;
}
