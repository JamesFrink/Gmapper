//
// Created by James on 18/01/2018.
//

#include "GestureClassifier.hpp"

//TODO: Make lock free

GestureClassifier::GestureClassifier( void )
{
    toProcessThread.setup( sizeof( AccelerometerData ) * 2000 );
    startThread( 6000 ); // Sleep 1/60th of a second each call
    ifttt.setup(  );
    simpleOSC.setup( 4096, 500000, "/Gmapper/gesture" );
    __android_log_print( ANDROID_LOG_DEBUG, "From GestureClassifier: ", "Started Thread");
}

GestureClassifier::~GestureClassifier( void )
{
    __android_log_print( ANDROID_LOG_DEBUG, "From GestureClassifier: ", "Stopped Thread");
}

void GestureClassifier::initializeInterface( INTERFACE::ArrayListAdapterJava* sharedMemory, INTERFACE::SettingsObjectJava* sharedSettings, std::string& directory )
{
    lock.lock(  );
    this->sharedMemory = sharedMemory;
    this->sharedSettings = sharedSettings;
    saveState.setup( directory + "/Gmapper", "index.json" );
    int32_t index = 0;
    for ( JSON& gestureJSON : saveState.gestureFiles )
    {
        Gesture toAdd;
        toAdd.from_json( gestureJSON, index++ );
        gestureList.push_back( toAdd );
        sharedMemory->addNoContext( sharedMemory->reference, INTERFACE::GestureJava::createJavaGestureNoContext( toAdd ) );
    }
    gestureIdCounter = saveState.indexFile[ "gestureIdCounter" ];
    sharedSettings->from_json( saveState.indexFile[ "settings" ] );
    simpleOSC.setHostAndPort( sharedSettings->oscHost, sharedSettings->oscPort );
    trained = train(  ); // Calling this at startup because saving all the values would be 2x the work for each training session
    lock.unlock(  );

    sharedSettings->wearableChangedCallback = [ & ]( bool useWearable )
    {
        saveState.lockIndexFile.lock(  );
        saveState.indexFile[ "settings" ][ "useWearable" ] = useWearable;
        saveState.writeIndexFile(  );
        saveState.lockIndexFile.unlock(  );
    };

    sharedSettings->IFTTTChangedCallback = [ & ]( bool sendIFTTT, std::string webhookKeyIFTTT )
    {
        saveState.lockIndexFile.lock(  );
        saveState.indexFile[ "settings" ][ "sendIFTTT" ] = sendIFTTT;
        saveState.indexFile[ "settings" ][ "webhookKeyIFTTT" ] = webhookKeyIFTTT;
        saveState.writeIndexFile(  );
        saveState.lockIndexFile.unlock(  );
    };

    sharedSettings->OSCChangedCallback = [ & ]( bool sendOSC, std::string oscHost, int32_t oscPort )
    {
        saveState.lockIndexFile.lock(  );
        saveState.indexFile[ "settings" ][ "sendOSC" ] = sendOSC;
        saveState.indexFile[ "settings" ][ "oscHost" ] = oscHost;
        saveState.indexFile[ "settings" ][ "oscPort" ] = oscPort;
        saveState.writeIndexFile(  );
        saveState.lockIndexFile.unlock(  );
        simpleOSC.setHostAndPort( oscHost, oscPort );
    };

    sharedSettings->segmentationChangedCallback = [ & ]( float startF, int32_t startFT, float stopF, int32_t stopFT, float costThreshold )
    {
        saveState.lockIndexFile.lock(  );
        saveState.indexFile[ "settings" ][ "startF" ] = startF;
        saveState.indexFile[ "settings" ][ "startFT" ] = startFT;
        saveState.indexFile[ "settings" ][ "stopF" ] = stopF;
        saveState.indexFile[ "settings" ][ "stopFT" ] = stopFT;
        saveState.indexFile[ "settings" ][ "costThreshold" ] = costThreshold;
        saveState.writeIndexFile(  );
        saveState.lockIndexFile.unlock(  );
    };
}

void GestureClassifier::loadSettings( void )
{
    sharedSettings->syncSettingsToJava(  );
}

void GestureClassifier::synchronizeSharedList ( INTERFACE::ArrayListAdapterJava* sharedMemory )
{
    // Only have to update the view
    lock.lock(  );
    this->sharedMemory = sharedMemory;
    for ( Gesture& g : gestureList )
        sharedMemory->addNoContext( sharedMemory->reference, INTERFACE::GestureJava::createJavaGestureNoContext( g ) );
    lock.unlock(  );
}

