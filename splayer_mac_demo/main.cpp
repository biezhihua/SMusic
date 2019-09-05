#include <SDLMediaPlayer.h>
#include <SDLAudioDevice.h>

int main() {
    SDLMediaPlayer *mediaPlayer = SDLMediaPlayer::Builder{}
            .withMediaSync(new SDLMediaSync())
            .withAudioDevice(new SDLAudioDevice())
            .withDebug(true)
            .build();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    mediaPlayer->start();
    return mediaPlayer->eventLoop();
}
