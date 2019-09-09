#include <message/MessageDevice.h>

MessageDevice::MessageDevice() {
    msgQueue = nullptr;
    msgListener = nullptr;
}

MessageDevice::~MessageDevice() {
    msgQueue = nullptr;
    msgListener = nullptr;
}

void MessageDevice::run() {
    Msg msg;
    for (;;) {

        if (!msgQueue) {
            ALOGE(TAG, "%s message loop break msgQueue = %p", __func__, msgQueue);
            break;
        }

        msg.free();

        int ret = msgQueue->getMsg(&msg, true);

        if (ret >= 0 && msg.what != -1) {
            if (msgListener != nullptr) {
                msgListener->onMessage(&msg);
            }
        }

        if (msg.what == Msg::MSG_REQUEST_QUIT) {
        }
    }
}

void MessageDevice::setMsgQueue(MessageQueue *msgQueue) {
    MessageDevice::msgQueue = msgQueue;
}

void MessageDevice::setMsgListener(IMessageListener *msgListener) {
    MessageDevice::msgListener = msgListener;
}