void GestureClassifier::newAccelData ( float x, float y, float z )
{   // Call from Android to add new accelerometer data
    AccelerometerData data { x *= GravityNormalizer, y *= GravityNormalizer, z *= GravityNormalizer };
    toProcessThread.push( &data, 1 );
}

void GestureClassifier::addNewGesture ( std::string nameOfGesture )
{
    Gesture toAdd = Gesture{ gestureIdCounter++, false, nameOfGesture, { } };
    lock.lock(  );
    gestureList.push_back( toAdd );
    lock.unlock(  );

    sharedMemory->addNoContext( sharedMemory->reference, INTERFACE::GestureJava::createJavaGestureNoContext( toAdd ) );

    saveState.lockIndexFile.lock(  );
    saveState.indexFile[ "gestureIdCounter" ] = gestureIdCounter;
    saveState.addNewGesture( toAdd.to_json(  ) ); // Writes index file anyway
    saveState.lockIndexFile.unlock(  );
}

void GestureClassifier::setNameOfGesture ( int32_t index, std::string nameOfGesture )
{
    lock.lock(  );
    gestureList[ index ].nameOfGesture = nameOfGesture;
    lock.unlock(  );

    INTERFACE::GestureJava::setGestureNameNoContext (
            INTERFACE::ArrayListAdapterJava::getAtNoContext ( sharedMemory->reference, index, jobjectRefType::JNILocalRefType ), nameOfGesture );
    INTERFACE::ArrayListAdapterJava::notifyChangedNoContext ( sharedMemory->reference, index );

    saveState.gestureFiles[ index ][ "nameOfGesture" ] = nameOfGesture;
    saveState.writeGestureFile( index );
}

void GestureClassifier::removeGesture ( int32_t index )
{
    lock.lock(  );
    collectTrainingData = false;
    gestureList.erase( gestureList.begin(  ) + index );

    // Update trainingData labels
    for ( int32_t indexOfGesture = index; indexOfGesture < gestureList.size(  ); ++indexOfGesture )
        gestureList[ indexOfGesture ].setIndexLabels( indexOfGesture );

    trained = train(  );
    lock.unlock(  );

    sharedMemory->removeAtNoContext ( sharedMemory->reference, index );
    saveState.removeGesture( index );
}

void GestureClassifier::setGestureActive ( int32_t index, bool active )
{
    lock.lock(  );
    gestureList[ index ].active = active;
    trained = train(  );
    lock.unlock(  );

    INTERFACE::GestureJava::setGestureActiveNoContext (
            INTERFACE::ArrayListAdapterJava::getAtNoContext ( sharedMemory->reference, index, jobjectRefType::JNILocalRefType ), active );
    INTERFACE::ArrayListAdapterJava::notifyChangedNoContext ( sharedMemory->reference, index );
    saveState.gestureFiles[ index ][ "active" ] = active;
    saveState.writeGestureFile( index );
}

void GestureClassifier::clearAllExampleData ( int32_t index )
{
    lock.lock(  );
    Gesture& g = gestureList[ index ];
    g.active = false;
    g.t.clear(  );
    trained = train(  );
    lock.unlock(  );

    INTERFACE::TrainingSeriesObjectJava::clearTrainingDataNoContext( sharedMemory->reference, index );
    saveState.gestureFiles[ index ][ "t" ].clear(  );
    saveState.writeGestureFile( index );
}

void GestureClassifier::removeTrainingSeriesObject( int32_t indexOfGesture, int32_t indexOfTrainingData )
{
    lock.lock(  );
    Gesture& g = gestureList[ indexOfGesture ];
    g.t.erase( g.t.begin(  ) + indexOfTrainingData );
    g.setIndexLabels( indexOfGesture );

    int32_t numExamples( static_cast< int32_t >( g.t.size(  ) ) );
    if ( numExamples == 0 )
        g.active = false;

    trained = train(  );

    lock.unlock(  );

    INTERFACE::TrainingSeriesObjectJava::removeSeriesNoContext( sharedMemory->reference,
                                                                indexOfGesture,
                                                                indexOfTrainingData,
                                                                numExamples );
    saveState.gestureFiles[ indexOfGesture ][ "t" ].erase( saveState.gestureFiles[ indexOfGesture ][ "t" ].begin(  ) + indexOfTrainingData );
    saveState.writeGestureFile( indexOfGesture );
}

void GestureClassifier::waitForExampleData( int32_t index )
{
    lock.lock(  );
    segmentationTimer.reset(  );
    if ( !collectTrainingData && index != -1 )
    {
        collectTrainingDataFor = index;
        collectTrainingData = true;
        state = BELOW_THRESHOLD_BUT_WAITING_FOR_DATA;
    } else {
        collectTrainingData = false;
        state = BELOW_THRESHOLD;
    }
    if ( setCurrentClassificationState != nullptr )
        if ( !setCurrentClassificationState( static_cast< int32_t >( state ) ) )
            __android_log_print( ANDROID_LOG_WARN, "GestureClassifier: ", "JVM Callback Failed" );

    tempSeries.input.clear(  );
    lock.unlock(  );
}

