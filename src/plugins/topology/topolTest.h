/***************************************************************************
  topolTest.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOPOLTEST_H
#define TOPOLTEST_H

#include <QObject>

#include <qgsvectorlayer.h>
#include <qgsgeometry.h>
#include "qgsspatialindex.h"

#include "topolError.h"

class topolTest;
class QgisInterface;
class WKTReader;

enum ValidateType { ValidateAll, ValidateExtent, ValidateSelected };

typedef ErrorList( topolTest::*testFunction )( double, QgsVectorLayer*, QgsVectorLayer*, bool );

class TopologyRule
{
  public:
    testFunction f;
    bool useSecondLayer;
    bool useTolerance;
    bool useSpatialIndex;
    QList<QGis::GeometryType> layer1SupportedTypes;
    QList<QGis::GeometryType> layer2SupportedTypes;

    bool layer1AcceptsType( QGis::GeometryType type )
    {
      return layer1SupportedTypes.contains( type );
    }

    bool layer2AcceptsType( QGis::GeometryType type )
    {
      return layer2SupportedTypes.contains( type );
    }


    /**
     * Constructor
     * initializes the test to use both layers and not to use the tolerance
     */
    TopologyRule( testFunction f0 = 0,
                  bool useSecondLayer0 = true,
                  bool useTolerance0 = false,
                  bool useSpatialIndex0 = false,
                  QList<QGis::GeometryType> layer1SupportedTypes0 = QList<QGis::GeometryType>(),
                  QList<QGis::GeometryType> layer2SupportedTypes0 = QList<QGis::GeometryType>()
                )
        : f( f0 )
        , useSecondLayer( useSecondLayer0 )
        , useTolerance( useTolerance0 )
        , useSpatialIndex( useSpatialIndex0 )
        , layer1SupportedTypes( layer1SupportedTypes0 )
        , layer2SupportedTypes( layer2SupportedTypes0 )
    {}
};

/**
  helper class to pass as comparator to map,set etc..
  */
class PointComparer
{
  public:
    bool operator()( QgsPoint p1, QgsPoint p2 )const
    {
      if ( p1.x() < p2.x() )
      {
        return true;
      }

      if ( p1.x() == p2.x() && p1.y() < p2.y() )
      {
        return true;
      }

      return false;
    }
};


class topolTest: public QObject
{
    Q_OBJECT

  public:
    topolTest( QgisInterface* qgsIface );
    ~topolTest();

    /**
     * Returns copy of the test map
     */
    QMap<QString, TopologyRule> testMap() { return mTopologyRuleMap; }
    /**
     * Runs the test and returns all found errors
     * @param testName name of the test
     * @param layer1 pointer to the first layer
     * @param layer2 pointer to the second layer
     * @param type type what features to validate
     * @param tolerance possible tolerance
     */
    ErrorList runTest( QString testName, QgsVectorLayer* layer1, QgsVectorLayer* layer2, ValidateType type, double tolerance );

    /**
     * Checks for intersections of the two layers
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 pointer to the second layer
     */
    ErrorList checkOverlapWithLayer( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );
    /**
     * Checks for self-intersections in the layer
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkSelfIntersections( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

#if 0 //unused and broken
    /**
     * Checks for features that are too close
     * @param tolerance allowed tolerance
     * @param layer1 pointer to the first layer
     * @param layer2 pointer to the second layer
     */
    ErrorList checkCloseFeature( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );
#endif

    /**
    * Checks for short segments
    * @param tolerance tolerance - not used
    * @param layer1 pointer to the first layer
    * @param layer2 pointer to the second layer
    */
    ErrorList checkSegmentLength( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );
    /**
     * Checks for dangling lines
     * @param tolerance allowed tolerance
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkDanglingLines( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );
    /**
     * Checks for points not covered by any segment
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 pointer to the second layer
     */
    ErrorList checkPointCoveredBySegment( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );
    /**
     * Checks for invalid geometries
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkValid( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for duplicate geometries
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkDuplicates( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for pseudo nodes
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkPseudos( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for overlaps of polygons from same layer
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkOverlaps( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for gaps among polygons from same layer
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkGaps( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for points form one layer that are not covered by line ends form another layer
     * @param tolerance not used
     * @param layer1 pointer to the first point layer
     * @param layer2 pointer to the second line layer
     */
    ErrorList checkPointCoveredByLineEnds( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for points that are not inside any polygons from another layer
     * @param tolerance not used
     * @param layer1 pointer to the first point layer
     * @param layer2 pointer to the second polygon layer
     */
    ErrorList checkPointInPolygon( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for polygons that do not contain any points form another layer
     * @param tolerance not used
     * @param layer1 pointer to the first polygon layer
     * @param layer2 pointer to the second point layer
     */
    ErrorList checkPolygonContainsPoint( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for multipart features
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    ErrorList checkMultipart( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );

    /**
     * Checks for line features that do not have both ends covered by points from another layer
     * @param tolerance not used
     * @param layer1 pointer to the first linelayer
     * @param layer2 pointer to the second point layer
     */
    ErrorList checkyLineEndsCoveredByPoints( double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2, bool isExtent );


  public slots:
    /**
     * Checks for invalid geometries
     * @param tolerance not used
     * @param layer1 pointer to the first layer
     * @param layer2 not used
     */
    void setTestCancelled();

  private:
    QMap<QString, QgsSpatialIndex*> mLayerIndexes;
    QMap<QString, TopologyRule> mTopologyRuleMap;

    QList<FeatureLayer> mFeatureList1;
    QMap<QgsFeatureId, FeatureLayer> mFeatureMap2;

    QgisInterface* theQgsInterface;
    bool mTestCancelled;

    /**
     * Builds spatial index for the layer
     * @param layer pointer to the layer
     */
    QgsSpatialIndex* createIndex( QgsVectorLayer* layer, QgsRectangle extent );

    /**
     * Fills the feature list with features from the layer
     * @param layer pointer to the layer
     * @param extent of the layer to add features
     */
    void fillFeatureList( QgsVectorLayer* layer, QgsRectangle extent );

    /**
     * Fills the feature map with features from the layer
     * @param layer pointer to the layer
     * @param extent of the layer to create index
     */
    void fillFeatureMap( QgsVectorLayer* layer, QgsRectangle extent );

    /**
     * Returns true if the test was cancelled
     */
    bool testCancelled();

  signals:
    /**
     * Informs progress dialog about current status
     * @param value process status
     */
    void progress( int value );
};

#endif
