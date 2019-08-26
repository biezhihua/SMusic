#include <iostream>
#include <string>

#include "MacMediaPlayer.h"
#include "MediaPlayer.h"

int main() {
    MediaPlayer *mediaPlayer = MacMediaPlayer::create();
    mediaPlayer->create();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    return mediaPlayer->eventLoop();

}
