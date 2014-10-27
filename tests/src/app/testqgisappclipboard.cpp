/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Frida  Nov 23  2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>

#include <QApplication>
#include <QObject>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QtTest>

#include <qgisapp.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsclipboard.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>

/** \ingroup UnitTests
 * This is a unit test for the QgisApp clipboard.
 */
class TestQgisAppClipboard: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void copyPaste();

  private:
    QgisApp * mQgisApp;
    QString mTestDataDir;
};

//runs before all tests
void TestQgisAppClipboard::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QString( TEST_DATA_DIR ) + QDir::separator(); //defined in CmakeLists.txt
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgisAppClipboard::cleanupTestCase()
{
}

void TestQgisAppClipboard::copyPaste()
{
  qDebug() << "TestQgisAppClipboard::copyPaste()";

  QMap<QString, int> filesCounts;
  filesCounts.insert( "points.shp", 17 );
  filesCounts.insert( "lines.shp", 6 );
  filesCounts.insert( "polys.shp", 10 );

  foreach ( QString fileName, filesCounts.keys() )
  {
    // add vector layer
    QString filePath = mTestDataDir + fileName;
    qDebug() << "add vector layer: " << filePath;
    QgsVectorLayer *inputLayer = mQgisApp->addVectorLayer( filePath, fileName, "ogr" );
    QVERIFY( inputLayer->isValid() );

    // copy all features to clipboard
    inputLayer->selectAll();
    mQgisApp->editCopy( inputLayer );

    QgsFeatureList features = mQgisApp->clipboard()->copyOf();
    qDebug() << features.size() << " features copied to clipboard";

    QVERIFY( features.size() == filesCounts.value( fileName ) );

    QgsVectorLayer *pastedLayer = mQgisApp->pasteAsNewMemoryVector( "pasted" );
    QVERIFY( pastedLayer );
    QVERIFY( pastedLayer->isValid() );
    qDebug() << pastedLayer->featureCount() << " features in pasted layer";
    QVERIFY( pastedLayer->featureCount() == filesCounts.value( fileName ) );
  }
}

QTEST_MAIN( TestQgisAppClipboard )
#include "moc_testqgisappclipboard.cxx"
