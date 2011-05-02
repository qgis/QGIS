/***************************************************************************
  testqgsapplication.cpp
  --------------------------------------
Date                 : Sun Sep 16 12:22:49 AKDT 2007
Copyright            : (C) 2007 by Gary E. Sherman
Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <iostream>

#include <QPixmap>

#include <qgsapplication.h>

//header for class being tested
#include <qgsapplication.h>

class TestQgsApplication: public QObject
{
    Q_OBJECT;
  private slots:
    void checkTheme();
    void initTestCase();
  private:
    QString getQgisPath();
};


void TestQgsApplication::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::showSettings();
};

void TestQgsApplication::checkTheme()
{
  QString myIconPath = QgsApplication::defaultThemePath();
  QPixmap myPixmap;
  myPixmap.load( myIconPath + "/mIconProjectionDisabled.png" );
  qDebug( "Checking if a theme icon exists:" );
  qDebug( "%s/mIconProjectionDisabled.png", myIconPath.toLocal8Bit().constData() );
  QVERIFY( !myPixmap.isNull() );

};


QTEST_MAIN( TestQgsApplication )
#include "moc_testqgsapplication.cxx"

