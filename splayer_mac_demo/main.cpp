#include <iostream>
#include <string>
#include <MacMessage.h>
#include <MacAudio.h>

int main() {

    MediaPlayer *mediaPlayer = MediaPlayer::Builder{}
            .withMessage(new MacMessage())
            .withOptions(new MacOptions())
            .withEvent(new MacEvent())
            .withAudio(new MacAudio())
            .withSurface(new MacSurface())
            .build();

//    auto *player = new MacMediaPlayer();
    mediaPlayer->create();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    return mediaPlayer->eventLoop();

}
