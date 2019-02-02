//
// Created by biezhihua on 2019/2/2.
//

#ifndef SMUSIC_SUBTHREADCALLJAVACONTEXT_H
#define SMUSIC_SUBTHREADCALLJAVACONTEXT_H


#include <jni.h>

class SubThreadCallJavaContext {
public:
    JavaVM *javaVM;
    jobject obj;
};

#endif //SMUSIC_SUBTHREADCALLJAVACONTEXT_H
