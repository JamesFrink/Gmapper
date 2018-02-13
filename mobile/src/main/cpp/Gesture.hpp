//
// Created by James on 19/01/2018.
//

#ifndef GMAPPER_GESTURE_HPP
#define GMAPPER_GESTURE_HPP

#include "RapidLib/src/seriesClassification.h" // trainingSeries
#include "json.hpp"
using JSON = nlohmann::json;

struct trainingSeriesObject {
    int32_t lengthInTime;
    int32_t numberOfTimesRecognized;
    std::string dateAndTime;
    trainingSeries series;

    trainingSeriesObject ( void )
    {

    }

    trainingSeriesObject ( int32_t lengthInTime, int32_t numberOfTimesRecognized, std::string dateAndTime, trainingSeries series )
    {
        this->lengthInTime = lengthInTime;
        this->numberOfTimesRecognized = numberOfTimesRecognized;
        this->dateAndTime = dateAndTime;
        this->series = series;
    }

    JSON to_json( void )
    {
        JSON tSeriesJSON;
        tSeriesJSON[ "lengthInTime" ] = lengthInTime;
        tSeriesJSON[ "dateAndTime" ] = dateAndTime;
        tSeriesJSON[ "numberOfTimesRecognized" ] = numberOfTimesRecognized;
        std::vector< JSON > tVectSeries;
        for ( std::vector< double >& subSeries : series.input )
        {
            JSON subSeriesJSON = subSeries;
            tVectSeries.push_back( subSeriesJSON );
        }
        tSeriesJSON[ "series" ] = tVectSeries;
        return tSeriesJSON;
    }

    void from_json( JSON tSeriesJSON, std::string indexString )
    {
        lengthInTime = tSeriesJSON[ "lengthInTime" ];
        dateAndTime = tSeriesJSON[ "dateAndTime" ];
        numberOfTimesRecognized = tSeriesJSON[ "numberOfTimesRecognized" ];
        for ( JSON& subSeriesJSON : tSeriesJSON[ "series" ] )
        {
            std::vector< double > subSeries = subSeriesJSON;
            series.input.push_back( subSeries );
        }
        series.label = indexString;
    }
};

struct Gesture {
    int32_t id  = -1;
    bool active = false;
    std::string nameOfGesture;
    std::vector< trainingSeriesObject > t;

    Gesture( void )
    {
    }

    Gesture( int32_t id, bool active, std::string nameOfGesture, std::vector< trainingSeriesObject > t )
    {
        this->id = id;
        this->active = active;
        this->nameOfGesture = nameOfGesture;
        this->t = t;
    }

    void setIndexLabels ( int32_t index )
    {
        std::string indexStr = std::to_string( index );
        int32_t trainingExampleIndex = 0;
        for ( trainingSeriesObject& tObject : t )
            tObject.series.label = indexStr + "_" + std::to_string( trainingExampleIndex++ );
    }

    JSON to_json( void )
    {
        JSON gestureJSON;
        gestureJSON[ "id" ] = id;
        gestureJSON[ "active" ] = active;
        gestureJSON[ "nameOfGesture" ] = nameOfGesture;
        std::vector< JSON > jsonifiedTseries;

        for ( trainingSeriesObject& o : t )
            jsonifiedTseries.push_back( o.to_json(  ) );

        gestureJSON[ "t" ] = jsonifiedTseries;
        return gestureJSON;
    }

    void from_json( JSON gestureJSON, int32_t indexOfGesture )
    {
        id = gestureJSON[ "id" ];
        active = gestureJSON[ "active" ];
        nameOfGesture = gestureJSON[ "nameOfGesture" ];
        int32_t indexOfTrainingExample = 0;
        for ( JSON& json : gestureJSON[ "t" ] )
        {
            trainingSeriesObject tObj;
            tObj.from_json( json, std::to_string( indexOfGesture ) + "_" + std::to_string( indexOfTrainingExample++ ) );
            t.push_back( tObj );
        }
    }
};


#endif //GMAPPER_GESTURE_HPP
