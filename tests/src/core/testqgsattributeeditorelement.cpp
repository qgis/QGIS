/***************************************************************************
     testqgsattributeeditorelement.cpp
     --------------------------------------
    Date                 : 28.04.2022
    Copyright            : (C) 2022 by Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QObject>

#include "qgsattributeeditorelement.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorcontainer.h"
#include "qgseditformconfig.h"
#include "qgsfontutils.h"
#include "qgstest.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsAttributeEditorElement label font and color serialization
 */
class TestQgsAttributeEditorElement: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testLabelFontAndColor();
};

void TestQgsAttributeEditorElement::initTestCase()
{
  // Runs once before any tests are run

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsSettings().clear();
}


void TestQgsAttributeEditorElement::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}

void TestQgsAttributeEditorElement::testLabelFontAndColor()
{

  QgsEditFormConfig editFormConfig;

  QFont font0 { QgsFontUtils::getStandardTestFont() };
  font0.setBold( true );
  font0.setItalic( false );
  font0.setUnderline( true );
  font0.setStrikeOut( false );

  QgsAttributeEditorElement::LabelStyle style
  {
    QColor( Qt::GlobalColor::darkCyan ),
    font0,
    true,
    true
  };


  QgsAttributeEditorField *field1 = new QgsAttributeEditorField( "f1", 0, nullptr );

  QFont font1 { QgsFontUtils::getStandardTestFont() };
  font1.setBold( true );
  font1.setItalic( true );
  font1.setUnderline( true );
  font1.setStrikeOut( true );

  field1->setLabelStyle(
  {
    QColor( Qt::GlobalColor::blue ),
    font1,
    true,
    true} );

  editFormConfig.invisibleRootContainer()->addChildElement( field1 );

  QgsAttributeEditorField *field2 = new QgsAttributeEditorField( "f2", 1, nullptr );

  QFont font2 { QgsFontUtils::getStandardTestFont() };

  field2->setLabelStyle(
  {
    QColor( Qt::GlobalColor::blue ),
    font2,
    false,
    true } );

  QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( "group1", nullptr );
  container->setLabelStyle(
  {
    QColor( Qt::GlobalColor::darkCyan ),
    font0,
    true,
    true
  } );

  container->addChildElement( field2 );
  editFormConfig.addTab( container );
  editFormConfig.setLayout( QgsEditFormConfig::TabLayout );

  QDomDocument doc;
  QDomNode node = doc.createElement( "config" );
  QgsReadWriteContext ctx;
  editFormConfig.writeXml( node, ctx );

  QgsEditFormConfig config;
  config.readXml( node, ctx );

  const QgsAttributeEditorElement *field1config { config.tabs()[0] };
  QCOMPARE( field1config->name(), QString( "f1" ) );
  QCOMPARE( field1config->labelStyle(), field1->labelStyle() );

  const auto group1config { static_cast<QgsAttributeEditorContainer *>( config.tabs()[1] ) };
  QCOMPARE( group1config->name(), QString( "group1" ) );
  QCOMPARE( group1config->labelStyle(), container->labelStyle() );

  const auto field2config { group1config->children().at( 0 ) };
  QCOMPARE( field2config->name(), QString( "f2" ) );
  QCOMPARE( field2config->labelStyle(), field2->labelStyle() );

}

QGSTEST_MAIN( TestQgsAttributeEditorElement )
#include "testqgsattributeeditorelement.moc"
