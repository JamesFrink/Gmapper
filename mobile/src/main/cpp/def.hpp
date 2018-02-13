//
// Created by James on 24/01/2018.
//

#ifndef GMAPPER_DEF_HPP
#define GMAPPER_DEF_HPP

#include <jni.h>

struct jniDefs {
    JavaVM* vm = nullptr;

    jclass ArrayListAdapter_class;
    jmethodID ArrayListAdapter_constructor;
    jmethodID ArrayListAdapter_size;
    jmethodID ArrayListAdapter_clear;
    jmethodID ArrayListAdapter_get;
    jmethodID ArrayListAdapter_add;
    jmethodID ArrayListAdapter_set;
    jmethodID ArrayListAdapter_removeAt;
    jmethodID ArrayListAdapter_updateAt;
    jclass GestureJava_class;
    jmethodID GestureJava_constructor;
    jmethodID GestureJava_id;
    jmethodID GestureJava_nameOfGesture;
    jmethodID GestureJava_active;
    jmethodID GestureJava_numberOfRecordings;
    jfieldID GestureJava_trainingSeriesObjects;
    jclass TrainingSeriesObjectJava_class;
    jmethodID TrainingSeriesObjectJava_constructor;
    jmethodID TrainingSeriesObjectJava_numTimesRecognized;
    jfieldID TrainingSeriesObjectJava_lengthOfGesture;
    jfieldID TrainingSeriesObjectJava_timeStamp;
    jclass SettingsJava_class;
    jmethodID SettingsJava_setAllValues;
};

#endif //GMAPPER_DEF_HPP
