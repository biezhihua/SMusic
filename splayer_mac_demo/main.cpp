#include <MacMediaPlayer.h>
#include <MacAudioDevice.h>

int main() {
    MacMediaPlayer *mediaPlayer = MacMediaPlayer::Builder{}
            .withMediaSync(new MacMediaSync())
            .withAudioDevice(new MacAudioDevice())
            .withDebug(true)
            .build();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    mediaPlayer->start();
    return mediaPlayer->eventLoop();
}
