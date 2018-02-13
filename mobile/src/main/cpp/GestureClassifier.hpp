//
// Created by James on 18/01/2018.
//

#ifndef GMAPPER_GESTURECLASSIFIER_HPP
#define GMAPPER_GESTURECLASSIFIER_HPP

#include "ThreadedProcess.h" // Atomic etc
#include <functional>
#include <math.h>
#include <android/log.h>
#include <algorithm>

#include "RapidLib/src/seriesClassification.h" // DTW
#include "Gesture.hpp"
#include "RingBufferAny.hpp" // Inter-thread communication
#include "Spinlock.hpp"
#include "Timer.h"
#include "InterfaceJNI.hpp"
#include "SaveStateManager.hpp"
#include "IFTTTThreaded.hpp"
#include "SimpleOSCThreaded.hpp"

static inline std::string getCurrentDateAndTime()
{ // Returns current date and time formatted as a string
    auto t = std::time(nullptr);
    std::ostringstream oss;
    oss << std::asctime(localtime(&t));
    std::string out = oss.str();
    return out;
}

class GestureClassifier : public ThreadedProcess {
public:
    GestureClassifier ( void );
    ~GestureClassifier( void );

    void initializeInterface( INTERFACE::ArrayListAdapterJava* sharedMemory, INTERFACE::SettingsObjectJava* sharedSettings, std::string& directory );
    void loadSettings( void );

    void newAccelData ( float x, float y, float z );
    void addNewGesture ( std::string nameOfGesture );
    void setNameOfGesture ( int32_t id, std::string nameofGesture );
    void removeGesture ( int32_t id );
    void setGestureActive ( int32_t id, bool active );
    void clearAllExampleData ( int32_t id );
    void removeTrainingSeriesObject ( int32_t id, int32_t index );
    void waitForExampleData ( int32_t id );
    void setClassificationState ( bool input );

    void synchronizeSharedList( INTERFACE::ArrayListAdapterJava* sharedMemory );

    int32_t getNumGestures ( void );
    Gesture getGesture ( int32_t id );
    Gesture getGestureByIndex ( int32_t index );

    std::function< bool ( int32_t index ) > classifiedGestureCallback = nullptr;
    std::function< bool ( int32_t newState ) > setCurrentClassificationState = nullptr;

protected:
    struct AccelerometerData
    {
        float x, y, z;
        float length( void ) const
        {   // Get the length of the vector
            return sqrtf( x * x + y * y + z * z );
        }
    };

    bool train ( void );
    void mainThreadCallback ( void );
    void processData ( AccelerometerData& data ); // Runs in thread

private:
    int32_t gestureIdCounter = 0;
    SaveStateManager saveState;
    INTERFACE::ArrayListAdapterJava* sharedMemory = nullptr;
    INTERFACE::SettingsObjectJava* sharedSettings = nullptr;

    IFTTTThreaded ifttt;
    SimpleOSCThreaded simpleOSC;

    bool recognitionRunning = false;
    bool runningGestureClassifier = false; // TODO
    bool collectTrainingData = false;

    // -- Training data things
    int32_t collectTrainingDataFor; // Have this be a pointer to an objeet again
    // ---

    bool trained = false;
    seriesClassification classifier;

    Spinlock lock;
    std::vector< Gesture > gestureList;
    trainingSeries tempSeries;
    Timer segmentationTimer, recordingLengthTimer;

    std::vector< std::vector< double > > segment;
    // --

    enum STATES {
        BELOW_THRESHOLD,
        BELOW_THRESHOLD_BUT_WAITING_FOR_DATA,
        RECOGNIZING_GESTURE,
        COLLECTING_TRAINING_DATA,
        RECOGNIZED_GESTURE,
        DONE_COLLECTING_TRAINING_DATA
    } state = BELOW_THRESHOLD;

    // Could be RingBufferV< AccelerometerData > if other commands will remain blocking
    RingBufferAny toProcessThread;
    static constexpr float GravityNormalizer = 1.0f / 9.807f;
};


#endif //GMAPPER_GESTURECLASSIFIER_HPP
