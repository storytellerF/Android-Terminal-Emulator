/*
 * Copyright (C) 2007, 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Exec"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <cerrno>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <csignal>

#include "termExec.h"

static void setPtyWindowSize(JNIEnv *env, jobject clazz,
                             jint fd, jint row, jint col, jint xPixel,
                             jint yPixel) {
    struct winsize sz{};

    sz.ws_row = row;
    sz.ws_col = col;
    sz.ws_xpixel = xPixel;
    sz.ws_ypixel = yPixel;

    // TODO: handle the situation, when the file descriptor is incompatible with TIOCSWINSZ (e.g. not from /dev/ptmx)
    if (ioctl(fd, TIOCSWINSZ, &sz) == -1)
        env->ThrowNew(env->FindClass("java/io/IOException"), "Failed to issue TIOCSWINSZ ioctl");
}

// tcgetattr /tcsetattr are not part of Bionic at API level 4. Here's a compatible version.

static __inline__ int my_tcgetattr(int fd, struct termios *s) {
    return ioctl(fd, TCGETS, s);
}

static __inline__ int my_tcsetattr(int fd, const struct termios *s) {
    return ioctl(fd, TCSETS, (void *) s);
}

static void setPtyUtf8Mode(JNIEnv *env, jobject clazz, jint fd, jboolean utf8Mode) {
    struct termios tios{};

    if (my_tcgetattr(fd, &tios) != 0)
        env->ThrowNew(env->FindClass("java/io/IOException"), "Failed to get terminal attributes");

    if (utf8Mode) {
        tios.c_iflag |= IUTF8;
    } else {
        tios.c_iflag &= ~IUTF8;
    }

    if (my_tcsetattr(fd, &tios) != 0)
        env->ThrowNew(env->FindClass("java/io/IOException"), "Failed to set terminal UTF-8 mode");
}

extern "C"
JNIEXPORT void JNICALL
Java_jackpal_androidterm_Exec_setPtyWindowSizeInternal(JNIEnv *env, jclass clazz, jint fd, jint row,
                                                       jint col, jint xpixel, jint ypixel) {
    setPtyWindowSize(env, clazz, fd, row, col, xpixel, ypixel);
}
extern "C"
JNIEXPORT void JNICALL
Java_jackpal_androidterm_Exec_setPtyUTF8ModeInternal(JNIEnv *env, jclass clazz, jint fd,
                                                     jboolean utf8_mode) {
    setPtyUtf8Mode(env, clazz, fd, utf8_mode);
}