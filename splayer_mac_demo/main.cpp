#include <SDLMediaPlayer.h>
#include <SDLAudioDevice.h>
#include <SDLVideoDevice.h>
#include <message/MessageDevice.h>
#include <SDLMediaSync.h>

SDLMediaPlayer *mediaPlayer = nullptr;

int main() {
    SDLMediaPlayer *mediaPlayer = SDLMediaPlayer::Builder{}
            .withMediaSync(new SDLMediaSync())
            .withAudioDevice(new SDLAudioDevice())
            .withVideoDevice(new SDLVideoDevice())
            .withDebug(true)
            .build();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    mediaPlayer->start();
    return mediaPlayer->eventLoop();
}
