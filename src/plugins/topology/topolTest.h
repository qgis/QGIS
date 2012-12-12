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

typedef ErrorList (topolTest::*testFunction)(double, QgsVectorLayer*, QgsVectorLayer*);

enum ValidateType { ValidateAll, ValidateExtent, ValidateSelected };

class test
{
public:
  bool useSecondLayer;
  bool useTolerance;
  testFunction f;

  /**
   * Constructor
   * initializes the test to use both layers and not to use the tolerance
   */
  test()
  {
    useSecondLayer = true;
    useTolerance = false;
    f = 0;
  }
};

class topolTest: public QObject
{
Q_OBJECT

public:
  topolTest(QgisInterface* qgsIface);
  ~topolTest();

  /**
   * Returns copy of the test map
   */
  QMap<QString, test> testMap() { return mTestMap; }
  /**
   * Runs the test and returns all found errors
   * @param testName name of the test
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   * @param type type what features to validate
   * @param tolerance possible tolerance
   */
  ErrorList runTest(QString testName, QgsVectorLayer* layer1, QgsVectorLayer* layer2, ValidateType type, double tolerance);

  /**
   * Checks for intersections of the two layers
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   */
  ErrorList checkIntersections(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for self-intersections in the layer
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 not used
   */
  ErrorList checkSelfIntersections(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for features that are too close
   * @param tolerance allowed tolerance
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   */
  ErrorList checkCloseFeature(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for features from second layer, that are contained in features from first layer
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   */
  ErrorList checkPolygonContains(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for short segments
   * @param tolerance tolerance - not used
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   */
  ErrorList checkSegmentLength(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for dangling lines
   * @param tolerance allowed tolerance
   * @param layer1 pointer to the first layer
   * @param layer2 not used
   */
  ErrorList checkDanglingLines(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for points not covered by any segment
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 pointer to the second layer
   */
  ErrorList checkPointCoveredBySegment(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);
  /**
   * Checks for invalid geometries
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 not used
   */
  ErrorList checkValid(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);

  /**
   * Checks for duplicate geometries
   * @param tolerance not used
   * @param layer1 pointer to the first layer
   * @param layer2 not used
   */
  ErrorList checkDuplicates(double tolerance, QgsVectorLayer* layer1, QgsVectorLayer* layer2);


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
  QMap<QString, test> mTestMap;

  QList<FeatureLayer> mFeatureList1;
  QMap<int, FeatureLayer> mFeatureMap2;

  QgisInterface* theQgsInterface;
  bool mTestCancelled;

  /**
   * Builds spatial index for the layer
   * @param layer pointer to the layer
   */
  QgsSpatialIndex* createIndex(QgsVectorLayer* layer);
  /**
   * Fills the feature map with features from the layer
   * @param layer pointer to the layer
   */
  void fillFeatureMap(QgsVectorLayer* layer);
  /**
   * Returns true if the test was cancelled
   */
  bool testCancelled();

signals:
  /**
   * Informs progress dialog about current status
   * @param value process status
   */
  void progress(int value);
};

#endif
