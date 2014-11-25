/***************************************************************************
                         testqgscomposerobject.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposerobject.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgsdatadefined.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerObject: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsComposerObject
    void composition(); //test fetching composition from QgsComposerObject
    void writeReadXml(); //test writing object to xml and reading back from it
    void setRetrieveDDProperty(); //test setting and retreiving a data defined property
    void evaluateDDProperty(); //test evaluating data defined properties
    void writeRetrieveDDProperty(); //test writing and retrieving dd properties from xml

  private:
    bool renderCheck( QString testName, QImage &image, int mismatchCount = 0 );
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;

};

void TestQgsComposerObject::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer Object Tests</h1>\n";
}

void TestQgsComposerObject::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerObject::init()
{

}

void TestQgsComposerObject::cleanup()
{

}

void TestQgsComposerObject::creation()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  QVERIFY( object );
  delete object;
}

void TestQgsComposerObject::composition()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  QCOMPARE( object->composition(), mComposition );
  delete object;
}

void TestQgsComposerObject::writeReadXml()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( "qgis" );
  QDomElement noNode;
  QCOMPARE( object->writeXML( noNode, doc ), false );

  //test writing with node
  QDomElement composerObjectElem = doc.createElement( "ComposerObject" );
  rootNode.appendChild( composerObjectElem );
  QVERIFY( object->writeXML( composerObjectElem, doc ) );

  //check if object node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( "ComposerObject" );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node
  QgsComposerObject* readObject = new QgsComposerObject( mComposition );

  //test reading with no node
  QCOMPARE( readObject->readXML( noNode, doc ), false );

  //test reading node
  QVERIFY( readObject->readXML( composerObjectElem, doc ) );

  delete object;
  delete readObject;
}

void TestQgsComposerObject::setRetrieveDDProperty()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  object->setDataDefinedProperty( QgsComposerObject::Transparency, true, true, QString( "10 + 40" ), QString() );
  object->prepareDataDefinedExpressions();

  //test retrieving bad properties
  QgsDataDefined* result = 0;
  result = object->dataDefinedProperty( QgsComposerObject::NoProperty );
  QVERIFY( !result );
  result = object->dataDefinedProperty( QgsComposerObject::AllProperties );
  QVERIFY( !result );
  //property not set
  result = object->dataDefinedProperty( QgsComposerObject::BlendMode );
  QVERIFY( !result );

  //test retrieving good property
  result = object->dataDefinedProperty( QgsComposerObject::Transparency );
  QVERIFY( result );
  QVERIFY( result->isActive() );
  QVERIFY( result->useExpression() );
  QCOMPARE( result->expression()->dump(), QString( "10 + 40" ) );

  delete object;
}

void TestQgsComposerObject::evaluateDDProperty()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  object->setDataDefinedProperty( QgsComposerObject::Transparency, true, true, QString( "10 + 40" ), QString() );
  object->prepareDataDefinedExpressions();

  QVariant result;
  //test evaluating bad properties
  QCOMPARE( object->dataDefinedEvaluate( QgsComposerObject::NoProperty, result ), false );
  QCOMPARE( object->dataDefinedEvaluate( QgsComposerObject::AllProperties, result ), false );
  //not set property
  QCOMPARE( object->dataDefinedEvaluate( QgsComposerObject::BlendMode, result ), false );

  //test retrieving good property
  QVERIFY( object->dataDefinedEvaluate( QgsComposerObject::Transparency, result ) );
  QCOMPARE( result.toInt(), 50 );

  delete object;
}

void TestQgsComposerObject::writeRetrieveDDProperty()
{
  QgsComposerObject* object = new QgsComposerObject( mComposition );
  object->setDataDefinedProperty( QgsComposerObject::TestProperty, true, true, QString( "10 + 40" ), QString() );
  object->prepareDataDefinedExpressions();

  //test writing object with dd settings
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      "qgis", "http://mrcc.com/qgis.dtd", "SYSTEM" );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( "qgis" );
  QDomElement composerObjectElem = doc.createElement( "ComposerObject" );
  rootNode.appendChild( composerObjectElem );
  QVERIFY( object->writeXML( composerObjectElem, doc ) );

  //check if object node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( "ComposerObject" );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node containing dd settings
  QgsComposerObject* readObject = new QgsComposerObject( mComposition );
  QVERIFY( readObject->readXML( composerObjectElem, doc ) );

  QVariant result;
  //test getting not set dd from restored object
  QgsDataDefined* dd = readObject->dataDefinedProperty( QgsComposerObject::BlendMode );
  QVERIFY( !dd );

  //test getting good property
  dd = readObject->dataDefinedProperty( QgsComposerObject::TestProperty );
  QVERIFY( dd );
  QVERIFY( dd->isActive() );
  QVERIFY( dd->useExpression() );
  //evaluating restored dd property
  QVERIFY( readObject->dataDefinedEvaluate( QgsComposerObject::TestProperty, result ) );
  QCOMPARE( result.toInt(), 50 );

  delete object;
  delete readObject;
}

bool TestQgsComposerObject::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator();
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsComposerObject )
#include "testqgscomposerobject.moc"