void GestureClassifier::setClassificationState ( bool input )
{
    lock.lock(  );
    runningGestureClassifier = input;
    lock.unlock(  );
}

int32_t GestureClassifier::getNumGestures ( void )
{
    lock.lock(  );
    int32_t retSize = static_cast< int32_t >( gestureList.size(  ) );
    lock.unlock(  );
    return retSize;
}

Gesture GestureClassifier::getGesture ( int32_t id )
{
    Gesture returnGesture{ -1, false, "NULL", {  } };
    lock.lock(  );
    for (  Gesture& g : gestureList )
    {
        if ( g.id == id )
        {
            returnGesture = g;
            break;
        }
    }
    lock.unlock(  );
    return returnGesture; // TODO: remove NULL after testing
}

Gesture GestureClassifier::getGestureByIndex ( int32_t index )
{
    Gesture returnGesture{ -1, false, "NULL", {  } };
    lock.lock(  );
    if ( index >= 0 && index < gestureList.size(  ) )
    {
        returnGesture = gestureList[index];
    }
    lock.unlock(  );
    return returnGesture;
}

bool GestureClassifier::train ( void )
{   // Has to be called within a locked context ( else allow non blocking )
    std::vector< trainingSeries > toTrainOn;
    for ( Gesture& g : gestureList ) {
        if ( g.active ) {
            for ( trainingSeriesObject& example : g.t )
                if ( example.series.input.size(  ) > 0 )
                    toTrainOn.push_back( example.series );
        }
    }

    if ( toTrainOn.size(  ) > 0 )
        return classifier.train( toTrainOn );
    else
        return false;
}

void GestureClassifier::mainThreadCallback ( void )
{
    RingBufferAny::VariableHeader outputHeader;
    lock.lock(  );
    while ( toProcessThread.anyAvailableForPop( outputHeader ) )
    {
        if ( outputHeader.type_index == typeid( AccelerometerData ) )
        {
            AccelerometerData data[ outputHeader.valuesPassed ];
            toProcessThread.pop( data, outputHeader.valuesPassed );
            for ( uint32_t i = 0; i < outputHeader.valuesPassed; ++i )
            {
                processData( data[ i ] ); // Code below for easier reading
            }
        }
    }
    lock.unlock(  );
}

