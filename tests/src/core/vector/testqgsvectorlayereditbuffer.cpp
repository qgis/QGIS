/***************************************************************************
     testqgsvectorlayereditbuffer.cpp
     --------------------------------------
    Date                 : May 2022
    Copyright            : (C) 2022 by Sandro Santilli
    Email                : strk at kbt dot io
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

#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"

class TestQgsVectorLayerEditBuffer: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void constructor();
  private:
    QgsVectorLayer *mLayerPoint = nullptr;
};

//runs before all tests
void TestQgsVectorLayerEditBuffer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mLayerPoint = new QgsVectorLayer( "Point", "layer point", "memory" );
  QVERIFY( mLayerPoint->isValid() );
}

//runs after all tests
void TestQgsVectorLayerEditBuffer::cleanupTestCase()
{
  delete mLayerPoint;

  QgsApplication::exitQgis();
}

void TestQgsVectorLayerEditBuffer::constructor()
{
  QgsVectorLayerEditBuffer buf( mLayerPoint );
  QVERIFY( ! buf.isModified() );

  // TODO: buf.addedFeatures().isEmpty()
  // TODO: buf.allAddedOrEditedFeatures().isEmpty()
  // TODO: buf.changedAttributeValues().isEmpty()
  // TODO: all other inspector methods
}


QGSTEST_MAIN( TestQgsVectorLayerEditBuffer )
#include "testqgsvectorlayereditbuffer.moc"




