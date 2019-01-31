#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_bzh_smusic_Demo_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from FFmpeg C++";
    return env->NewStringUTF(hello.c_str());
}
