/***************************************************************************
  testqgsaigissuggestionengine.cpp
  --------------------------------
  begin                : July 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/qgsaigissuggestionengine.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QCryptographicHash>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAiGisSuggestionEngine : public QObject
{
    Q_OBJECT

  private slots:
    void init();
    void cleanup();
    void emptyProjectSuggestsDataLoading();
    void flagsMissingCrsAndEmptyVector();
    void flagsDuplicateFields();
    void formatHealthBlockCapsAndFullVariant();
    void promptBlockRespectsGlobalToggle();
    void settingsKeysAreStable();

  private:
    static QList<QgsAiGisSuggestion> idsToSuggestions( int count );
    static bool containsId( const QList<QgsAiGisSuggestion> &suggestions, const QString &idPrefix, QgsAiGisSuggestion *match = nullptr );
};

void TestQgsAiGisSuggestionEngine::init()
{
  QgsProject::instance()->clear();
}

void TestQgsAiGisSuggestionEngine::cleanup()
{
  QgsProject::instance()->clear();
}

QList<QgsAiGisSuggestion> TestQgsAiGisSuggestionEngine::idsToSuggestions( int count )
{
  QList<QgsAiGisSuggestion> suggestions;
  for ( int i = 0; i < count; ++i )
  {
    QgsAiGisSuggestion suggestion;
    suggestion.id = u"synthetic-%1"_s.arg( i );
    suggestion.title = u"Title %1"_s.arg( i );
    suggestion.detail = u"Detail %1"_s.arg( i );
    suggestion.actionPrompt = u"Action prompt %1"_s.arg( i );
    suggestion.risk = u"medium"_s;
    suggestions << suggestion;
  }
  return suggestions;
}

bool TestQgsAiGisSuggestionEngine::containsId( const QList<QgsAiGisSuggestion> &suggestions, const QString &idPrefix, QgsAiGisSuggestion *match )
{
  for ( const QgsAiGisSuggestion &suggestion : suggestions )
  {
    if ( suggestion.id.startsWith( idPrefix ) )
    {
      if ( match )
        *match = suggestion;
      return true;
    }
  }
  return false;
}

void TestQgsAiGisSuggestionEngine::emptyProjectSuggestsDataLoading()
{
  const QList<QgsAiGisSuggestion> suggestions = QgsAiGisSuggestionEngine::suggestionsForProject( QgsProject::instance() );
  QCOMPARE( suggestions.size(), 1 );
  QCOMPARE( suggestions.first().id, u"project-empty"_s );
  QCOMPARE( suggestions.first().risk, u"low"_s );

  QVERIFY( QgsAiGisSuggestionEngine::suggestionsForProject( nullptr ).isEmpty() );
}

void TestQgsAiGisSuggestionEngine::flagsMissingCrsAndEmptyVector()
{
  QgsVectorLayer *noCrsLayer = new QgsVectorLayer( u"Point?field=name:string"_s, u"No CRS points"_s, u"memory"_s );
  QVERIFY( noCrsLayer->isValid() );
  noCrsLayer->setCrs( QgsCoordinateReferenceSystem() );
  QVERIFY( !noCrsLayer->crs().isValid() );
  QgsProject::instance()->addMapLayer( noCrsLayer );

  QgsVectorLayer *emptyLayer = new QgsVectorLayer( u"Point?field=name:string&crs=EPSG:4326"_s, u"Empty points"_s, u"memory"_s );
  QVERIFY( emptyLayer->isValid() );
  QgsProject::instance()->addMapLayer( emptyLayer );

  const QList<QgsAiGisSuggestion> suggestions = QgsAiGisSuggestionEngine::suggestionsForProject( QgsProject::instance() );

  QgsAiGisSuggestion missingCrs;
  QVERIFY( containsId( suggestions, u"missing-crs:%1"_s.arg( noCrsLayer->id() ), &missingCrs ) );
  QCOMPARE( missingCrs.risk, u"medium"_s );
  QVERIFY( missingCrs.title.contains( u"No CRS points"_s ) );
  QVERIFY( !missingCrs.actionPrompt.isEmpty() );

  QgsAiGisSuggestion emptyVector;
  QVERIFY( containsId( suggestions, u"empty-vector:%1"_s.arg( emptyLayer->id() ), &emptyVector ) );
  QCOMPARE( emptyVector.risk, u"low"_s );
}

void TestQgsAiGisSuggestionEngine::flagsDuplicateFields()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=name:string&field=NAME:string&crs=EPSG:4326"_s, u"Duplicate fields"_s, u"memory"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer );

  const QList<QgsAiGisSuggestion> suggestions = QgsAiGisSuggestionEngine::suggestionsForProject( QgsProject::instance() );

  QgsAiGisSuggestion fields;
  QVERIFY( containsId( suggestions, u"fields:%1"_s.arg( layer->id() ), &fields ) );
  QCOMPARE( fields.risk, u"medium"_s );
}

void TestQgsAiGisSuggestionEngine::formatHealthBlockCapsAndFullVariant()
{
  QVERIFY( QgsAiGisSuggestionEngine::formatHealthBlock( {}, false ).isEmpty() );

  const QList<QgsAiGisSuggestion> suggestions = idsToSuggestions( 15 );

  const QString compact = QgsAiGisSuggestionEngine::formatHealthBlock( suggestions, false );
  QVERIFY( compact.startsWith( "## Current project GIS health"_L1 ) );
  QCOMPARE( compact.count( u"- [medium]"_s ), 10 );
  QVERIFY( compact.contains( u"…5 more."_s ) );
  QVERIFY( !compact.contains( u"Suggested action:"_s ) );

  const QString full = QgsAiGisSuggestionEngine::formatHealthBlock( suggestions, true );
  QCOMPARE( full.count( u"- [medium]"_s ), 10 );
  QCOMPARE( full.count( u"Suggested action:"_s ), 10 );
  QVERIFY( full.contains( u"Action prompt 0"_s ) );
}

void TestQgsAiGisSuggestionEngine::promptBlockRespectsGlobalToggle()
{
  QgsSettings settings;
  const QString globalKey = QgsAiGisSuggestionEngine::globalEnabledSettingsKey();
  const QVariant savedGlobal = settings.value( globalKey );

  settings.setValue( globalKey, false );
  QVERIFY( QgsAiGisSuggestionEngine::promptHealthBlockForProject( QgsProject::instance() ).isEmpty() );

  settings.setValue( globalKey, true );
  const QString block = QgsAiGisSuggestionEngine::promptHealthBlockForProject( QgsProject::instance() );
  QVERIFY( block.contains( u"## Current project GIS health"_s ) );
  QVERIFY( block.contains( u"No layers loaded"_s ) );

  if ( savedGlobal.isValid() )
    settings.setValue( globalKey, savedGlobal );
  else
    settings.remove( globalKey );
}

void TestQgsAiGisSuggestionEngine::settingsKeysAreStable()
{
  // Keys must stay compatible with the pre-engine "GIS tab" settings so user
  // preferences survive the refactoring.
  QCOMPARE( QgsAiGisSuggestionEngine::globalEnabledSettingsKey(), u"strata/gis_tab/enabled"_s );

  const QString unsavedHash = QString::fromLatin1( QCryptographicHash::hash( QByteArrayLiteral( "unsaved" ), QCryptographicHash::Sha1 ).toHex() );
  QCOMPARE( QgsAiGisSuggestionEngine::projectEnabledSettingsKey( QString() ), u"strata/gis_tab/project_enabled/%1"_s.arg( unsavedHash ) );
  QCOMPARE( QgsAiGisSuggestionEngine::dismissedSettingsKey( QString() ), u"strata/gis_tab/dismissed/%1"_s.arg( unsavedHash ) );
}

QGSTEST_MAIN( TestQgsAiGisSuggestionEngine )
#include "testqgsaigissuggestionengine.moc"
