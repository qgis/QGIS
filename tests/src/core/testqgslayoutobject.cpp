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
#include "qgstest.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsprintlayout.h"

class TestQgsLayoutObject: public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutObject() : QgsTest( QStringLiteral( "Layout Object Tests" ) ) {}

  private slots:

    void creation(); //test creation of QgsLayoutObject
    void layout(); //test fetching layout from QgsLayoutObject
    void customProperties();
    void context();
    void writeReadXml();
    void writeRetrieveDDProperty(); //test writing and retrieving dd properties from xml
    void writeRetrieveCustomProperties(); //test writing/retrieving custom properties from xml

};


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
  const QStringList keys = object->customProperties();
  QCOMPARE( keys.length(), 2 );
  QVERIFY( keys.contains( "testprop1" ) );
  QVERIFY( keys.contains( "testprop2" ) );

  delete object;
  delete layout;
}

void TestQgsLayoutObject::context()
{
  QgsProject p;
  p.setTitle( QStringLiteral( "my title" ) );
  QgsPrintLayout l( &p );
  l.setName( QStringLiteral( "my layout" ) );

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

void TestQgsLayoutObject::writeReadXml()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutObject *object = new QgsLayoutObject( &l );
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );

  //test writing with no parent node
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QDomElement noNode;
  QCOMPARE( object->writeObjectPropertiesToElement( noNode, doc, QgsReadWriteContext() ), false );

  //test writing with node
  QDomElement layoutObjectElem = doc.createElement( QStringLiteral( "item" ) );
  rootNode.appendChild( layoutObjectElem );
  QVERIFY( object->writeObjectPropertiesToElement( layoutObjectElem, doc, QgsReadWriteContext() ) );

  //check if object node was written
  const QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "LayoutObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node
  QgsLayoutObject *readObject = new QgsLayoutObject( &l );

  //test reading with no node
  QCOMPARE( readObject->readObjectPropertiesFromElement( noNode, doc, QgsReadWriteContext() ), false );

  //test node with no layout object child
  const QDomElement badLayoutObjectElem = doc.createElement( QStringLiteral( "item" ) );
  rootNode.appendChild( badLayoutObjectElem );
  QCOMPARE( readObject->readObjectPropertiesFromElement( badLayoutObjectElem, doc, QgsReadWriteContext() ), false );

  //test reading node
  QVERIFY( readObject->readObjectPropertiesFromElement( layoutObjectElem, doc, QgsReadWriteContext() ) );

  delete object;
  delete readObject;
}

void TestQgsLayoutObject::writeRetrieveDDProperty()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutObject *object = new QgsLayoutObject( &l );
  object->dataDefinedProperties().setProperty( QgsLayoutObject::TestProperty, QgsProperty::fromExpression( QStringLiteral( "10 + 40" ) ) );

  //test writing object with dd settings
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QVERIFY( object->writeObjectPropertiesToElement( rootNode, doc, QgsReadWriteContext() ) );

  //check if object node was written
  const QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "LayoutObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node containing dd settings
  QgsLayoutObject *readObject = new QgsLayoutObject( &l );
  QVERIFY( readObject->readObjectPropertiesFromElement( rootNode, doc, QgsReadWriteContext() ) );

  //test getting not set dd from restored object
  QgsProperty dd = readObject->dataDefinedProperties().property( QgsLayoutObject::BlendMode );
  QVERIFY( !dd );

  //test getting good property
  dd = readObject->dataDefinedProperties().property( QgsLayoutObject::TestProperty );
  QVERIFY( dd );
  QVERIFY( dd.isActive() );
  QCOMPARE( dd.propertyType(), QgsProperty::ExpressionBasedProperty );

  delete object;
  delete readObject;
}

void TestQgsLayoutObject::writeRetrieveCustomProperties()
{
  QgsProject p;
  QgsLayout l( &p );

  QgsLayoutObject *object = new QgsLayoutObject( &l );

  object->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  object->setCustomProperty( QStringLiteral( "testprop2" ), 5 );

  //test writing object with custom properties
  QDomImplementation DomImplementation;
  const QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QVERIFY( object->writeObjectPropertiesToElement( rootNode, doc, QgsReadWriteContext() ) );

  //check if object node was written
  const QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "LayoutObject" ) );
  QCOMPARE( evalNodeList.count(), 1 );

  //test reading node containing custom properties
  QgsLayoutObject *readObject = new QgsLayoutObject( &l );
  QVERIFY( readObject->readObjectPropertiesFromElement( rootNode, doc, QgsReadWriteContext() ) );

  //test retrieved custom properties
  QCOMPARE( readObject->customProperties().length(), 2 );
  QVERIFY( readObject->customProperties().contains( QString( "testprop" ) ) );
  QVERIFY( readObject->customProperties().contains( QString( "testprop2" ) ) );
  QCOMPARE( readObject->customProperty( "testprop" ).toString(), QString( "testval" ) );
  QCOMPARE( readObject->customProperty( "testprop2" ).toInt(), 5 );

  delete object;
  delete readObject;
}


QGSTEST_MAIN( TestQgsLayoutObject )
#include "testqgslayoutobject.moc"
