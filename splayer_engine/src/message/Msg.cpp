#include <message/Msg.h>

Msg::Msg() = default;

Msg::~Msg() {
    free();
}

const char *Msg::getMsgSimpleName(int what) {
    switch (what) {
        case MSG_FLUSH:
            return "MSG_FLUSH";
        default:
            return "NONE";
    }
}

void Msg::free() {
    what = -1;
    arg1 = -1;
    arg2 = -1;
}
