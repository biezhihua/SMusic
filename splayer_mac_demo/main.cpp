#include <iostream>
#include <string>
#include <MacMediaPlayer.h>

int main() {
    auto *player = new MacMediaPlayer();
    player->create();
    player->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    player->prepareAsync();
    return player->eventLoop();
}
