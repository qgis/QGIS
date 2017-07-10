/***************************************************************************
                         testqgslayoututils.cpp
                         ---------------------
    begin                : July 2017
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

#include "qgslayout.h"
#include "qgstest.h"
#include "qgslayoututils.h"

class TestQgsLayoutUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void normalizedAngle(); //test normalised angle function

  private:
    QString mReport;

};

void TestQgsLayoutUtils::initTestCase()
{
  mReport = "<h1>Layout Utils Tests</h1>\n";
}

void TestQgsLayoutUtils::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutUtils::init()
{

}

void TestQgsLayoutUtils::cleanup()
{

}

void TestQgsLayoutUtils::normalizedAngle()
{
  QList< QPair< double, double > > testVals;
  testVals << qMakePair( 0.0, 0.0 );
  testVals << qMakePair( 90.0, 90.0 );
  testVals << qMakePair( 180.0, 180.0 );
  testVals << qMakePair( 270.0, 270.0 );
  testVals << qMakePair( 360.0, 0.0 );
  testVals << qMakePair( 390.0, 30.0 );
  testVals << qMakePair( 720.0, 0.0 );
  testVals << qMakePair( 730.0, 10.0 );
  testVals << qMakePair( -10.0, 350.0 );
  testVals << qMakePair( -360.0, 0.0 );
  testVals << qMakePair( -370.0, 350.0 );
  testVals << qMakePair( -760.0, 320.0 );

  //test normalized angle helper function
  QList< QPair< double, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )

  {
    double result = QgsLayoutUtils::normalizedAngle( ( *it ).first );
    qDebug() << QString( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QVERIFY( qgsDoubleNear( result, ( *it ).second ) );

  }

  //test with allowing negative angles
  QList< QPair< double, double > > negativeTestVals;
  negativeTestVals << qMakePair( 0.0, 0.0 );
  negativeTestVals << qMakePair( 90.0, 90.0 );
  negativeTestVals << qMakePair( 360.0, 0.0 );
  negativeTestVals << qMakePair( -10.0, -10.0 );
  negativeTestVals << qMakePair( -359.0, -359.0 );
  negativeTestVals << qMakePair( -360.0, 0.0 );
  negativeTestVals << qMakePair( -361.0, -1.0 );
  negativeTestVals << qMakePair( -370.0, -10.0 );
  negativeTestVals << qMakePair( -760.0, -40.0 );
  it = negativeTestVals.constBegin();
  for ( ; it != negativeTestVals.constEnd(); ++it )

  {
    double result = QgsLayoutUtils::normalizedAngle( ( *it ).first, true );
    qDebug() << QString( "actual: %1 expected: %2" ).arg( result ).arg( ( *it ).second );
    QVERIFY( qgsDoubleNear( result, ( *it ).second ) );

  }
}


QGSTEST_MAIN( TestQgsLayoutUtils )
#include "testqgslayoututils.moc"
