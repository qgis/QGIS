/***************************************************************************
    testqgslabelingwidget.cpp
    ---------------------
    begin                : 2025/02/06
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgslabelingwidget.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerreference.h"
#include "qgsrulebasedlabeling.h"

class TestQgsLabelingWidget : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLabelingWidget()
      : QgsTest( QStringLiteral( "Labeling Widget Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testRuleKeyPreserved();
};

void TestQgsLabelingWidget::initTestCase()
{
}

void TestQgsLabelingWidget::cleanupTestCase()
{
}

void TestQgsLabelingWidget::init()
{
}

void TestQgsLabelingWidget::cleanup()
{
}

void TestQgsLabelingWidget::testRuleKeyPreserved()
{
  // test that rule keys are preserved and not reset when editing labels with a rule based rendering
  QgsVectorLayer layer( QStringLiteral( "Point?field=pk:int" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );

  QgsFeature ft1( layer.fields() );
  ft1.setAttribute( QStringLiteral( "pk" ), 1 );
  ft1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 1 1 )" ) ) );
  layer.dataProvider()->addFeature( ft1 );

  QgsFeature ft2( layer.fields() );
  ft2.setAttribute( QStringLiteral( "pk" ), 2 );
  ft2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 2 2 )" ) ) );
  layer.dataProvider()->addFeature( ft2 );

  auto label_settings = std::make_unique<QgsPalLayerSettings>();
  label_settings->fieldName = QStringLiteral( "pk" );

  QgsTextMaskSettings mask;
  mask.setEnabled( true );
  mask.setMaskedSymbolLayers( QList<QgsSymbolLayerReference>() << QgsSymbolLayerReference( layer.id(), QStringLiteral( "test_unique_id" ) ) );

  QgsTextFormat text_format = label_settings->format();
  text_format.setMask( mask );
  label_settings->setFormat( text_format );

  auto root = std::make_unique<QgsRuleBasedLabeling::Rule>( new QgsPalLayerSettings() );

  auto rule = std::make_unique<QgsRuleBasedLabeling::Rule>( label_settings.release() );
  rule->setDescription( "test rule" );
  rule->setFilterExpression( QStringLiteral( "\"{pk}\" % 2 = 0" ) );
  rule->setActive( true );

  const QString rootRuleKey = root->ruleKey();
  const QString ruleKey = rule->ruleKey();

  root->appendChild( rule.release() );

  auto ruleSettings = std::make_unique<QgsRuleBasedLabeling>( root.release() );
  layer.setLabelsEnabled( true );
  layer.setLabeling( ruleSettings.release() );

  QgsLabelingWidget widget( &layer, nullptr );
  widget.writeSettingsToLayer();

  QgsRuleBasedLabeling *labelingAfter = dynamic_cast<QgsRuleBasedLabeling *>( layer.labeling() );
  QVERIFY( labelingAfter );
  QCOMPARE( labelingAfter->rootRule()->ruleKey(), rootRuleKey );
  QCOMPARE( labelingAfter->rootRule()->children().count(), 1 );
  QCOMPARE( labelingAfter->rootRule()->children().at( 0 )->ruleKey(), ruleKey );
}

QGSTEST_MAIN( TestQgsLabelingWidget )
#include "testqgslabelingwidget.moc"
