#include <SDL.h>
#include <iostream>
#include "MacMediaPlayer.h"
#include <string>

Mutex *logMutex = new Mutex();

int main() {
    MediaPlayer *player = new MacMediaPlayer();
    player->create();
    player->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    player->prepareAsync();
    return 0;
}
