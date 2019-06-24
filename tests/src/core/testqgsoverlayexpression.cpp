/***************************************************************************
     test_qgsoverlayexpression.cpp
     --------------------------------------
    Date                 : 20.6.2019
    Copyright            : (C) 2019 by Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QtConcurrentMap>

#include <qgsapplication.h>
//header for class being tested
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsgeometry.h"
#include "qgsrenderchecker.h"
#include "qgsexpressioncontext.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsvectorlayerutils.h"
#include "qgsexpressioncontextutils.h"

class TestQgsOverlayExpression: public QObject
{
    Q_OBJECT

  public:

    TestQgsOverlayExpression() = default;

  private:
    QgsVectorLayer *mPolysLayer = nullptr;
    QgsVectorLayer *mRectanglesLayer = nullptr;

  private slots:

    void initTestCase();

    void cleanupTestCase();

    void testIntersect();
};



void TestQgsOverlayExpression::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  QString polysFileName = testDataDir + "polys.shp";
  QFileInfo polysFileInfo( polysFileName );
  mPolysLayer = new QgsVectorLayer( polysFileInfo.filePath(),
                                    QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mPolysLayer );

  QString rectanglesFileName = testDataDir + "rectangles.shp";
  QFileInfo rectanglesFileInfo( rectanglesFileName );
  mRectanglesLayer = new QgsVectorLayer( rectanglesFileInfo.filePath(),
                                         QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( mRectanglesLayer );

}

void TestQgsOverlayExpression::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOverlayExpression::testIntersect()
{
  QgsExpressionContext context;

  QgsExpression expression( "geometry_overlay_intersects('rectangles')" );

  expression.prepare( &context );

  QgsFeature feat;
  mPolysLayer->getFeatures().nextFeature( feat );

  context.setFeature( feat );
  const QVariant result = expression.evaluate( &context );

  QVERIFY( result.toBool() );
}

QGSTEST_MAIN( TestQgsOverlayExpression )

#include "testqgsoverlayexpression.moc"
