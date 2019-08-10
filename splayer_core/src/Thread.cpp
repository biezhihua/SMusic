#include "Thread.h"

static void *runThread(void *data) {
    auto *thread = static_cast<Thread *>(data);
    if (thread) {
#ifdef __ANDROID__
        ALOGD("%s android", __func__);
        pthread_setname_np(pthread_self(), thread->name->c_str());
#elif __MAC_10_14_4
        pthread_setname_np(thread->name->c_str());
#else
        ALOGD("%s else", __func__);
        //
#endif
        thread->retval = thread->func(thread->data);
    }
    return nullptr;
}

Thread::Thread(int (*func)(void *), void *data, const char *name) {
    Thread::func = func;
    Thread::data = data;
    Thread::name = new string(name);
    Thread::retval = pthread_create(&id, nullptr, runThread, this);
}

Thread::~Thread() {
    delete name;
}

int Thread::waitThread() {
    pthread_join(id, nullptr);
    return retval;
}

void Thread::detachThread() {
    pthread_detach(id);
}
