#include <iostream>
#include <string>
#include <MacMediaPlayer.h>

Mutex *LOG_MUTEX = new Mutex();

int main() {
    MediaPlayer *player = new MacMediaPlayer();
    player->create();
    player->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    player->prepareAsync();
    return 0;
}
