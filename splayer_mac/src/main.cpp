#include <SDL.h>
#include <iostream>
#include "MacMediaPlayer.h"
#include <string>

Mutex *logMutex = new Mutex();

int main() {
    auto *player = new MacMediaPlayer();
    int ret = 0;


//    printf("\n");
//    printf("\x1B[31mTexting\033[0m\t\t");
//    printf("\x1B[32mTexting\033[0m\t\t");
//    printf("\x1B[33mTexting\033[0m\t\t");
//    printf("\x1B[34mTexting\033[0m\t\t");
//    printf("\x1B[35mTexting\033[0m\n");
//
//    printf("\x1B[36mTexting\033[0m\t\t");
//    printf("\x1B[36mTexting\033[0m\t\t");
//    printf("\x1B[36mTexting\033[0m\t\t");
//    printf("\x1B[37mTexting\033[0m\t\t");
//    printf("\x1B[93mTexting\033[0m\n");
//
//    printf("\033[3;42;30mTexting\033[0m\t\t");
//    printf("\033[3;43;30mTexting\033[0m\t\t");
//    printf("\033[3;44;30mTexting\033[0m\t\t");
//    printf("\033[3;104;30mTexting\033[0m\t\t");
//    printf("\033[3;100;30mTexting\033[0m\n");
//
//    printf("\033[3;47;35mTexting\033[0m\t\t");
//    printf("\033[2;47;35mTexting\033[0m\t\t");
//    printf("\033[1;47;35mTexting\033[0m\t\t");
//    printf("\t\t");
//    printf("\n");

    std::string input;
    int exit = 1;
    while (exit) {
        std::getline(std::cin, input);
        if ("create" == input) {
            ret = player->create();
        } else if ("url" == input) {
            ret = player->setDataSource("/Users/biezhihua/Downloads/复仇者联盟.mkv");
        } else if ("prepare" == input) {
            ret = player->prepareAsync();
        }

        if ("exit" == input) {
            exit = 0;
        }
    }

    std::cout << "Main End" << std::endl;
    return 0;
}
