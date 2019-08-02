#include <iostream>
#include "MacMediaPlayer.h"


int main() {
    auto *player = new MyMediaPlayer();

    int ret = player->create();

    std::cout << "Main Create Result " << ret << std::endl;

    return 0;
}