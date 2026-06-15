/***************************************************************************
  testqgsqgisprofileimporter.cpp
  --------------------------------
  begin                : June 2026
 ***************************************************************************/

#include "qgsqgisprofileimporter.h"
#include "qgstest.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

class TestQgsQgisProfileImporter : public QObject
{
    Q_OBJECT

  private slots:
    void detectsSiblingAndLegacyProfiles();
    void importsFullProfile();
    void manualImportUsesUniqueProfileName();
    void firstRunOfferLogic();

  private:
    static QString configRoot( const QString &basePath );
    static QString targetProfilesRoot( const QString &basePath );
    static QString sourceProfilesRoot( const QString &basePath );
    static QString sourceProfilePath( const QString &basePath, const QString &profileName );
    static bool writeFile( const QString &path, const QByteArray &content );
    static void createProfile( const QString &basePath, const QString &profileName, int pluginCount = 0 );
};

QString TestQgsQgisProfileImporter::configRoot( const QString &basePath )
{
  return QDir( basePath ).filePath( u"QGIS/QGIS4"_s );
}

QString TestQgsQgisProfileImporter::targetProfilesRoot( const QString &basePath )
{
  return QDir( configRoot( basePath ) ).filePath( u"profiles"_s );
}

QString TestQgsQgisProfileImporter::sourceProfilesRoot( const QString &basePath )
{
  return QDir( basePath ).filePath( u"QGIS/QGIS3/profiles"_s );
}

QString TestQgsQgisProfileImporter::sourceProfilePath( const QString &basePath, const QString &profileName )
{
  return QDir( sourceProfilesRoot( basePath ) ).filePath( profileName );
}

bool TestQgsQgisProfileImporter::writeFile( const QString &path, const QByteArray &content )
{
  QFileInfo fileInfo( path );
  if ( !QDir().mkpath( fileInfo.absolutePath() ) )
    return false;

  QFile file( path );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;

  file.write( content );
  file.close();
  return true;
}

void TestQgsQgisProfileImporter::createProfile( const QString &basePath, const QString &profileName, int pluginCount )
{
  const QString profilePath = sourceProfilePath( basePath, profileName );
  QVERIFY( writeFile( QDir( profilePath ).filePath( u"qgis.db"_s ), "db" ) );
  QVERIFY( writeFile( QDir( profilePath ).filePath( u"QGIS/QGIS3.ini"_s ), "[qgis]\n" ) );

  for ( int i = 0; i < pluginCount; ++i )
  {
    QVERIFY( writeFile( QDir( profilePath ).filePath( u"python/plugins/plugin%1/metadata.txt"_s.arg( i + 1 ) ), "name=plugin" ) );
  }
}

void TestQgsQgisProfileImporter::detectsSiblingAndLegacyProfiles()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  createProfile( tempDir.path(), u"default"_s, 2 );
  createProfile( tempDir.path(), u"field"_s, 0 );
  QVERIFY( QDir().mkpath( QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"strata"_s ) ) );

  QList<QgsQgisProfileImporter::Candidate> candidates = QgsQgisProfileImporter::detectCandidates( configRoot( tempDir.path() ), targetProfilesRoot( tempDir.path() ) );
  QCOMPARE( candidates.size(), 2 );
  QCOMPARE( candidates.at( 0 ).profileName, u"default"_s );
  QCOMPARE( candidates.at( 0 ).sourceVersionLabel, u"QGIS 3"_s );
  QCOMPARE( candidates.at( 0 ).pluginCount, 2 );
  QCOMPARE( candidates.at( 1 ).profileName, u"field"_s );

  const QByteArray originalHome = qgetenv( "HOME" );
  QTemporaryDir homeDir;
  QVERIFY( homeDir.isValid() );
  qputenv( "HOME", homeDir.path().toUtf8() );
  const auto restoreHome = qScopeGuard( [originalHome] { qputenv( "HOME", originalHome ); } );
  QVERIFY( writeFile( QDir( homeDir.path() ).filePath( u".qgis3/profiles/legacy/qgis.db"_s ), "legacy" ) );

  QTemporaryDir legacyTempDir;
  QVERIFY( legacyTempDir.isValid() );
  candidates = QgsQgisProfileImporter::detectCandidates( configRoot( legacyTempDir.path() ), targetProfilesRoot( legacyTempDir.path() ) );

  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.constFirst().profileName, u"legacy"_s );
  QCOMPARE( candidates.constFirst().sourceVersionLabel, u"QGIS 3 legacy"_s );
}

