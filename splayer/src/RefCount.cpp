#include "RefCount.h"

RefCount::RefCount() {
    ALOGD(__func__);
}

RefCount::~RefCount() {
    ALOGD(__func__);
}

int RefCount::incrementReference() {
    ALOGD("%s count=%d", "incrementReference", referenceCount);
    return __sync_add_and_fetch(&referenceCount, 1);
}

int RefCount::decrementReference() {
    ALOGD("%s count=%d", "decrementReference", referenceCount);
    return __sync_sub_and_fetch(&referenceCount, 1);

}
