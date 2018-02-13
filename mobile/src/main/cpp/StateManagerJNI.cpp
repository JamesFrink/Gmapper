#include <jni.h>
#include <string>
#include <android/log.h>
#include "InterfaceJNI.hpp"
#include "GestureClassifier.hpp"
#include "def.hpp"

extern struct jniDefs JNIOBJECTS;
/*
 * TO ANYONE READING: USE SWIG OR A DIFFERENT METHOD TO ACCESS / PARSE NATIVE CODE!
 * If you are wondering why I persisted:
 * - interesting learning experience
 * - was already 1/2 way there when I discovered dynamic accessors and that SWIG worked with android
 *
 * Behold the boilerplate ( more beautiful boilerplate hidden away in INTERFACE:: )
 */

static INTERFACE::SettingsObjectJava settings;
static INTERFACE::ArrayListAdapterJava sharedMemoryInterface;
static GestureClassifier gestureInstance;

// nl.jamesfrink.gmapper.StateManager instance
static jclass stateManagerClass;
static jobject stateManagerJava;

jmethodID setCurrentClassificationStateCallback;
jmethodID classifiedGestureCallback;
//--

// Just to be sure
static bool alreadyInitialized = false;
// --

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_initializeJNI ( JNIEnv * const env, jobject pThis, jobject sharedMemory, jstring directory )
{
    if ( !alreadyInitialized )
    {
        __android_log_print(ANDROID_LOG_WARN, "JNI", "INITIALIZING JNI");

        // get JVM instance
        if ( !INTERFACE::initializeJVM( env ) )
            return jboolean( false );

        // Set callbacks into Java
        jobject stateManager = env->GetObjectClass( pThis );

        stateManagerClass = static_cast< jclass > ( env->NewGlobalRef( stateManager ) );
        stateManagerJava = env->NewGlobalRef( pThis );
        setCurrentClassificationStateCallback = env->GetMethodID( stateManagerClass, "setCurrentClassificationState", "(I)V" );
        classifiedGestureCallback = env->GetMethodID( stateManagerClass, "classifiedGestureCallback", "(I)V" );

        sharedMemoryInterface.setInstance( env->NewGlobalRef( sharedMemory ) );
        std::string directoryString = INTERFACE::jstring2string( env, directory );
        gestureInstance.initializeInterface( &sharedMemoryInterface, &settings, directoryString );

        env->DeleteLocalRef(stateManager); // No longer needed

        gestureInstance.classifiedGestureCallback = [  ]( int32_t gestureIndex )
        {
            return INTERFACE::runOutsideOfContext( [ gestureIndex ]( JNIEnv *g_env ) {
                g_env->CallVoidMethod( stateManagerJava, classifiedGestureCallback,
                                       gestureIndex );
                return true;
            });
        };

        gestureInstance.setCurrentClassificationState = [  ]( int32_t state ) {
            return INTERFACE::runOutsideOfContext( [ state ]( JNIEnv *g_env ) {
                g_env->CallVoidMethod( stateManagerJava, setCurrentClassificationStateCallback,
                                      state );
                return true;
            });
        };

        alreadyInitialized = true;
    } else {
        __android_log_print( ANDROID_LOG_WARN, "JNI", "ALREADY INIT" );
        jobject stateManager = env->GetObjectClass( pThis );

        stateManagerClass = static_cast< jclass > ( env->NewGlobalRef( stateManager ) );
        stateManagerJava = env->NewGlobalRef( pThis );
        setCurrentClassificationStateCallback = env->GetMethodID( stateManagerClass, "setCurrentClassificationState", "(I)V" );
        sharedMemoryInterface.setInstance( env->NewGlobalRef( sharedMemory ) );
        env->DeleteLocalRef(stateManager); // No longer needed

        gestureInstance.synchronizeSharedList( &sharedMemoryInterface );
    }

    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_loadSettings ( JNIEnv *env, jobject, jobject settingsToSet )
{
    settings.reference = env->NewGlobalRef( settingsToSet );
    gestureInstance.loadSettings(  );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_settingsSetWearable ( JNIEnv *env, jobject, jboolean useWear )
{
    settings.lock.lock(  );
    settings.useWearable = useWear;
    if ( settings.wearableChangedCallback != nullptr )
        settings.wearableChangedCallback( useWear );
    settings.lock.unlock(  );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_settingsSetIFTTT ( JNIEnv *env, jobject, jboolean sendIFTTT, jstring webhookKey )
{
    std::string webHookString = INTERFACE::jstring2string( env, webhookKey );
    settings.lock.lock(  );
    settings.sendIFTTT = sendIFTTT;
    settings.webhookKeyIFTTT = webHookString;
    settings.lock.unlock(  );
    if ( settings.IFTTTChangedCallback != nullptr )
        settings.IFTTTChangedCallback( sendIFTTT, webHookString );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_settingsSetOSC ( JNIEnv *env, jobject, jboolean sendOSC, jstring host, jint port )
{
    std::string hostString = INTERFACE::jstring2string( env, host );
    settings.lock.lock(  );
    settings.sendOSC = sendOSC;
    settings.oscHost = hostString;
    settings.oscPort = port;
    settings.lock.unlock(  );
    if ( settings.OSCChangedCallback != nullptr )
        settings.OSCChangedCallback( sendOSC, hostString, port );
    return jboolean( true );
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_settingsSetSegmentation ( JNIEnv *env, jobject, jfloat startF, jint startFT, jfloat stopF, jint stopFT, jfloat costThreshold )
{
    settings.lock.lock(  );
    settings.startF = startF;
    settings.startFT = startFT;
    settings.stopF = stopF;
    settings.stopFT = stopFT;
    settings.costThreshold = costThreshold;
    settings.lock.unlock(  );
    if ( settings.segmentationChangedCallback != nullptr )
        settings.segmentationChangedCallback( startF, startFT, stopF, stopFT, costThreshold );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_addNewGesture ( JNIEnv *env, jobject, jstring nameOfGesture )
{
    gestureInstance.addNewGesture( INTERFACE::jstring2string( env, nameOfGesture ) );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_setNameOfGesture ( JNIEnv *env, jobject, jint id, jstring nameOfGesture )
{
    gestureInstance.setNameOfGesture( id, INTERFACE::jstring2string( env, nameOfGesture ) );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_removeGesture ( JNIEnv *env, jobject, jint id )
{
    gestureInstance.removeGesture( id );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_setGestureActive ( JNIEnv *env, jobject, jint id, jboolean active )
{
    gestureInstance.setGestureActive( id, active );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_clearAllExampleData ( JNIEnv *env, jobject, jint id )
{
    gestureInstance.clearAllExampleData( id );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_removeTrainingSeriesObject ( JNIEnv *env, jobject, jint id, jint index )
{
    gestureInstance.removeTrainingSeriesObject ( id, index );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_waitForExampleData ( JNIEnv *env, jobject, jint id )
{
    gestureInstance.waitForExampleData( id );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_setClassificationState ( JNIEnv *env, jobject, jboolean input )
{
    gestureInstance.setClassificationState( input );
    return jboolean( true );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_nl_jamesfrink_gmapper_StateManager_getGesture ( JNIEnv *env, jobject pThis, jint id )
{   // Get gesture in java
    // Call to C++ side of things:
    Gesture g = gestureInstance.getGesture( id );
    return INTERFACE::GestureJava::createJavaGesture( env, g );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_nl_jamesfrink_gmapper_StateManager_getGestureByIndex ( JNIEnv *env, jobject pThis, jint index )
{   // Get gesture in java
    // Call to C++ side of things:
    Gesture g = gestureInstance.getGestureByIndex( index );
    return INTERFACE::GestureJava::createJavaGesture( env, g );
}


extern "C"
JNIEXPORT jint JNICALL
Java_nl_jamesfrink_gmapper_StateManager_getNumGestures ( JNIEnv *env, jobject )
{
    return gestureInstance.getNumGestures(  );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_addAccelData ( JNIEnv *env, jobject, jfloat x, jfloat y, jfloat z )
{
    gestureInstance.newAccelData( x, y, z );
    return jboolean( true );
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_nl_jamesfrink_gmapper_StateManager_destroyJNI ( JNIEnv *env, jobject pThis )
{
    __android_log_print( ANDROID_LOG_DEBUG, "JNI", "Destroying instances" );
    // Isn't necessary to delete these?
    env->DeleteGlobalRef( sharedMemoryInterface.reference );
    env->DeleteGlobalRef( stateManagerJava );

    gestureInstance.setCurrentClassificationState = []( int32_t state ) {
        return INTERFACE::runOutsideOfContext([state](JNIEnv *g_env) {
            // Empty callback until recreated view
            __android_log_print(ANDROID_LOG_DEBUG, "nl.jamesfrink.gmapper.Gesture", "id recognized in JNI background %d", state );
            return true;
        });
    };

    //alreadyInitialized = false;// TODO: IF USE THIS FIX MEMORY LEAKS
    // TODO: find out if this needs to happen or if all memory is freed on destroy anyway
    return jboolean( true );
}