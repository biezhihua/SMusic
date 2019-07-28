
#include "../include/Thread.h"

static void *runThread(void *data) {
    ALOGD(__func__);
    Thread *thread = static_cast<Thread *>(data);
    if (thread) {
#ifdef __ANDROID__
        pthread_setname_np(pthread_self(), thread->name->c_str());
#elif __MAC_10_14_4
        pthread_setname_np(thread->name->c_str());
#else
        //
#endif
        thread->retval = thread->func(thread->data);
    }
    return nullptr;
}

Thread::Thread(int (*func)(void *), void *data, const char *name) {
    ALOGD(__func__);
    Thread::func = func;
    Thread::data = data;
    Thread::name = new string(name);
    Thread::retval = pthread_create(&id, nullptr, runThread, this);
}

Thread::~Thread() {
    ALOGD(__func__);
    delete name;
}

int Thread::waitThread() {
    ALOGD(__func__);
    pthread_join(id, nullptr);
    return retval;
}

void Thread::detachThread() {
    ALOGD(__func__);
    pthread_detach(id);
}
