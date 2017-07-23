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
#include "qgsproject.h"

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
    void customProperties();
    void context();

  private:
    QString mReport;

};

void TestQgsLayoutObject::initTestCase()
{
  mReport = "<h1>Layout Object Tests</h1>\n";
}

void TestQgsLayoutObject::cleanupTestCase()
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

void TestQgsLayoutObject::init()
{

}

void TestQgsLayoutObject::cleanup()
{

}

void TestQgsLayoutObject::creation()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  QgsLayoutObject *object = new QgsLayoutObject( layout );
  QVERIFY( object );
  delete object;
  delete layout;
}

void TestQgsLayoutObject::layout()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  QgsLayoutObject *object = new QgsLayoutObject( layout );
  QCOMPARE( object->layout(), layout );
  delete object;
  delete layout;
}

void TestQgsLayoutObject::customProperties()
{
  QgsProject p;
  QgsLayout *layout = new QgsLayout( &p );
  QgsLayoutObject *object = new QgsLayoutObject( layout );

  QCOMPARE( object->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );
  QVERIFY( object->customProperties().isEmpty() );
  object->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  QCOMPARE( object->customProperty( "testprop", "defaultval" ).toString(), QString( "testval" ) );
  QCOMPARE( object->customProperties().length(), 1 );
  QCOMPARE( object->customProperties().at( 0 ), QString( "testprop" ) );

  //test no crash
  object->removeCustomProperty( QStringLiteral( "badprop" ) );

  object->removeCustomProperty( QStringLiteral( "testprop" ) );
  QVERIFY( object->customProperties().isEmpty() );
  QCOMPARE( object->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );

  object->setCustomProperty( QStringLiteral( "testprop1" ), "testval1" );
  object->setCustomProperty( QStringLiteral( "testprop2" ), "testval2" );
  QStringList keys = object->customProperties();
  QCOMPARE( keys.length(), 2 );
  QVERIFY( keys.contains( "testprop1" ) );
  QVERIFY( keys.contains( "testprop2" ) );

  delete object;
  delete layout;
}

void TestQgsLayoutObject::context()
{
  QgsProject p;
  p.setTitle( "my title" );
  QgsLayout l( &p );
  l.setName( "my layout" );

  QgsLayoutObject *object = new QgsLayoutObject( nullptr );
  // no crash
  QgsExpressionContext c = object->createExpressionContext();
  delete object;

  object = new QgsLayoutObject( &l );
  c = object->createExpressionContext();
  // should contain project variables
  QCOMPARE( c.variable( "project_title" ).toString(), QStringLiteral( "my title" ) );
  // and layout variables
  QCOMPARE( c.variable( "layout_name" ).toString(), QStringLiteral( "my layout" ) );
  delete object;
}


QGSTEST_MAIN( TestQgsLayoutObject )
#include "testqgslayoutobject.moc"
