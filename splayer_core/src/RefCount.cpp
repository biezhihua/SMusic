#include "RefCount.h"

RefCount::RefCount() {
}

RefCount::~RefCount() {
}

int RefCount::incrementReference() {
    return __sync_add_and_fetch(&referenceCount, 1);
}

int RefCount::decrementReference() {
    return __sync_sub_and_fetch(&referenceCount, 1);

}
