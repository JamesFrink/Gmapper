//
// Created by James on 27/01/2018.
//

#include <android/log.h>
#include "SaveStateManager.hpp"
#include "InterfaceJNI.hpp"

bool doesFileExist( const std::string& nameOfFile )
{
    std::ifstream infile( nameOfFile.c_str(  ) );
    return infile.good(  );
}

bool doesDirectoryExist( const std::string& nameOfDirectory )
{
    const char* pathName = nameOfDirectory.c_str(  );
    struct stat info;

    if( stat( pathName, &info ) != 0 )
        return false;
    else return ( info.st_mode & S_IFDIR ) != 0;
}

bool createDirectory ( const std::string& pathOfDirectory )
{
    int errNo = 0;
    if ( (errNo = mkdir( pathOfDirectory.c_str(  ), 0770 ) ) != 0 )
    {
        __android_log_print( ANDROID_LOG_WARN, "MKDIR", "FAILED %d", errNo );
        return false;
    }
    return true;
}

bool deleteFile( const std::string& nameOfFile )
{
    if( remove( nameOfFile.c_str(  ) ) != 0 )
        return true;
    else
        return true;
}

SaveStateManager::SaveStateManager ( void )//  : ThreadedProcess (  )
{

};

void SaveStateManager::setup( const std::string nameOfMainFolder, const std::string nameOfIndexFile )
{
    this->nameOfMainFolder = nameOfMainFolder;
    this->nameOfIndexFile = nameOfIndexFile;

    if ( doesDirectoryExist( nameOfMainFolder ) && doesFileExist( nameOfMainFolder + "/" + nameOfIndexFile ) )
    {   // Load all ( potentially ) stored data
        std::ifstream in( nameOfMainFolder + "/" + nameOfIndexFile );
        in >> indexFile;
        in.close(  );
        // TODO: remember to get gestureidcounter from this file
        // Load all files that are listed
        std::vector< std::string > gf = indexFile[ "gestureFiles" ];
        for ( std::string& fileName : gf )
        {
            std::ifstream g ( nameOfMainFolder + "/" + gestures + "/" + fileName );
            JSON tempGesture;
            g >> tempGesture;
            gestureFiles.push_back( tempGesture );
        }

    } else {
        // Create folder and index file
        __android_log_print( ANDROID_LOG_DEBUG, "test", "%s", nameOfMainFolder.c_str(  ) );

        if ( !createDirectory( nameOfMainFolder ) )
            __android_log_print( ANDROID_LOG_WARN, "CREATEMAINFOLDER", "FAILED" );

        indexFile[ "gestureIdCounter" ] = 0;
        indexFile[ "gestureFiles" ] = std::vector< std::string >(  );
        indexFile[ "eventFiles" ] = std::vector< std::string >(  );
        indexFile[ "modelValues" ] = "placeHolder";

        INTERFACE::SettingsObjectJava tSettings;
        indexFile[ "settings" ] = tSettings.to_json(  );

        std::ofstream output_file( nameOfMainFolder + "/" + nameOfIndexFile );
        output_file << std::setw(4) << indexFile << std::endl;
        output_file.close();

        if ( !createDirectory( nameOfMainFolder + "/" + gestures ) )
            __android_log_print( ANDROID_LOG_WARN, "CREATEGESTUREFOLDER", "FAILED" );

        if ( !createDirectory( nameOfMainFolder + "/" + events ) )
            __android_log_print( ANDROID_LOG_WARN, "CREATEEVENTFOLDER", "FAILED" );

        __android_log_print( ANDROID_LOG_DEBUG, "INIT", "CREATED FOLDERS AND INDEX FILE FOR DATA STORAGE" );
    }

    //startThread( 1000 ); // 1000ms sleepTime ( could be longer )
}

void SaveStateManager::writeIndexFile ( void )
{
    std::ofstream output_file( nameOfMainFolder + "/" + nameOfIndexFile );
    output_file << std::setw(4) << indexFile << std::endl;
    output_file.close();
}

void SaveStateManager::writeGestureFile ( JSON gesture )
{
    std::ofstream output_file( nameOfMainFolder + "/" + gestures + "/" + std::to_string( gesture[ "id" ] ) );
    output_file << std::setw(4) << gesture << std::endl;
    output_file.close();
}

void SaveStateManager::writeGestureFile ( int32_t index )
{
    JSON gesture = gestureFiles[ index ];
    std::ofstream output_file( nameOfMainFolder + "/" + gestures + "/" + std::to_string( gesture[ "id" ] ) );
    output_file << std::setw(4) << gesture << std::endl;
    output_file.close();
}

void SaveStateManager::addNewGesture ( JSON gestureJSON )
{
    gestureFiles.push_back( gestureJSON );
    indexFile[ "gestureFiles" ].push_back( std::to_string( gestureJSON[ "id" ] ) );
    writeGestureFile( gestureJSON );
    writeIndexFile(  );
}

void SaveStateManager::removeGesture( int32_t index )
{
    gestureFiles.erase( gestureFiles.begin(  ) + index );
    std::string name = indexFile[ "gestureFiles" ][ index ];
    indexFile[ "gestureFiles" ].erase( indexFile[ "gestureFiles" ].begin(  ) + index );
    if ( !deleteFile( nameOfMainFolder + "/" + gestures + "/" + name ) )
        __android_log_print( ANDROID_LOG_WARN, "Delete", "Could not delete file" );
    writeIndexFile(  );
}