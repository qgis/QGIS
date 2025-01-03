/***************************************************************************
     testqgsprojectexpressions.cpp
     --------------------
    Date                 : Aug 2024
    Copyright            : (C) 2024 by Germán Carrillo
    Email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include "qgstest.h"

#include <qgisapp.h>
#include <qgsapplication.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the Save Python Expression in project support.
 */
class TestQgsProjectExpressions : public QObject
{
    Q_OBJECT

  public:
    TestQgsProjectExpressions();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void projectExpressions();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsProjectExpressions::TestQgsProjectExpressions() = default;

//runs before all tests
void TestQgsProjectExpressions::initTestCase()
{
  const QByteArray configPath = QByteArray( TEST_DATA_DIR ) + "/test_qgis_config_path";
  qputenv( "QGIS_CUSTOM_CONFIG_PATH", configPath );

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  qDebug() << "TestQgsProjectExpressions::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
  mQgisApp->loadPythonSupport();
}

//runs after all tests
void TestQgsProjectExpressions::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProjectExpressions::projectExpressions()
{
  const int count_before_project = QgsExpression::functionCount();
  QVERIFY( QgsExpression::functionIndex( QStringLiteral( "mychoice" ) ) != -1 ); // User expression loaded

  const QString myExpression = QStringLiteral( "mychoice(1, 2)" );
  QgsExpression exp( QStringLiteral( "mychoice(1, 2)" ) );
  QCOMPARE( exp.evaluate().toInt(), 1 );

  // Load expressions from project
  // Project registers 2 functions: mychoice (overwriting it) and myprojectfunction
  const QByteArray projectPath = QByteArray( TEST_DATA_DIR ) + "/projects/test_project_functions.qgz";

  const Qgis::PythonEmbeddedMode pythonEmbeddedMode = QgsSettings().enumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), Qgis::PythonEmbeddedMode::Ask );
  QgsSettings().setEnumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), Qgis::PythonEmbeddedMode::Never );
  QgsProject::instance()->read( projectPath );
  QCOMPARE( QgsExpression::functionIndex( QStringLiteral( "myprojectfunction" ) ), -1 );

  // Set the global setting to accept expression functions
  QgsSettings().setEnumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), Qgis::PythonEmbeddedMode::SessionOnly );
  QgsProject::instance()->loadFunctionsFromProject();
  QgsSettings().setEnumValue( QStringLiteral( "qgis/enablePythonEmbedded" ), pythonEmbeddedMode );

  QVERIFY( QgsExpression::functionIndex( QStringLiteral( "myprojectfunction" ) ) != -1 );
  QVERIFY( QgsExpression::functionIndex( QStringLiteral( "mychoice" ) ) != -1 ); // Overwritten function
  const int count_project_loaded = QgsExpression::functionCount();

  QCOMPARE( count_project_loaded - count_before_project, 1 ); // myprojectfunction

  exp = myExpression;                    // Re-parse it
  QCOMPARE( exp.evaluate().toInt(), 2 ); // Different result because now it's from project

  // Unload expressions from project, reload user ones
  QgsProject::instance()->cleanFunctionsFromProject();
  const int count_project_unloaded = QgsExpression::functionCount();
  QCOMPARE( count_before_project, count_project_unloaded ); // myprojectfunction is gone

  QCOMPARE( QgsExpression::functionIndex( QStringLiteral( "myprojectfunction" ) ), -1 );
  exp = myExpression;                    // Re-parse it
  QCOMPARE( exp.evaluate().toInt(), 1 ); // Original result, coming from user function
}


QGSTEST_MAIN( TestQgsProjectExpressions )
#include "testqgsprojectexpressions.moc"
