#include <jni.h>
#include <string>
#include <fcntl.h>
#include <unistd.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_MainActivity_openSwitchFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    int ret = open("/dev/fpga_push_switch", O_RDWR);
    return ret;
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_closeFromJNI(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    close(fd);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_MainActivity_readSwitchFromJNI(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    unsigned char buf[9];
    int i;
    read(fd, buf, sizeof(buf));
    for(i=0; i<9; i++) {
        if (buf[i] == 1) {
            return i + 1;
        }
    }
    return -1;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myapplication_MyReaderService_readSwitchFromJNI2(
        JNIEnv *env,
        jobject /* this */,
        jint fd) {
    unsigned char buf[9];
    int i;
    read(fd, buf, sizeof(buf));
    for(i=0; i<9; i++) {
        if (buf[i] == 1) {
            return i + 1;
        }
    }
    return -1;
}