//
// Created by James on 24/01/2018.
//

#include "InterfaceJNI.hpp"

namespace INTERFACE {
    bool initializeJVM( JNIEnv *env )
    {
        // get JVM instance
        if ( !env->GetJavaVM( &JNIOBJECTS.vm ) < 0 ) {
            __android_log_print(ANDROID_LOG_WARN, "JNI", "Failed to get JVM");
            return false;
        }
        // Initialize ArrayList object
        JNIOBJECTS.ArrayListAdapter_class             = reinterpret_cast< jclass >( env->NewGlobalRef( env->FindClass( "nl/jamesfrink/gmapper/ArrayListAdapter" ) ) );
        JNIOBJECTS.ArrayListAdapter_size        = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "size", "()I" );
        JNIOBJECTS.ArrayListAdapter_clear        = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "clear", "()V" );
        JNIOBJECTS.ArrayListAdapter_get         = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "get", "(I)Ljava/lang/Object;" );
        JNIOBJECTS.ArrayListAdapter_add         = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "add", "(Ljava/lang/Object;)Z" );
        JNIOBJECTS.ArrayListAdapter_set         = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "set", "(ILjava/lang/Object;)Ljava/lang/Object;" );
        JNIOBJECTS.ArrayListAdapter_removeAt    = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "remove", "(I)Ljava/lang/Object;" );
        JNIOBJECTS.ArrayListAdapter_updateAt    = env->GetMethodID( JNIOBJECTS.ArrayListAdapter_class, "update", "(I)V" );
        // Initialize nl.jamesfrink.gmapper.Gesture object
        JNIOBJECTS.GestureJava_class = reinterpret_cast< jclass >( env->NewGlobalRef( env->FindClass( "nl/jamesfrink/gmapper/Gesture" ) ) );
        JNIOBJECTS.GestureJava_constructor = env->GetMethodID( JNIOBJECTS.GestureJava_class, "<init>", "(IZLjava/lang/String;I)V" );
        // Methods/Fields
        JNIOBJECTS.GestureJava_id                      = env->GetMethodID( JNIOBJECTS.GestureJava_class, "setId", "(I)V" );
        JNIOBJECTS.GestureJava_nameOfGesture           = env->GetMethodID( JNIOBJECTS.GestureJava_class, "setNameOfGesture", "(Ljava/lang/String;)V" );
        JNIOBJECTS.GestureJava_active                  = env->GetMethodID( JNIOBJECTS.GestureJava_class, "setActive", "(Z)V" );
        JNIOBJECTS.GestureJava_numberOfRecordings      = env->GetMethodID( JNIOBJECTS.GestureJava_class, "setNumberOfRecordings", "(I)V" );
        JNIOBJECTS.GestureJava_trainingSeriesObjects   = env->GetFieldID( JNIOBJECTS.GestureJava_class, "trainingSeriesObjects", "Lnl/jamesfrink/gmapper/ArrayListAdapter;" );
        // Initialize nl.jamesfrink.gmapper.TrainingSeriesObject object
        JNIOBJECTS.TrainingSeriesObjectJava_class = reinterpret_cast< jclass >( env->NewGlobalRef( env->FindClass( "nl/jamesfrink/gmapper/TrainingSeriesObject" ) ) );;
        JNIOBJECTS.TrainingSeriesObjectJava_constructor = env->GetMethodID( JNIOBJECTS.TrainingSeriesObjectJava_class, "<init>", "(ILjava/lang/String;I)V" );
        // Method/Fields
        JNIOBJECTS.TrainingSeriesObjectJava_numTimesRecognized  = env->GetMethodID( JNIOBJECTS.TrainingSeriesObjectJava_class, "setNumTimesRecognized", "(I)V" );
        JNIOBJECTS.TrainingSeriesObjectJava_lengthOfGesture = env->GetFieldID( JNIOBJECTS.TrainingSeriesObjectJava_class, "lengthOfGesture", "I" );
        JNIOBJECTS.TrainingSeriesObjectJava_timeStamp       = env->GetFieldID( JNIOBJECTS.TrainingSeriesObjectJava_class, "timeStamp", "Ljava/lang/String;" );
        // Initialize SettingsObject
        JNIOBJECTS.SettingsJava_class = reinterpret_cast< jclass >( env->NewGlobalRef( env->FindClass( "nl/jamesfrink/gmapper/SettingsCallback" ) ) );
        // Field
        JNIOBJECTS.SettingsJava_setAllValues = env->GetMethodID( JNIOBJECTS.SettingsJava_class, "setAllValues", "(ZZLjava/lang/String;ZLjava/lang/String;IFIFIF)V" );

        return true;
    }

    bool runOutsideOfContext( std::function< bool ( JNIEnv *g_env ) > callback ) {
        JNIEnv *g_env = nullptr;
        bool alreadyAttached = true;
        // double check it's all ok
        int getEnvStat = JNIOBJECTS.vm->GetEnv(reinterpret_cast< void ** >( &g_env ), JNI_VERSION_1_6);
        // Check if this thread is not attached to the JVM
        if (getEnvStat == JNI_EDETACHED) {
            alreadyAttached = false;

            if ( JNIOBJECTS.vm->AttachCurrentThread( &g_env, NULL ) != 0 ) {
                __android_log_print(ANDROID_LOG_WARN, "JNI", "Failed to attach");
                return false;
            }
        }
        // Call the actual callback
        bool success = callback(g_env);

        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
        }
        // Detach thread if necessary
        if (!alreadyAttached)
            JNIOBJECTS.vm->DetachCurrentThread();

        return success;
    }

    jobject createReference( JNIEnv* env, jobject object, jobjectRefType refType )
    {
        switch ( refType )
        {
            case jobjectRefType::JNIGlobalRefType:
                object = env->NewGlobalRef( object );
                break;
            case jobjectRefType::JNIWeakGlobalRefType:
                object = env->NewWeakGlobalRef( object );
                break;
            case jobjectRefType::JNILocalRefType:
                object = env->NewLocalRef( object );
                break;
            default:
                break;
        }
        return object;
    }

    jobject createReferenceNoContext( jobject object, jobjectRefType refType )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            object = createReference( g_env, object, refType );
            return true;
        } );
        return object;
    }

    void deleteReference( JNIEnv* env, jobject object )
    {
        jobjectRefType refType = env->GetObjectRefType( object );
        switch ( refType )
        {
            case jobjectRefType::JNIGlobalRefType:
                env->DeleteGlobalRef( object );
                break;
            case jobjectRefType::JNIWeakGlobalRefType:
                env->DeleteWeakGlobalRef( object );
                break;
            case jobjectRefType::JNILocalRefType:
                env->DeleteLocalRef( object );
                break;
            default:
                break;
        }
    }

    void deleteReferenceNoContext( jobject object )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            deleteReference( g_env, object );
            return true;
        } );
    }

    std::string jstring2string ( JNIEnv *env, jstring jStr ) {
        if ( !jStr )
            return "";

        const jclass stringClass = env->GetObjectClass( jStr );
        const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
        const jbyteArray stringJbytes = ( jbyteArray ) env->CallObjectMethod( jStr, getBytes, env->NewStringUTF( "UTF-8" ) );

        size_t length = ( size_t ) env->GetArrayLength( stringJbytes );
        jbyte* pBytes = env->GetByteArrayElements( stringJbytes, NULL );

        std::string ret = std::string( ( char * ) pBytes, length );
        env->ReleaseByteArrayElements( stringJbytes, pBytes, JNI_ABORT );

        env->DeleteLocalRef( stringJbytes );
        env->DeleteLocalRef( stringClass );
        return ret;
    }
    // SettingsObjectJava
    // ---------------------------------------------------------------------------------------------
    /*
     *
     *  this->useWearable = useWearable;
        this->sendIFTTT = sendIFTTT;
        this->webhookKeyIFTTT = webhookKeyIFTTT;
        this->sendOSC = sendOSC;
        this->oscHost = oscHost;
        this->oscPort = oscPort;
        this->startF = startF;
        this->startFT = startFT;
        this->stopF = stopF;
        this->stopFT = stopFT;
     */
    void SettingsObjectJava::syncSettingsToJava( void )
    {
        lock.lock(  );
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jstring webhookKeyIFTTTJava = g_env->NewStringUTF( webhookKeyIFTTT.c_str(  ) );
            jstring oscHostJava = g_env->NewStringUTF( oscHost.c_str(  ) );
            g_env->CallVoidMethod( reference, JNIOBJECTS.SettingsJava_setAllValues,
                                  jboolean( useWearable ), jboolean( sendIFTTT ), webhookKeyIFTTTJava,
                                  jboolean( sendOSC ), oscHostJava, jint( oscPort ), startF, jint( startFT ),
                                  stopF, jint( stopFT ), costThreshold );
            g_env->DeleteLocalRef( webhookKeyIFTTTJava );
            g_env->DeleteLocalRef( oscHostJava );
            return true;
        } );
        lock.unlock(  );
    }

    void SettingsObjectJava::from_json ( JSON settingsObject )
    {
        lock.lock(  );
        useWearable = settingsObject[ "useWearable" ];
        sendIFTTT = settingsObject[ "sendIFTTT" ];
        webhookKeyIFTTT = settingsObject[ "webhookKeyIFTTT" ];
        sendOSC = settingsObject[ "sendOSC" ];
        oscHost = settingsObject[ "oscHost" ];
        oscPort = settingsObject[ "oscPort" ];
        startF = settingsObject[ "startF" ];
        startFT = settingsObject[ "startFT" ];
        stopF = settingsObject[ "stopF" ];
        stopFT = settingsObject[ "stopFT" ];
        costThreshold = settingsObject[ "costThreshold" ];
        lock.unlock(  );
    }

    JSON SettingsObjectJava::to_json ( void )
    {
        lock.lock(  );
        JSON settingsObject;
        settingsObject[ "useWearable" ] = useWearable;
        settingsObject[ "sendIFTTT" ] = sendIFTTT;
        settingsObject[ "webhookKeyIFTTT" ] = webhookKeyIFTTT;
        settingsObject[ "sendOSC" ] = sendOSC;
        settingsObject[ "oscHost" ] = oscHost;
        settingsObject[ "oscPort" ] = oscPort;
        settingsObject[ "startF" ] = startF;
        settingsObject[ "startFT" ] = startFT;
        settingsObject[ "stopF" ] = stopF;
        settingsObject[ "stopFT" ] = stopFT;
        settingsObject[ "costThreshold" ] = costThreshold;
        lock.unlock(  );
        return settingsObject;
    }



    // ArrayListAdapterJava
    // ---------------------------------------------------------------------------------------------
    jobject ArrayListAdapterJava::createInterface ( JNIEnv *env )
    {
        return reference = env->NewGlobalRef( env->NewObject( JNIOBJECTS.ArrayListAdapter_class, JNIOBJECTS.ArrayListAdapter_constructor, 0 ) );
    }

    void ArrayListAdapterJava::setInstance ( jobject object )
    {
        reference = object;
    }

    jobject ArrayListAdapterJava::getAt ( JNIEnv* env, jobject inArray, int32_t index, jobjectRefType refType )
    {

        return createReference( env, env->CallObjectMethod( inArray, JNIOBJECTS.ArrayListAdapter_get, jint( index ) ),
                                         refType );
    }


    jobject ArrayListAdapterJava::getAtNoContext ( jobject inArray, int32_t index, jobjectRefType refType )
    {
        jobject retObject = nullptr;
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            retObject = getAt( g_env, inArray, index, refType );
            return true;
        } );
        return retObject;
    }

    jboolean ArrayListAdapterJava::addNoContext ( jobject inArray, jobject object )
    {
        jboolean success = jboolean( false );
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            success = g_env->CallBooleanMethod( inArray, JNIOBJECTS.ArrayListAdapter_add, object );
            return true;
        } );
        return success;
    }

    jobject ArrayListAdapterJava::removeAtNoContext ( jobject inArray, int32_t index )
    {
        jobject retObject = nullptr;
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            retObject = g_env->CallObjectMethod( inArray, JNIOBJECTS.ArrayListAdapter_removeAt, jint( index ) );
            return true;
        } );
        return retObject;
    }

    void ArrayListAdapterJava::clear( JNIEnv* env, jobject inArray )
    {
        env->CallVoidMethod( inArray, JNIOBJECTS.ArrayListAdapter_clear );
    }

    void ArrayListAdapterJava::clearNoContext( jobject inArray )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            g_env->CallVoidMethod( inArray, JNIOBJECTS.ArrayListAdapter_clear );
            return true;
        });
    }

    void ArrayListAdapterJava::notifyChangedNoContext ( jobject inArray, int32_t index )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            g_env->CallVoidMethod( inArray, JNIOBJECTS.ArrayListAdapter_updateAt, jint( index ) );
            return true;
        });
    }
    // ---------------------------------------------------------------------------------------------


    // GestureJava
    // ---------------------------------------------------------------------------------------------
    jobject GestureJava::createJavaGesture ( JNIEnv *env, Gesture& g )
    {
        jboolean active = jboolean( g.active ); // JBOOL
        jstring nameOfGesture = env->NewStringUTF( g.nameOfGesture.c_str(  ) ); // JSTRING
        jint numberOfRecordings = static_cast< int32_t >( g.t.size(  ) ); // JINT
        jobject result = env->NewObject( JNIOBJECTS.GestureJava_class, JNIOBJECTS.GestureJava_constructor, g.id, active, nameOfGesture, numberOfRecordings );
        TrainingSeriesObjectJava::addTrainingSeriesToGestureJava( env, result, g.t );
        env->DeleteLocalRef( nameOfGesture );
        // Return java equiv.
        return result;
    }

    jobject GestureJava::createJavaGestureNoContext ( Gesture& g )
    {
        jobject result = nullptr;
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            result = createJavaGesture( g_env, g );
            return true;
        } );

        // Return java equiv.
        return result;
    }

    void GestureJava::setGestureNameNoContext ( jobject refToElement, std::string& nameOfGesture )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jstring nGest = g_env->NewStringUTF( nameOfGesture.c_str(  ) ); // JSTRING
            g_env->CallVoidMethod( refToElement, JNIOBJECTS.GestureJava_nameOfGesture, nGest );
            deleteReference( g_env, refToElement );
            g_env->DeleteLocalRef( nGest );
            return true;
        } );
    }

    void GestureJava::setGestureActive ( JNIEnv* env, jobject refToElement, bool active )
    {
        env->CallVoidMethod( refToElement, JNIOBJECTS.GestureJava_active, jboolean( active ) );
    }

    void GestureJava::setGestureActiveNoContext ( jobject refToElement, bool active )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            setGestureActive( g_env, refToElement, active );
            deleteReference( g_env, refToElement );
            return true;
        } );
    }

    void GestureJava::setGestureNumberOfExamples ( JNIEnv* env, jobject refToElement, int32_t numberOfExamples )
    {
        env->CallVoidMethod( refToElement, JNIOBJECTS.GestureJava_numberOfRecordings, numberOfExamples );
    }

    void GestureJava::setGestureNumberOfExamplesNoContext ( jobject refToElement, int32_t numberOfExamples )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            setGestureNumberOfExamples( g_env, refToElement, numberOfExamples );
            deleteReference( g_env, refToElement );
            return true;
        } );
    }