void GestureClassifier::processData( AccelerometerData &data )
{ // Runs in thread and is where all the machine learning happens
    float currentForce = data.length(  );
    STATES oldState = state;

    sharedSettings->lock.lock(  );

    if ( recognitionRunning )
    {
        if ( collectTrainingData )
        {
            // Collect training data
            state = COLLECTING_TRAINING_DATA;
            tempSeries.input.push_back( { double( data.x ), double( data.y ), double( data.z ) } );
        } else {
            // Process data and check for gesture
            state = RECOGNIZING_GESTURE;
            segment.push_back( { double( data.x ), double( data.y ), double( data.z ) } );
            /* This would send guesses before gesture was done being analyzed
            if ( trained )
                classLabel = "Class Found: " + classifier.run( segment );
             */
        }
        // Check if recognition should stop
        if ( currentForce < sharedSettings->stopF )
        {
            // Wait until threshold of time has also been surpassed
            if ( segmentationTimer.getTimeElapsed(  ) >= sharedSettings->stopFT )
            {
                // TrainingData state
                if ( collectTrainingData )
                {
                    state = DONE_COLLECTING_TRAINING_DATA;
                    collectTrainingData = false;
                    // Add the trainingSet and train the model

                    __android_log_print(ANDROID_LOG_DEBUG, "AddExample", "data %d", collectTrainingDataFor );
                    Gesture& trainingGesture = gestureList[ collectTrainingDataFor ];
                    int32_t numExamples = static_cast< int32_t >( trainingGesture.t.size(  ) );
                    tempSeries.label = std::to_string( collectTrainingDataFor ) + "_" + std::to_string( numExamples );
                    __android_log_print(ANDROID_LOG_DEBUG, "AddExample", "numExamples %d", numExamples );

                    trainingSeriesObject seriesObject {
                            static_cast< int32_t >( recordingLengthTimer.getTimeElapsed(  ) ), 0, // ms
                            getCurrentDateAndTime(  ), tempSeries };

                    trainingGesture.t.push_back( seriesObject );

                    jobject  collectTrainingDataForJavaRef = INTERFACE::ArrayListAdapterJava::getAtNoContext( sharedMemory->reference, collectTrainingDataFor, jobjectRefType::JNIGlobalRefType );
                    INTERFACE::TrainingSeriesObjectJava::addSingleSeriesToGestureJavaNoContext( collectTrainingDataForJavaRef, seriesObject, ++numExamples );
                    INTERFACE::ArrayListAdapterJava::notifyChangedNoContext( sharedMemory->reference, collectTrainingDataFor );
                    INTERFACE::deleteReferenceNoContext( collectTrainingDataForJavaRef );

                    saveState.gestureFiles[ collectTrainingDataFor ][ "t" ].push_back( seriesObject.to_json(  ) );
                    saveState.writeGestureFile( collectTrainingDataFor );

                    if ( trainingGesture.active )
                        trained = train(  );
                    // Todo: notify changes ( done getting example data + training etc )

                } else {
                    state = RECOGNIZED_GESTURE;

                    if ( trained && !gestureList.empty(  ) )
                    {
                        // This is the important line
                        std::string labelIdIndex = classifier.run( segment );
                        // -----
                        int32_t indexOfGesture, indexOfTrainingObject;
                        std::string label;
                        std::string::size_type pos = labelIdIndex.find( '_' );
                        if( labelIdIndex.npos != pos ) {
                            indexOfGesture = std::stoi( labelIdIndex.substr( 0, pos ) );
                            // Get cost to gesture
                            double cost = classifier.getCosts(  )[ indexOfGesture ];
                            // ----
                            if ( cost < sharedSettings->costThreshold ) {
                                indexOfTrainingObject = std::stoi(labelIdIndex.substr(pos + 1));
                                Gesture &detected = gestureList[indexOfGesture];
                                int32_t timesRecognized = ++detected.t[indexOfTrainingObject].numberOfTimesRecognized;

                                jobject gestureJavaRecognized = INTERFACE::ArrayListAdapterJava::getAtNoContext(
                                        sharedMemory->reference, indexOfGesture,
                                        jobjectRefType::JNIGlobalRefType);
                                INTERFACE::TrainingSeriesObjectJava::setNumberOfTimesRecognizedNoContext(
                                        gestureJavaRecognized, indexOfTrainingObject,
                                        timesRecognized);
                                INTERFACE::deleteReferenceNoContext(gestureJavaRecognized);

                                label = detected.nameOfGesture +
                                        std::to_string(indexOfTrainingObject);

                                if (sharedSettings->sendIFTTT)
                                    ifttt.triggerEvent(detected.nameOfGesture.c_str(),
                                                       sharedSettings->webhookKeyIFTTT.c_str(),
                                                       std::to_string(indexOfGesture),
                                                       std::to_string(indexOfTrainingObject),
                                                       std::to_string(timesRecognized));

                                if (sharedSettings->sendOSC)
                                    simpleOSC.triggerEvent(detected.nameOfGesture);

                                if (classifiedGestureCallback != nullptr)
                                    if (!classifiedGestureCallback(indexOfGesture))
                                        __android_log_print(ANDROID_LOG_WARN, "GestureClassifier",
                                                            "Java Callback Failed");

                                saveState.gestureFiles[indexOfGesture]["t"][indexOfTrainingObject]["numberOfTimesRecognized"] = timesRecognized;
                                saveState.writeGestureFile(indexOfGesture);
                            }
                        }
                    }
                }

                recognitionRunning = false;
                segmentationTimer.reset(  );
            }
        } else {
            segmentationTimer.reset(  );
        }
    } else {
        // Check if recognition should start
        if ( currentForce > sharedSettings->startF )
        {
            // Wait until threshold of time has also been surpassed
            if ( segmentationTimer.getTimeElapsed(  ) >= sharedSettings->startFT )
            {
                recognitionRunning = true;
                segment.clear(  );
                segmentationTimer.reset(  );

                if ( collectTrainingData )
                    recordingLengthTimer.reset(  );
            }
        } else {
            segmentationTimer.reset(  );
            if ( collectTrainingData )
                state = BELOW_THRESHOLD_BUT_WAITING_FOR_DATA;
            else
                state = BELOW_THRESHOLD;
        }
    }

    sharedSettings->lock.unlock(  );

    if ( oldState != state )
    {   // Send transition state message to Java
        if ( setCurrentClassificationState != nullptr )
            if ( !setCurrentClassificationState( static_cast< int32_t >( state ) ) )
                __android_log_print( ANDROID_LOG_WARN, "GestureClassifier: ", "JVM Callback Failed" );
    }
}