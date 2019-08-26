#include "MacMessage.h"

void MacMessage::onMessage(Event *event, Msg *msg) {
    if (msg) {
        ALOGD("MacMessage", "%s what = %d arg1 = %d arg2 = %d obj = %p", __func__, msg->what, msg->arg1, msg->arg2, msg->obj);
    }
}
