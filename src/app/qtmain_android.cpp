/*
    Copyright (c) 2009-2011, BogDan Vatra <bog_dan_ro@yahoo.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the  BogDan Vatra <bog_dan_ro@yahoo.com> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY BogDan Vatra <bog_dan_ro@yahoo.com> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL BogDan Vatra <bog_dan_ro@yahoo.com> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <android/log.h>
#include <pthread.h>
#include <QSemaphore>
#include <QDir>
#include <QDebug>
#include <qglobal.h>

#include <stdlib.h>

#include <jni.h>

static JavaVM *m_javaVM = NULL;
static JNIEnv *m_env = NULL;
static jobject objptr;
static QSemaphore m_quitAppSemaphore;
static QList<QByteArray> m_applicationParams;
static const char * const QtApplicationClassPathName = "org/kde/necessitas/origo/QtApplication";

extern "C" int main(int, char **); //use the standard main method to start the application
static void * startMainMethod(void * /*data*/)
{

    char **  params;
    params=(char**)malloc(sizeof(char*)*m_applicationParams.length());
    for (int i=0;i<m_applicationParams.size();i++)
        params[i]= (char*)m_applicationParams[i].constData();

    int ret = main(m_applicationParams.length(), params);
    
    qDebug()<<"MainMethod finished, it's time to cleanup ";
    free(params);
    Q_UNUSED(ret);

    JNIEnv* env;
    if (m_javaVM->AttachCurrentThread(&env, NULL)<0)
    {
        qCritical()<<"AttachCurrentThread failed";
        return false;
    }
    jclass applicationClass = env->GetObjectClass(objptr);
    if (applicationClass){
        jmethodID quitApp = env->GetStaticMethodID(applicationClass, "quitApp", "()V");
        env->CallStaticVoidMethod(applicationClass, quitApp);
    }
    m_javaVM->DetachCurrentThread();
    return NULL;
}

static jboolean startQtApp(JNIEnv* env, jobject /*object*/, jstring paramsString, jstring environmentString)
{
    qDebug()<<"startQtApp";
    const char * nativeString = env->GetStringUTFChars(environmentString, 0);
    QByteArray string=nativeString;
    env->ReleaseStringUTFChars(environmentString, nativeString);
    m_applicationParams=string.split('\t');
    qDebug()<<"environmentString"<<string<<m_applicationParams;
    foreach (string, m_applicationParams)
        if (putenv(string.constData()))
            qWarning()<<"Can't set environment"<<string;

    nativeString = env->GetStringUTFChars(paramsString, 0);
    string=nativeString;
    env->ReleaseStringUTFChars(paramsString, nativeString);

    qDebug()<<"paramsString"<<string;
    m_applicationParams=string.split('\t');

    // Go home
    QDir::setCurrent(QDir::homePath());

    pthread_t appThread;
    return pthread_create(&appThread, NULL, startMainMethod, NULL)==0;
}


static JNINativeMethod methods[] = {
    {"startQtApp", "(Ljava/lang/String;Ljava/lang/String;)V", (void *)startQtApp}
};

/*
* Register several native methods for one class.
*/
static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz=env->FindClass(className);
    if (clazz == NULL)
    {
        __android_log_print(ANDROID_LOG_FATAL,"Qt", "Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    jmethodID constr = env->GetMethodID(clazz, "<init>", "()V");
    if(!constr) {
        __android_log_print(ANDROID_LOG_FATAL,"Qt", "Native registration unable to find  constructor for class '%s'", className);
        return JNI_FALSE;;
    }
    jobject obj = env->NewObject(clazz, constr);
    objptr = env->NewGlobalRef(obj);
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0)
    {
        __android_log_print(ANDROID_LOG_FATAL,"Qt", "RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

/*
* Register native methods for all classes we know about.
*/
static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, QtApplicationClassPathName, methods, sizeof(methods) / sizeof(methods[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

typedef union {
    JNIEnv* nativeEnvironment;
    void* venv;
} UnionJNIEnvToVoid;

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    __android_log_print(ANDROID_LOG_INFO,"Qt", "qt start");
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    m_javaVM = 0;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_FATAL,"Qt","GetEnv failed");
        return -1;
    }
    m_env = uenv.nativeEnvironment;
    if (!registerNatives(m_env))
    {
        __android_log_print(ANDROID_LOG_FATAL, "Qt", "registerNatives failed");
        return -1;
    }
    m_javaVM = vm;
    __android_log_print(ANDROID_LOG_INFO,"Qt", "JNI_OnLoad OK");
    return JNI_VERSION_1_4;
}
