/***************************************************************************
     testqgsrelation.cpp
     --------------------------------------
    Date                 : 8.03.2022
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


#include "qgsproject.h"
#include "qgsattributeeditorelement.h"
#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorrelation.h"
#include "qgsmaplayerstylemanager.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRelation changing style
 */
class TestQgsRelation: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testValidRelationAfterChangingStyle();
};

void TestQgsRelation::initTestCase()
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


void TestQgsRelation::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}

void TestQgsRelation::testValidRelationAfterChangingStyle()
{
  const QString projectPath = QStringLiteral( TEST_DATA_DIR ) + "/relations.qgs";

  QgsProject *p = QgsProject::instance();

  QVERIFY( p->read( projectPath ) );

  const auto layers { p->mapLayers().values( ) };
  for ( const auto &l : std::as_const( layers ) )
  {
    QVERIFY( l->isValid() );
  }

  auto relations = p->relationManager()->relations();
  auto relation = relations.first();
  QVERIFY( relation.isValid() );
  auto referencedLayer = relation.referencedLayer();

  bool valid { false };
  QCOMPARE( static_cast<QgsAttributeEditorContainer *>( referencedLayer->editFormConfig().tabs()[0] )->children().count(), 7 );
  auto tabs { static_cast<QgsAttributeEditorContainer *>( referencedLayer->editFormConfig().tabs()[0] )->children() };
  for ( const auto &tab : tabs )
  {
    if ( tab->type() == QgsAttributeEditorElement::AeTypeRelation )
    {
      valid = static_cast<QgsAttributeEditorRelation *>( tab )->relation().isValid();
    }
  }

  QVERIFY( valid );

  QVERIFY( referencedLayer->styleManager()->setCurrentStyle( "custom" ) );

  valid = false;
  QCOMPARE( static_cast<QgsAttributeEditorContainer *>( referencedLayer->editFormConfig().tabs()[0] )->children().count(), 7 );
  tabs = static_cast<QgsAttributeEditorContainer *>( referencedLayer->editFormConfig().tabs()[0] )->children();
  for ( const auto &tab : std::as_const( tabs ) )
  {
    if ( tab->type() == QgsAttributeEditorElement::AeTypeRelation )
    {
      valid = static_cast<QgsAttributeEditorRelation *>( tab )->relation().isValid();
    }
  }

  QVERIFY( valid );

}

QGSTEST_MAIN( TestQgsRelation )
#include "testqgsrelation.moc"
