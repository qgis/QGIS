/***************************************************************************
                          qgsspatialquery.h
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: $ */
#ifndef SPATIALQUERY_H
#define SPATIALQUERY_H

#include <geos_c.h>

#include <qgsvectorlayer.h>
#include <qgsspatialindex.h>

#include "qgsmngprogressbar.h"
#include "qgsreaderfeatures.h"


/**
* \brief Enum with the topologic relations
* \enum Topologic Relations
*
*/
enum Topologic_Relation
{
    Intersects,
    Disjoint,
    Touches,
    Crosses,
    Within,
    Equals,
    Overlaps,
    Contains
};


/**
* \class QgsSpatialQuery
* \brief Spatial Query core
*
*/
class QgsSpatialQuery
{
public:
    /**
    * \brief Constructor for a Spatial query.
    * \param pb Pointer to the MngProgressBar object.
    */
    QgsSpatialQuery(MngProgressBar *pb);

    /**
    * \brief Destructor
    */
    ~QgsSpatialQuery();

    /**
    * \brief Sets if using selected features in Target layer
    * \param useSelected TRUE if use selected.
    */
    void setSelectedFeaturesTarget(bool useSelected);

    /**
    * \brief Sets if using selected features in Reference layer
    * \param useSelected TRUE if use selected.
    */
    void setSelectedFeaturesReference(bool useSelected);

    /**
    * \brief Execute the query
    * \param qsetIndexResult    Reference to QSet contains the result query
    * \param relation           Enum Topologic Relation
    * \param lyrTarget          Pointer to Target Layer
    * \param lyrReference       Pointer to Reference Layer
    */
    void runQuery( QSet<int> & qsetIndexResult, int relation, QgsVectorLayer* lyrTarget, QgsVectorLayer* lyrReference );

    /**
    * \brief Gets the possible topologic relations
    * \param lyrTarget          Pointer to Target Layer
    * \param lyrReference       Pointer to Reference Layer
    * \returns QMap<QString, int> Nome inteligível and Topologic Relation
    */
    static QMap<QString, int> * getTypesOperations( QgsVectorLayer* lyrTarget, QgsVectorLayer* lyrReference );

    /**
    * \brief Gets the topologic dimension
    * \param geomType          Geometry Type
    * \returns short int       Topologic Dimension
    */
    static short int dimensionGeometry(QGis::GeometryType geomType);

private:

    /**
    * \brief Sets the target layer and reference layer
    * \param layerTarget       Target Layer
    * \param layerReference    Reference Layer
    */
    void setQuery( QgsVectorLayer *layerTarget, QgsVectorLayer *layerReference );

    /**
    * \brief Verify has valid Geometry in feature
    * \param QgsFeature       Feature
    */
    bool hasValidGeometry(QgsFeature &feature);

    /**
    * \brief Build the Spatial Index
    */
    void setSpatialIndexReference();

    /**
    * \brief Execute query
    * \param qsetIndexResult    Reference to QSet contains the result query
    * \param relation           Enum Topologic Relation
    */
    void execQuery(QSet<int> & qsetIndexResult, int relation);

    /**
    * \brief Populate index Result
    * \param qsetIndexResult    Reference to QSet contains the result query
    * \param idTarget           Id of the feature Target
    * \param geomTarget         Geometry the feature Target
    * \param operation          Pointer to function of GEOS operation
    */
    void populateIndexResult(
            QSet<int> & qsetIndexResult, int idTarget, QgsGeometry * geomTarget,
            char(*operation)(const GEOSGeometry *, const GEOSGeometry *) );
    /**
    * \brief Populate index Result Disjoint
    * \param qsetIndexResult    Reference to QSet contains the result query
    * \param idTarget           Id of the feature Target
    * \param geomTarget         Geometry the feature Target
    * \param operation          Pointer to function of GEOS operation
    */
    void populateIndexResultDisjoint(
            QSet<int> & qsetIndexResult, int idTarget, QgsGeometry * geomTarget,
            char(*operation)(const GEOSGeometry *, const GEOSGeometry *) );

    MngProgressBar *mPb;
    bool mUseReferenceSelection;
    bool mUseTargetSelection;

    QgsReaderFeatures * mReaderFeaturesTarget;
    QgsVectorLayer * mLayerTarget;
    QgsVectorLayer * mLayerReference;
    QgsSpatialIndex  mIndexReference;
};

#endif // SPATIALQUERY_H

