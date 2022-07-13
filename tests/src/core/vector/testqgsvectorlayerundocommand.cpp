/***************************************************************************
     testqgsvectorlayerundocommand.cpp
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
#include "qgsvectorlayerundocommand.h"

class TestQgsVectorLayerUndoCommand: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void baseClass();
    void changeAttribute();

  private:
    QgsVectorLayer *mLayerPoint = nullptr;
    QgsVectorLayerEditBuffer *mLayerEditBuffer = nullptr;
};

//runs before all tests
void TestQgsVectorLayerUndoCommand::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mLayerPoint = new QgsVectorLayer( "Point", "layer point", "memory" );
  QVERIFY( mLayerPoint->isValid() );

  mLayerEditBuffer = new QgsVectorLayerEditBuffer( mLayerPoint );
}

//runs after all tests
void TestQgsVectorLayerUndoCommand::cleanupTestCase()
{
  delete mLayerEditBuffer;
  delete mLayerPoint;

  QgsApplication::exitQgis();
}

void TestQgsVectorLayerUndoCommand::baseClass()
{
  QgsVectorLayerUndoCommand cmd( mLayerEditBuffer );

  QCOMPARE( cmd.layer(), mLayerPoint );
  QCOMPARE( cmd.id(), -1 ); // we should not need an override for this
}

void TestQgsVectorLayerUndoCommand::changeAttribute()
{
  std::unique_ptr< QgsVectorLayerUndoCommandChangeAttribute > cmd;

  // Should this be allowed at all, when fid is nonexistent ?
  cmd.reset( new QgsVectorLayerUndoCommandChangeAttribute(
               mLayerEditBuffer,
               1, // Positive (not-new) non-existent FID
               0, "newvalue", "oldvalue" ) );
  QCOMPARE( cmd->layer(), mLayerPoint );
  QCOMPARE( cmd->id(), -1 );
  cmd->undo();

  // Test for https://github.com/qgis/QGIS/issues/23243
  cmd.reset( new QgsVectorLayerUndoCommandChangeAttribute(
               mLayerEditBuffer,
               -1, // Negative (new) non-existent FID
               0, "newvalue", "oldvalue" ) );
  QCOMPARE( cmd->layer(), mLayerPoint );
  QCOMPARE( cmd->id(), -1 );
  cmd->undo();
}


QGSTEST_MAIN( TestQgsVectorLayerUndoCommand )
#include "testqgsvectorlayerundocommand.moc"




