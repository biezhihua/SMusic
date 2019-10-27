#include <message/Msg.h>

Msg::Msg() = default;

Msg::~Msg() {
    free();
}

void Msg::free() {
    what = -1;
    arg1I = -1;
    arg2I = -1;
    arg1F = -1.F;
    arg2F = -1.F;
}
