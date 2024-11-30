/***************************************************************************
    testqgsadvanceddigitizingtoolsregistry.cpp
     --------------------
    Date                 : July 2024
    Copyright            : (C) 2024 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsadvanceddigitizingtools.h"
#include "qgsadvanceddigitizingtoolsregistry.h"
#include <QSignalSpy>

class TestQgsAdvancedDigitizingToolsRegistry : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void guiRegistry();

  private:
};

void TestQgsAdvancedDigitizingToolsRegistry::initTestCase()
{
}

void TestQgsAdvancedDigitizingToolsRegistry::cleanupTestCase()
{
}

void TestQgsAdvancedDigitizingToolsRegistry::init()
{
}

void TestQgsAdvancedDigitizingToolsRegistry::cleanup()
{
}

class DummyAdvancedDigitizingTool : public QgsAdvancedDigitizingTool
{
    Q_OBJECT
  public:
    DummyAdvancedDigitizingTool()
      : QgsAdvancedDigitizingTool( nullptr, nullptr )
    {}
};

void TestQgsAdvancedDigitizingToolsRegistry::guiRegistry()
{
  // test QgsAnnotationItemGuiRegistry
  QgsAdvancedDigitizingToolsRegistry registry;

  // empty registry
  QVERIFY( !registry.toolMetadata( QString( "empty" ) ) );
  QVERIFY( registry.toolMetadataNames().isEmpty() );

  auto createTool = []( QgsMapCanvas *, QgsAdvancedDigitizingDockWidget * ) -> QgsAdvancedDigitizingTool * {
    return new DummyAdvancedDigitizingTool();
  };

  QgsAdvancedDigitizingToolMetadata *metadata = new QgsAdvancedDigitizingToolMetadata( QStringLiteral( "dummy" ), QStringLiteral( "My Dummy Tool" ), QIcon(), createTool );
  QVERIFY( registry.addTool( metadata ) );
  const QString name = registry.toolMetadataNames().value( 0 );
  QCOMPARE( name, QStringLiteral( "dummy" ) );

  // duplicate name not allowed
  metadata = new QgsAdvancedDigitizingToolMetadata( QStringLiteral( "dummy" ), QStringLiteral( "My Dummy Tool" ), QIcon(), createTool );
  QVERIFY( !registry.addTool( metadata ) );

  QVERIFY( registry.toolMetadata( name ) );
  QCOMPARE( registry.toolMetadata( name )->visibleName(), QStringLiteral( "My Dummy Tool" ) );

  QgsAdvancedDigitizingTool *tool = registry.toolMetadata( name )->createTool( nullptr, nullptr );
  QVERIFY( tool );

  registry.removeTool( name );
  QVERIFY( registry.toolMetadataNames().isEmpty() );
}


QGSTEST_MAIN( TestQgsAdvancedDigitizingToolsRegistry )
#include "testqgsadvanceddigitizingtoolsregistry.moc"
