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
#include "qgsmultirenderchecker.h"
#include "qgsproperty.h"
#include "qgsexpression.h"
#include "qgsapplication.h"
#include "qgsproject.h"

#include <QObject>
#include "qgstest.h"

class TestQgsComposerObject : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerObject() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsComposerObject
    void composition(); //test fetching composition from QgsComposerObject
    void writeReadXml(); //test writing object to xml and reading back from it
    void writeRetrieveDDProperty(); //test writing and retrieving dd properties from xml
    void customProperties(); //test setting/getting custom properties
    void writeRetrieveCustomProperties(); //test writing/retreiving custom properties from xml

  private:
    bool renderCheck( const QString &testName, QImage &image, int mismatchCount = 0 );
    QgsComposition *mComposition = nullptr;
    QString mReport;

};

void TestQgsComposerObject::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = QStringLiteral( "<h1>Composer Object Tests</h1>\n" );
}

void TestQgsComposerObject::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsComposerObject::init()
{
}

void TestQgsComposerObject::cleanup()
{
}

void TestQgsComposerObject::creation()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );
  QVERIFY( object );
  delete object;
}

void TestQgsComposerObject::composition()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );
  QCOMPARE( object->composition(), mComposition );
  delete object;
}

void TestQgsComposerObject::writeReadXml()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no node
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement noNode;
  QCOMPARE( object->writeXml( noNode, doc ), false );

  //test writing with node
  QDomElement composerObjectElem = doc.createElement( QStringLiteral( "ComposerObject" ) );
  rootNode.appendChild( composerObjectElem );
  QVERIFY( object->writeXml( composerObjectElem, doc ) );

  //check if object node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "ComposerObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node
  QgsComposerObject *readObject = new QgsComposerObject( mComposition );

  //test reading with no node
  QCOMPARE( readObject->readXml( noNode, doc ), false );

  //test reading node
  QVERIFY( readObject->readXml( composerObjectElem, doc ) );

  delete object;
  delete readObject;
}

void TestQgsComposerObject::writeRetrieveDDProperty()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );
  object->dataDefinedProperties().setProperty( QgsComposerObject::TestProperty, QgsProperty::fromExpression( QStringLiteral( "10 + 40" ) ) );
  object->prepareProperties();

  //test writing object with dd settings
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement composerObjectElem = doc.createElement( QStringLiteral( "ComposerObject" ) );
  rootNode.appendChild( composerObjectElem );
  QVERIFY( object->writeXml( composerObjectElem, doc ) );

  //check if object node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "ComposerObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node containing dd settings
  QgsComposerObject *readObject = new QgsComposerObject( mComposition );
  QVERIFY( readObject->readXml( composerObjectElem, doc ) );

  //test getting not set dd from restored object
  QgsProperty dd = readObject->dataDefinedProperties().property( QgsComposerObject::BlendMode );
  QVERIFY( !dd );

  //test getting good property
  dd = readObject->dataDefinedProperties().property( QgsComposerObject::TestProperty );
  QVERIFY( dd );
  QVERIFY( dd.isActive() );
  QCOMPARE( dd.propertyType(), QgsProperty::ExpressionBasedProperty );

  delete object;
  delete readObject;
}

void TestQgsComposerObject::customProperties()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );

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
}

void TestQgsComposerObject::writeRetrieveCustomProperties()
{
  QgsComposerObject *object = new QgsComposerObject( mComposition );
  object->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  object->setCustomProperty( QStringLiteral( "testprop2" ), 5 );

  //test writing object with custom properties
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement composerObjectElem = doc.createElement( QStringLiteral( "ComposerObject" ) );
  rootNode.appendChild( composerObjectElem );
  QVERIFY( object->writeXml( composerObjectElem, doc ) );

  //check if object node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "ComposerObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node containing custom properties
  QgsComposerObject *readObject = new QgsComposerObject( mComposition );
  QVERIFY( readObject->readXml( composerObjectElem, doc ) );

  //test retrieved custom properties
  QCOMPARE( readObject->customProperties().length(), 2 );
  QVERIFY( readObject->customProperties().contains( QString( "testprop" ) ) );
  QVERIFY( readObject->customProperties().contains( QString( "testprop2" ) ) );
  QCOMPARE( readObject->customProperty( "testprop" ).toString(), QString( "testval" ) );
  QCOMPARE( readObject->customProperty( "testprop2" ).toInt(), 5 );

  delete object;
  delete readObject;
}

bool TestQgsComposerObject::renderCheck( const QString &testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + '/';
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsComposerObject )
#include "testqgscomposerobject.moc"
