//
// Created by James on 27/01/2018.
//

#ifndef GMAPPER_SAVESTATEMANAGER_HPP
#define GMAPPER_SAVESTATEMANAGER_HPP

#include <stdio.h>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

//#include "ThreadedProcess.h"
#include "Gesture.hpp"

#include "json.hpp"
#include "Spinlock.hpp"

using JSON = nlohmann::json;

bool doesFileExist ( const std::string& nameOfFile );
bool doesDirectoryExist ( const std::string& nameOfDirectory );
bool createDirectory ( const std::string& nameOfDirectory );
bool deleteFile ( const std::string& nameOfFile );
// TODO: Threaded write?

#ifdef __ANDROID__
#include <ostream>
#include <sstream>
namespace std
{ // Workaround for android
    template <typename T>
    std::string to_string(T Value)
    {
        std::ostringstream TempStream;
        TempStream << Value;
        return TempStream.str();
    }

    inline long double strtold(const char * str, char ** str_end)
    {
        return strtod(str, str_end);
    }
}
#endif

class SaveStateManager {
public:
    SaveStateManager( void );
    void setup(  const std::string nameOfMainFolder, const std::string nameOfIndexFile );
    std::vector< JSON > gestureFiles;
    JSON indexFile;

    void writeGestureFile ( JSON gesture );
    void writeGestureFile ( int32_t index );
    void writeIndexFile ( void );
    void addNewGesture ( JSON gestureJSON );
    void removeGesture ( int32_t index );
    // getModel
    // std::vector< Event > getAllEvents ( void ); // TODO: Implement
    Spinlock lockIndexFile;
protected:
    //void mainThreadCallback( void );

    std::string nameOfMainFolder;
    std::string nameOfIndexFile;

    const char* gestures = "Gestures";
    const char* events = "Events"; // Todo
};


#endif //GMAPPER_SAVESTATEMANAGER_HPP
