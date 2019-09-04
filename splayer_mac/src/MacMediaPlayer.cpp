#include <MacMediaPlayer.h>

int MacMediaPlayer::eventLoop() {
    if (getMediaSync() != nullptr) {
        getMediaSync()->run();
    }
    return 0;
}