void TestQgsQgisProfileImporter::importsFullProfile()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  createProfile( tempDir.path(), u"default"_s, 1 );
  createProfile( tempDir.path(), u"field"_s, 0 );

  QSettings sourceProfilesSettings( QDir( sourceProfilesRoot( tempDir.path() ) ).filePath( u"profiles.ini"_s ), QSettings::IniFormat );
  sourceProfilesSettings.setValue( u"/core/defaultProfile"_s, u"default"_s );
  sourceProfilesSettings.setValue( u"/core/lastProfile"_s, u"field"_s );
  sourceProfilesSettings.setValue( u"/core/selectionPolicy"_s, 0 );
  sourceProfilesSettings.sync();

  const QString defaultSourceProfile = sourceProfilePath( tempDir.path(), u"default"_s );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"symbology-style.db"_s ), "style" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"qgis-auth.db"_s ), "auth" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"bookmarks.xml"_s ), "<bookmarks/>" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"processing/scripts/script.py"_s ), "print('ok')" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"project_templates/template.qgs"_s ), "<qgis/>" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"svg/icon.svg"_s ), "<svg/>" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"palettes/colors.gpl"_s ), "GIMP Palette" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"fonts/font.ttf"_s ), "font" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"themes/theme/style.qss"_s ), "QWidget {}" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"python/plugins/plugin1/__pycache__/cached.pyc"_s ), "pyc" ) );
  QVERIFY( writeFile( QDir( defaultSourceProfile ).filePath( u"previewImages/old.png"_s ), "preview" ) );

  const QString sourceSettingsPath = QDir( defaultSourceProfile ).filePath( u"QGIS/QGIS3.ini"_s );
  QSettings sourceSettings( sourceSettingsPath, QSettings::IniFormat );
  sourceSettings.setValue( u"PythonPlugins/plugin1"_s, true );
  sourceSettings.setValue( u"PythonPlugins/watchDogTimestamp/plugin1"_s, 123 );
  sourceSettings.setValue( u"paths/profile"_s, defaultSourceProfile );
  sourceSettings.sync();

  const QList<QgsQgisProfileImporter::Candidate> candidates = QgsQgisProfileImporter::detectCandidates( configRoot( tempDir.path() ), targetProfilesRoot( tempDir.path() ) );
  const QgsQgisProfileImporter::ImportResult result = QgsQgisProfileImporter::importProfiles( candidates, targetProfilesRoot( tempDir.path() ) );
  QVERIFY2( result.errors.isEmpty(), result.errors.message( QgsErrorMessage::Text ).toUtf8().constData() );
  QCOMPARE( result.importedProfileNames.size(), 2 );
  QCOMPARE( result.activeProfileName, u"field"_s );

  const QString importedDefaultProfile = QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"default"_s );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"python/plugins/plugin1/metadata.txt"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"qgis-auth.db"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"symbology-style.db"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"processing/scripts/script.py"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"project_templates/template.qgs"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"svg/icon.svg"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"palettes/colors.gpl"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"fonts/font.ttf"_s ) ) );
  QVERIFY( QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"themes/theme/style.qss"_s ) ) );
  QVERIFY( !QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"python/plugins/plugin1/__pycache__/cached.pyc"_s ) ) );
  QVERIFY( !QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"previewImages/old.png"_s ) ) );
  QVERIFY( !QFileInfo::exists( QDir( importedDefaultProfile ).filePath( u"QGIS/QGIS3.ini"_s ) ) );

  const QString targetSettingsPath = QDir( importedDefaultProfile ).filePath( u"QGIS/%1.ini"_s.arg( QCoreApplication::applicationName() ) );
  QVERIFY( QFileInfo::exists( targetSettingsPath ) );
  QSettings targetSettings( targetSettingsPath, QSettings::IniFormat );
  QCOMPARE( targetSettings.value( u"PythonPlugins/plugin1"_s ).toBool(), true );
  QVERIFY( !targetSettings.contains( u"PythonPlugins/watchDogTimestamp/plugin1"_s ) );
  QCOMPARE( targetSettings.value( u"migration/migrated_from_3"_s ).toBool(), true );
  QCOMPARE( targetSettings.value( u"strata/qgisImport/imported"_s ).toBool(), true );
  QCOMPARE( targetSettings.value( u"strata/qgisImport/sourceProfileName"_s ).toString(), u"default"_s );
  QCOMPARE( targetSettings.value( u"paths/profile"_s ).toString(), importedDefaultProfile );

  QSettings targetProfilesSettings( QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"profiles.ini"_s ), QSettings::IniFormat );
  QCOMPARE( targetProfilesSettings.value( u"/core/defaultProfile"_s ).toString(), u"default"_s );
  QCOMPARE( targetProfilesSettings.value( u"/core/lastProfile"_s ).toString(), u"field"_s );
  QCOMPARE( targetProfilesSettings.value( u"strata/qgisImport/decisionMade"_s ).toBool(), true );
}

