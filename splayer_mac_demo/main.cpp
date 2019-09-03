
#include <MacMediaPlayer.h>

int main() {
    auto *mediaPlayer = new MacMediaPlayer();
    mediaPlayer->setDataSource("/Users/biezhihua/Downloads/寄生虫.mp4");
    mediaPlayer->prepareAsync();
    mediaPlayer->start();

    while (true) {

    }
    return 0;
}
