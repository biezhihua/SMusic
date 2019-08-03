#include <iostream>
#include "MacMediaPlayer.h"
#include <SDL.h>

/**
 * https://github.com/Twinklebear/TwinklebearDev-Lessons
 * https://blog.csdn.net/leixiaohua1020/article/details/40680907
 */
int main() {

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Quit();

//    auto *player = new MacMediaPlayer();
//
//    int ret = 0;
//
//    std::string input;
//    int exit = 1;
//    while (exit) {
//        std::getline(std::cin, input);
//
//        if ("create" == input) {
//            ret = player->create();
//            std::cout << "Main create result " << ret << std::endl;
//        } else if ("url" == input) {
//            ret = player->setDataSource("I'm is url");
//            std::cout << "Main setDataSource result " << ret << std::endl;
//        } else if ("prepare" == input) {
//            ret = player->prepareAsync();
//            std::cout << "Main prepareAsync result " << ret << std::endl;
//        }
//
//        if ("exit" == input) {
//            exit = 0;
//        }
//    }
//
//    std::cout << "Main End" << std::endl;
    return 0;
}

