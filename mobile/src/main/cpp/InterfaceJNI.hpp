//
// Created by James on 24/01/2018.
//

#ifndef GMAPPER_INTERFACEJNI_HPP
#define GMAPPER_INTERFACEJNI_HPP
#include <jni.h>
#include <functional>
#include <android/log.h>
#include "Gesture.hpp"
#include "def.hpp"
#include "Spinlock.hpp"

extern struct jniDefs JNIOBJECTS;

namespace INTERFACE {
    bool initializeJVM( JNIEnv *env );
    bool runOutsideOfContext( std::function< bool ( JNIEnv *g_env ) > callback );

    static jobject createReference( JNIEnv* env, jobject object, jobjectRefType refType );
    static jobject createReferenceNoContext( jobject object, jobjectRefType refType );
    void deleteReference( JNIEnv* env, jobject object );
    void deleteReferenceNoContext( jobject object );

    std::string jstring2string ( JNIEnv *env, jstring jStr );

    struct SettingsObjectJava {
        jobject reference;
        void syncSettingsToJava( void );
        void from_json ( JSON settingsObject );
        JSON to_json ( void );

        std::function< void ( bool useWearable ) > wearableChangedCallback = nullptr;
        std::function< void ( bool sendIFTTT, std::string webhookIFTTT ) > IFTTTChangedCallback = nullptr;
        std::function< void ( bool sendOSC, std::string oscHost, int32_t oscPort ) > OSCChangedCallback = nullptr;
        std::function< void ( float startF, int32_t startFT, float stopF, int32_t stopFT, float costThreshold ) > segmentationChangedCallback = nullptr;

        Spinlock lock;
        bool useWearable = false; bool sendIFTTT = false; std::string webhookKeyIFTTT = "yourKeyHere";
        bool sendOSC = false; std::string oscHost = "localhost"; int32_t oscPort = 8888; float startF = 2.0f;
                int32_t startFT = 101; float stopF = 1.2f; int32_t stopFT = 300; float costThreshold = 100.0f;
    };

    struct ArrayListAdapterJava {
        jobject createInterface ( JNIEnv *env );
        void setInstance ( jobject object );
        static jobject getAt( JNIEnv* env, jobject inArray ,int32_t index, jobjectRefType refType );
        static jobject getAtNoContext( jobject inArray ,int32_t index, jobjectRefType refType );
        static jboolean addNoContext( jobject inArray, jobject object );
        static jobject removeAtNoContext( jobject inArray, int32_t index );
        static void clear( JNIEnv* env, jobject inArray );
        static void clearNoContext( jobject inArray );
        static void notifyChangedNoContext( jobject inArray, int32_t index );

        jobject reference = nullptr;
    //--
    };

    struct GestureJava {
        static jobject createJavaGesture ( JNIEnv *env, Gesture& g );
        static jobject createJavaGestureNoContext ( Gesture& g );
        static void setGestureNameNoContext ( jobject refToElement, std::string& nameOfGesture );
        static void setGestureActive ( JNIEnv* env, jobject refToElement, bool active );
        static void setGestureActiveNoContext ( jobject refToElement, bool active );
        static void setGestureNumberOfExamples( JNIEnv* env, jobject refToElement, int32_t numberOfExamples );
        static void setGestureNumberOfExamplesNoContext ( jobject refToElement, int32_t numberOfExamples );

        //static jobject getTrainingSeriesObjectsField( jobject inputGesture );
    };

    struct TrainingSeriesObjectJava {
        static void addSingleSeriesToGestureJavaNoContext ( jobject gestureJava, trainingSeriesObject& t, int32_t newNumExamples );
        static void removeSeriesNoContext( jobject inArray, int32_t indexOfGesture, int32_t indexOfTrainingGesture, int32_t newNumExamples );
        static void removeSeriesFromGestureNoContext( jobject gestureJava, int32_t index );
        static jobject createTrainingSeriesObject ( JNIEnv *env, trainingSeriesObject& t );
        static void addTrainingSeriesToGestureJava ( JNIEnv *env, jobject gestureJava, std::vector< trainingSeriesObject >& t );
        static jobject getTrainingSeriesAsArrayList ( JNIEnv *env, std::vector< trainingSeriesObject >& t );
        static void setNumberOfTimesRecognized( JNIEnv *env, jobject gestureJavaRecognized, int32_t indexOfTrainingObject, int32_t numberOfTimesRecognized );
        static void setNumberOfTimesRecognizedNoContext( jobject gestureJavaRecognized, int32_t indexOfTrainingObject, int32_t numberOfTimesRecognized );
        static void clearTrainingDataNoContext ( jobject inArray, int32_t indexOfGesture );
    };
}

#endif //GMAPPER_INTERFACEJNI_HPP
