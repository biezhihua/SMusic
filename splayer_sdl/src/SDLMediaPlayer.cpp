#include <SDLMediaPlayer.h>

int SDLMediaPlayer::eventLoop() {
    if (getMediaSync() != nullptr) {
        getMediaSync()->run();
    }
    return 0;
}
