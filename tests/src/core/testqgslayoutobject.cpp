/***************************************************************************
                         testqgslayoutobject.cpp
                         -----------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutobject.h"
#include "qgslayout.h"
#include "qgstest.h"

class TestQgsLayoutObject: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayoutObject
    void layout(); //test fetching layout from QgsLayoutObject

  private:
    QgsLayout *mLayout = nullptr;
    QString mReport;

};

void TestQgsLayoutObject::initTestCase()
{
  mLayout = new QgsLayout();
  mReport = "<h1>Layout Object Tests</h1>\n";
}

void TestQgsLayoutObject::cleanupTestCase()
{
  delete mLayout;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutObject::init()
{

}

void TestQgsLayoutObject::cleanup()
{

}

void TestQgsLayoutObject::creation()
{
  QgsLayoutObject *object = new QgsLayoutObject( mLayout );
  QVERIFY( object );
  delete object;
}

void TestQgsLayoutObject::layout()
{
  QgsLayoutObject *object = new QgsLayoutObject( mLayout );
  QCOMPARE( object->layout(), mLayout );
  delete object;
}

QGSTEST_MAIN( TestQgsLayoutObject )
#include "testqgslayoutobject.moc"
