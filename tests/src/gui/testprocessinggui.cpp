/***************************************************************************
                         testprocesinggui.cpp
                         ---------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>
#include "qgstest.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsnativealgorithms.h"
#include "qgsxmlutils.h"

class TestProcessingGui : public QObject
{
    Q_OBJECT
  public:
    TestProcessingGui() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testSetGetConfig();
    void testFilterAlgorithmConfig();
};

void TestProcessingGui::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestProcessingGui::cleanupTestCase()
{
  QgsApplication::exitQgis();
}
void TestProcessingGui::init()
{

}

void TestProcessingGui::cleanup()
{

}

void TestProcessingGui::testSetGetConfig()
{
  const QList< const QgsProcessingAlgorithm * > algorithms = QgsApplication::instance()->processingRegistry()->algorithms();

  // Find all defined widgets for native algorithms
  // and get the default configuration (that is, we create a widget
  // and get the configuration it returns without being modified in any way)
  // We then set and get this configuration and validate that it matches the original one.
  for ( const QgsProcessingAlgorithm *algorithm : algorithms )
  {
    std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::instance()->processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );

    if ( configWidget )
    {
      const QVariantMap defaultConfig = configWidget->configuration();
      configWidget->setConfiguration( defaultConfig );
      const QVariantMap defaultControlConfig = configWidget->configuration();
      QDomDocument defaultConfigDoc;
      QDomDocument defaultConfigControlDoc;
      QgsXmlUtils::writeVariant( defaultConfig, defaultConfigDoc );
      QgsXmlUtils::writeVariant( defaultControlConfig, defaultConfigControlDoc );
      QCOMPARE( defaultConfigDoc.toString(), defaultConfigControlDoc.toString() );
    }
  }
}

void TestProcessingGui::testFilterAlgorithmConfig()
{
  const QgsProcessingAlgorithm *algorithm = QgsApplication::instance()->processingRegistry()->algorithmById( QStringLiteral( "native:filter" ) );
  std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> configWidget( QgsGui::instance()->processingGuiRegistry()->algorithmConfigurationWidget( algorithm ) );

  QVariantMap config;
  QVariantList outputs;
  QVariantMap output;
  output.insert( QStringLiteral( "name" ), QStringLiteral( "test" ) );
  output.insert( QStringLiteral( "expression" ), QStringLiteral( "I am an expression" ) );
  output.insert( QStringLiteral( "isModelOutput" ), true );
  outputs.append( output );
  config.insert( QStringLiteral( "outputs" ), outputs );
  configWidget->setConfiguration( config );

  QVariantMap configControl = configWidget->configuration();

  QDomDocument configDoc;
  QDomDocument configControlDoc;
  QgsXmlUtils::writeVariant( config, configDoc );
  QgsXmlUtils::writeVariant( configControl, configControlDoc );
  QCOMPARE( configDoc.toString(), configControlDoc.toString() );
}

QGSTEST_MAIN( TestProcessingGui )
#include "testprocessinggui.moc"