void TestQgsQgisProfileImporter::manualImportUsesUniqueProfileName()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  createProfile( tempDir.path(), u"default"_s, 0 );
  QVERIFY( QDir().mkpath( QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"default"_s ) ) );

  const QList<QgsQgisProfileImporter::Candidate> candidates = QgsQgisProfileImporter::detectCandidates( configRoot( tempDir.path() ), targetProfilesRoot( tempDir.path() ) );
  QCOMPARE( candidates.size(), 1 );

  const QgsQgisProfileImporter::ImportResult result = QgsQgisProfileImporter::importProfileAsNewProfile( candidates.constFirst(), targetProfilesRoot( tempDir.path() ) );
  QVERIFY2( result.errors.isEmpty(), result.errors.message( QgsErrorMessage::Text ).toUtf8().constData() );
  QCOMPARE( result.activeProfileName, u"default-qgis"_s );
  QVERIFY( QFileInfo::exists( QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"default-qgis/qgis.db"_s ) ) );
}

void TestQgsQgisProfileImporter::firstRunOfferLogic()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  createProfile( tempDir.path(), u"default"_s, 0 );
  const QList<QgsQgisProfileImporter::Candidate> candidates = QgsQgisProfileImporter::detectCandidates( configRoot( tempDir.path() ), targetProfilesRoot( tempDir.path() ) );
  QVERIFY( QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( tempDir.path() ), candidates, true, false, false ) );
  QVERIFY( !QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( tempDir.path() ), candidates, false, false, false ) );
  QVERIFY( !QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( tempDir.path() ), candidates, true, true, false ) );
  QVERIFY( !QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( tempDir.path() ), candidates, true, false, true ) );

  QVERIFY( QDir().mkpath( QDir( targetProfilesRoot( tempDir.path() ) ).filePath( u"default"_s ) ) );
  QVERIFY( !QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( tempDir.path() ), candidates, true, false, false ) );

  QTemporaryDir declinedTempDir;
  QVERIFY( declinedTempDir.isValid() );
  createProfile( declinedTempDir.path(), u"default"_s, 0 );
  const QList<QgsQgisProfileImporter::Candidate> declinedCandidates = QgsQgisProfileImporter::detectCandidates( configRoot( declinedTempDir.path() ), targetProfilesRoot( declinedTempDir.path() ) );
  QgsQgisProfileImporter::markFirstRunImportDeclined( targetProfilesRoot( declinedTempDir.path() ) );
  QVERIFY( !QgsQgisProfileImporter::shouldOfferFirstRunImport( targetProfilesRoot( declinedTempDir.path() ), declinedCandidates, true, false, false ) );
}

QGSTEST_MAIN( TestQgsQgisProfileImporter )
#include "testqgsqgisprofileimporter.moc"