//    jobject GestureJava::getTrainingSeriesObjectsField( jobject inputGesture )
//    {
//        jobject retObject = nullptr;
//        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
//            retObject = g_env->GetObjectField( inputGesture, JNIOBJECTS.GestureJava_trainingSeriesObjects );
//            return true;
//        } );
//        return retObject;
//    }
    // ---------------------------------------------------------------------------------------------

    // TrainingSeriesObjectJava
    // ---------------------------------------------------------------------------------------------
    void TrainingSeriesObjectJava::addSingleSeriesToGestureJavaNoContext ( jobject gestureJava, trainingSeriesObject& t, int32_t newNumExamples )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jobject tempFieldObject = g_env->GetObjectField( gestureJava, JNIOBJECTS.GestureJava_trainingSeriesObjects );
            jobject trainingSeriesJava = TrainingSeriesObjectJava::createTrainingSeriesObject( g_env, t );
            g_env->CallBooleanMethod( tempFieldObject, JNIOBJECTS.ArrayListAdapter_add, trainingSeriesJava );
            GestureJava::setGestureNumberOfExamples( g_env, gestureJava, newNumExamples );
            g_env->DeleteLocalRef( tempFieldObject );
            g_env->DeleteLocalRef( trainingSeriesJava );
            return true;
        } );
    }

    void TrainingSeriesObjectJava::removeSeriesNoContext( jobject inArray, int32_t indexOfGesture, int32_t indexOfTrainingGesture, int32_t newNumExamples  )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jobject gestureJava = ArrayListAdapterJava::getAt( g_env, inArray, indexOfGesture, jobjectRefType::JNILocalRefType );
            jobject tempFieldObject = g_env->GetObjectField( gestureJava, JNIOBJECTS.GestureJava_trainingSeriesObjects );
            g_env->CallObjectMethod( tempFieldObject, JNIOBJECTS.ArrayListAdapter_removeAt, indexOfTrainingGesture );
            GestureJava::setGestureNumberOfExamples( g_env, gestureJava, newNumExamples );
            if ( newNumExamples == 0 )
                GestureJava::setGestureActive( g_env, gestureJava, false );
            g_env->CallVoidMethod( inArray, JNIOBJECTS.ArrayListAdapter_updateAt, indexOfGesture );
            g_env->DeleteLocalRef( tempFieldObject );
            g_env->DeleteLocalRef( gestureJava );
            return true;
        } );
    }

    void TrainingSeriesObjectJava::removeSeriesFromGestureNoContext( jobject gestureJava, int32_t index )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jobject tempFieldObject = g_env->GetObjectField( gestureJava, JNIOBJECTS.GestureJava_trainingSeriesObjects );
            g_env->CallObjectMethod( tempFieldObject, JNIOBJECTS.ArrayListAdapter_removeAt, index );
            g_env->DeleteLocalRef( tempFieldObject );
            deleteReference( g_env, gestureJava );
            return true;
        } );
    }

    jobject TrainingSeriesObjectJava::createTrainingSeriesObject ( JNIEnv *env, trainingSeriesObject& t )
    {
        jint timeLength = static_cast< int32_t >( t.lengthInTime ); // JINT
        jstring timeStamp = env->NewStringUTF( t.dateAndTime.c_str(  ) ); // JSTRING
        jobject result = env->NewObject( JNIOBJECTS.TrainingSeriesObjectJava_class, JNIOBJECTS.TrainingSeriesObjectJava_constructor, timeLength, timeStamp, t.numberOfTimesRecognized );
        env->DeleteLocalRef( timeStamp );
        // Return java equiv.
        return result;
    }
    void TrainingSeriesObjectJava::addTrainingSeriesToGestureJava( JNIEnv *env, jobject gestureJava, std::vector< trainingSeriesObject >& tSeries )
    {
        jobject array = env->GetObjectField( gestureJava, JNIOBJECTS.GestureJava_trainingSeriesObjects );
        for ( trainingSeriesObject& t: tSeries )
        {
            jobject tObject = TrainingSeriesObjectJava::createTrainingSeriesObject( env, t );
            env->CallBooleanMethod( array, JNIOBJECTS.ArrayListAdapter_add, tObject );
            env->DeleteLocalRef( tObject ); // TODO: does this need to happen?
        }
        env->DeleteLocalRef( array );
    }

    jobject TrainingSeriesObjectJava::getTrainingSeriesAsArrayList ( JNIEnv *env, std::vector< trainingSeriesObject >& t )
    {
        jobject result = env->NewObject( JNIOBJECTS.ArrayListAdapter_class, JNIOBJECTS.ArrayListAdapter_constructor, t.size(  ) );
        for ( trainingSeriesObject& o: t )
        {
            jobject tObject = TrainingSeriesObjectJava::createTrainingSeriesObject( env, o );
            env->CallBooleanMethod( result, JNIOBJECTS.ArrayListAdapter_add, tObject );
            env->DeleteLocalRef( tObject ); // TODO: does this need to happen?
        }
        return result;
    }

    void TrainingSeriesObjectJava::setNumberOfTimesRecognized( JNIEnv *env, jobject gestureJavaRecognized, int32_t indexOfTrainingObject, int32_t numberOfTimesRecognized )
    {
        jobject tempFieldObject = env->GetObjectField( gestureJavaRecognized, JNIOBJECTS.GestureJava_trainingSeriesObjects );
        jobject trainingDataObjectJava = INTERFACE::ArrayListAdapterJava::getAt( env, tempFieldObject, indexOfTrainingObject, jobjectRefType::JNILocalRefType );
        env->CallVoidMethod( trainingDataObjectJava, JNIOBJECTS.TrainingSeriesObjectJava_numTimesRecognized, numberOfTimesRecognized );
        env->DeleteLocalRef( tempFieldObject );
        env->DeleteLocalRef( trainingDataObjectJava );
    }

    void TrainingSeriesObjectJava::setNumberOfTimesRecognizedNoContext( jobject gestureJavaRecognized, int32_t indexOfTrainingObject, int32_t numberOfTimesRecognized )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            setNumberOfTimesRecognized( g_env, gestureJavaRecognized, indexOfTrainingObject, numberOfTimesRecognized );
            return true;
        } );
    }

    void TrainingSeriesObjectJava::clearTrainingDataNoContext ( jobject inArray, int32_t indexOfGesture )
    {
        runOutsideOfContext( [ & ]( JNIEnv *g_env ) {
            jobject gestureJava = ArrayListAdapterJava::getAt( g_env, inArray, indexOfGesture, jobjectRefType::JNILocalRefType );
            jobject tempFieldObject = g_env->GetObjectField( gestureJava, JNIOBJECTS.GestureJava_trainingSeriesObjects );
            g_env->CallVoidMethod( tempFieldObject, JNIOBJECTS.ArrayListAdapter_clear );
            GestureJava::setGestureNumberOfExamples( g_env, gestureJava, 0 );
            GestureJava::setGestureActive( g_env, gestureJava, false );

            g_env->DeleteLocalRef( gestureJava );
            g_env->DeleteLocalRef( tempFieldObject );
            return true;
        } );
    }

    // ---------------------------------------------------------------------------------------------
}