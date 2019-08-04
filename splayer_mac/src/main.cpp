#include <SDL.h>
#include <iostream>
#include "MacMediaPlayer.h"


int main() {
    auto *player = new MacMediaPlayer();

    int ret = 0;

    std::string input;
    int exit = 1;
    while (exit) {
        std::getline(std::cin, input);

        if ("create" == input) {
            ret = player->create();
            std::cout << "Main create result " << ret << std::endl;
        } else if ("url" == input) {
            ret = player->setDataSource("/Users/biezhihua/Downloads/复仇者联盟.mkv");
            std::cout << "Main setDataSource result " << ret << std::endl;
        } else if ("prepare" == input) {
            ret = player->prepareAsync();
            std::cout << "Main prepareAsync result " << ret << std::endl;
        }

        if ("exit" == input) {
            exit = 0;
        }
    }

    std::cout << "Main End" << std::endl;
    return 0;
}
