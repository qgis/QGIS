/***************************************************************************
  topolTest.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
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

#include "qgsgeometry.h"
#include "qgsspatialindex.h"

#include "topolError.h"

class topolTest;
class QgisInterface;
class WKTReader;

enum ValidateType { ValidateAll, ValidateExtent, ValidateSelected };

typedef ErrorList( topolTest::*testFunction )( QgsVectorLayer *, QgsVectorLayer *, bool );

class TopologyRule
{
  public:
    testFunction f;
    bool useSecondLayer;
    bool useSpatialIndex;
    QList<QgsWkbTypes::GeometryType> layer1SupportedTypes;
    QList<QgsWkbTypes::GeometryType> layer2SupportedTypes;

    bool layer1AcceptsType( QgsWkbTypes::GeometryType type )
    {
      return layer1SupportedTypes.contains( type );
    }

    bool layer2AcceptsType( QgsWkbTypes::GeometryType type )
    {
      return layer2SupportedTypes.contains( type );
    }


    /**
     * Constructor
     * initializes the test to use both layers
     */
    explicit TopologyRule( testFunction f0 = nullptr,
                           bool useSecondLayer0 = true,
                           bool useSpatialIndex0 = false,
                           const QList<QgsWkbTypes::GeometryType> &layer1SupportedTypes0 = QList<QgsWkbTypes::GeometryType>(),
                           const QList<QgsWkbTypes::GeometryType> &layer2SupportedTypes0 = QList<QgsWkbTypes::GeometryType>()
                         )
      : f( f0 )
      , useSecondLayer( useSecondLayer0 )
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
    bool operator()( const QgsPointXY &p1, const QgsPointXY &p2 )const
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
    explicit topolTest( QgisInterface *qgsIface );
    ~topolTest() override;

    /**
     * Returns copy of the test map
     */
    QMap<QString, TopologyRule> testMap() { return mTopologyRuleMap; }

    /**
     * Runs the test and returns all found errors
     * \param testName name of the test
     * \param layer1 pointer to the first layer
     * \param layer2 pointer to the second layer
     * \param type type what features to validate
     */
    ErrorList runTest( const QString &testName, QgsVectorLayer *layer1, QgsVectorLayer *layer2, ValidateType type );

    /**
     * Checks for intersections of the two layers
     * \param layer1 pointer to the first layer
     * \param layer2 pointer to the second layer
     */
    ErrorList checkOverlapWithLayer( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for dangling lines
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkDanglingLines( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for points not covered by any segment
     * \param layer1 pointer to the first layer
     * \param layer2 pointer to the second layer
     */
    ErrorList checkPointCoveredBySegment( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for invalid geometries
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkValid( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for duplicate geometries
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkDuplicates( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for pseudo nodes
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkPseudos( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for overlaps of polygons from same layer
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkOverlaps( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for gaps among polygons from same layer
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkGaps( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for points form one layer that are not covered by line ends form another layer
     * \param layer1 pointer to the first point layer
     * \param layer2 pointer to the second line layer
     */
    ErrorList checkPointCoveredByLineEnds( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for points that are not inside any polygons from another layer
     * \param layer1 pointer to the first point layer
     * \param layer2 pointer to the second polygon layer
     */
    ErrorList checkPointInPolygon( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for polygons that do not contain any points form another layer
     * \param layer1 pointer to the first polygon layer
     * \param layer2 pointer to the second point layer
     */
    ErrorList checkPolygonContainsPoint( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for multipart features
     * \param layer1 pointer to the first layer
     * \param layer2 not used
     */
    ErrorList checkMultipart( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );

    /**
     * Checks for line features that do not have both ends covered by points from another layer
     * \param layer1 pointer to the first linelayer
     * \param layer2 pointer to the second point layer
     */
    ErrorList checkyLineEndsCoveredByPoints( QgsVectorLayer *layer1, QgsVectorLayer *layer2, bool isExtent );


  public slots:

    //! Cancels the test
    void setTestCanceled();

  private:
    QMap<QString, QgsSpatialIndex *> mLayerIndexes;
    QMap<QString, TopologyRule> mTopologyRuleMap;

    QList<FeatureLayer> mFeatureList1;
    QMap<QgsFeatureId, FeatureLayer> mFeatureMap2;

    QgisInterface *qgsInterface = nullptr;
    bool mTestCanceled;

    /**
     * Builds spatial index for the layer
     * \param layer pointer to the layer
     */
    QgsSpatialIndex *createIndex( QgsVectorLayer *layer, const QgsRectangle &extent );

    /**
     * Fills the feature list with features from the layer
     * \param layer pointer to the layer
     * \param extent of the layer to add features
     */
    void fillFeatureList( QgsVectorLayer *layer, const QgsRectangle &extent );

    /**
     * Fills the feature map with features from the layer
     * \param layer pointer to the layer
     * \param extent of the layer to create index
     */
    void fillFeatureMap( QgsVectorLayer *layer, const QgsRectangle &extent );

    /**
     * Returns true if the test was canceled
     */
    bool testCanceled();

  signals:

    /**
     * Informs progress dialog about current status
     * \param value process status
     */
    void progress( int value );
};

#endif
