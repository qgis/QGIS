/***************************************************************************
  testqgsairulesskillsstore.cpp
  ------------------------
  begin                : July 2026
***************************************************************************/

#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsairulesskillsstore.h"
#include "qgsapplication.h"
#include "qgstest.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

class TestQgsAiRulesSkillsStore : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void slugify();
    void listRulesParsesFrontmatter();
    void listRulesLegacyPlainFile();
    void ruleRawMarkdownRoundTrip();
    void ruleTitleDerivedFromMarkdown();
    void writeAndReadRuleRoundTrip();
    void updateRuleOverwritesInPlace();
    void deleteRuleRemovesFile();
    void writeRuleRejectsEmptySlug();

    void listSkillsParsesFrontmatter();
    void skillFrontmatterSupportsReferencesAndCustomProperties();
    void writeAndReadSkillRoundTrip();
    void deleteSkillRemovesFolder();

    void invalidPathIsRejectedForWrites();
    void missingWorkspaceIsRejected();

  private:
    QTemporaryDir *mWorkspaceDir = nullptr;
    QgsAiFileContextProvider *mContextProvider = nullptr;
    QgsAiRulesSkillsStore *mStore = nullptr;
};

void TestQgsAiRulesSkillsStore::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiRulesSkillsStore::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiRulesSkillsStore::init()
{
  mWorkspaceDir = new QTemporaryDir();
  QVERIFY( mWorkspaceDir->isValid() );
  mContextProvider = new QgsAiFileContextProvider( mWorkspaceDir->path() );
  mStore = new QgsAiRulesSkillsStore( mContextProvider );
}

void TestQgsAiRulesSkillsStore::cleanup()
{
  delete mStore;
  mStore = nullptr;
  delete mContextProvider;
  mContextProvider = nullptr;
  delete mWorkspaceDir;
  mWorkspaceDir = nullptr;
}

void TestQgsAiRulesSkillsStore::slugify()
{
  QCOMPARE( QgsAiRulesSkillsStore::slugify( u"My Great Rule"_s ), u"my-great-rule"_s );
  QCOMPARE( QgsAiRulesSkillsStore::slugify( u"  --Weird__Name!! "_s ), u"weird-name"_s );
  QCOMPARE( QgsAiRulesSkillsStore::slugify( QString() ), u"untitled"_s );
  QCOMPARE( QgsAiRulesSkillsStore::slugify( u"Ω invalid Ω"_s ), u"invalid"_s );
}

void TestQgsAiRulesSkillsStore::listRulesParsesFrontmatter()
{
  const QString rulesDir = QDir( mWorkspaceDir->path() ).filePath( u".strata/rules"_s );
  QVERIFY( QDir().mkpath( rulesDir ) );

  QFile file( QDir( rulesDir ).filePath( u"geo-style.md"_s ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( QByteArrayLiteral(
    "---\n"
    "description: Style conventions for symbology\n"
    "globs: *.qml, *.qgz\n"
    "alwaysApply: false\n"
    "enabled: true\n"
    "---\n"
    "Use colorblind-safe palettes.\n"
  ) );
  file.close();

  const QList<QgsAiRuleInfo> rules = mStore->listRules( u".strata/rules"_s );
  QCOMPARE( rules.size(), 1 );
  const QgsAiRuleInfo &rule = rules.first();
  QCOMPARE( rule.slug, u"geo-style"_s );
  QCOMPARE( rule.description, u"Style conventions for symbology"_s );
  QCOMPARE( rule.globs, QStringList( { u"*.qml"_s, u"*.qgz"_s } ) );
  QVERIFY( !rule.alwaysApply );
  QVERIFY( rule.enabled );
  QCOMPARE( mStore->readRuleBody( rule ), u"Use colorblind-safe palettes."_s );
}

void TestQgsAiRulesSkillsStore::listRulesLegacyPlainFile()
{
  const QString rulesDir = QDir( mWorkspaceDir->path() ).filePath( u".strata/rules"_s );
  QVERIFY( QDir().mkpath( rulesDir ) );

  QFile file( QDir( rulesDir ).filePath( u"legacy-notes.txt"_s ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( "Just plain legacy content, no frontmatter." );
  file.close();

  const QList<QgsAiRuleInfo> rules = mStore->listRules( u".strata/rules"_s );
  QCOMPARE( rules.size(), 1 );
  const QgsAiRuleInfo &rule = rules.first();
  QCOMPARE( rule.slug, u"legacy-notes"_s );
  // No frontmatter: name is derived from the first content line, and it defaults to always-applied.
  QCOMPARE( rule.name, u"Just plain legacy content, no frontmatter."_s );
  QVERIFY( rule.alwaysApply );
  QVERIFY( rule.enabled );
  QCOMPARE( mStore->readRuleBody( rule ), u"Just plain legacy content, no frontmatter."_s );
}

void TestQgsAiRulesSkillsStore::ruleRawMarkdownRoundTrip()
{
  const QString markdown = u"---\n"
                           "name: Raw Rule\n"
                           "description: Keep markdown exactly\n"
                           "globs:\n"
                           "  - *.qgz\n"
                           "  - *.gpkg\n"
                           "alwaysApply: false\n"
                           "enabled: false\n"
                           "---\n\n"
                           "# Raw Rule\n\n"
                           "Use project-safe paths only.\n"_s;

  QString error;
  QVERIFY2( mStore->writeRuleMarkdown( u".strata/rules"_s, u"raw-rule"_s, markdown, &error ), qPrintable( error ) );

  const QList<QgsAiRuleInfo> rules = mStore->listRules( u".strata/rules"_s );
  QCOMPARE( rules.size(), 1 );
  const QgsAiRuleInfo &rule = rules.first();
  QCOMPARE( rule.name, u"Raw Rule"_s );
  QCOMPARE( rule.description, u"Keep markdown exactly"_s );
  QCOMPARE( rule.globs, QStringList( { u"*.qgz"_s, u"*.gpkg"_s } ) );
  QVERIFY( !rule.alwaysApply );
  QVERIFY( !rule.enabled );
  QCOMPARE( mStore->readRuleMarkdown( rule ), markdown );
}

void TestQgsAiRulesSkillsStore::ruleTitleDerivedFromMarkdown()
{
  QCOMPARE( QgsAiRulesSkillsStore::titleFromMarkdown( u"---\nname: Frontmatter title\n---\n# Heading title\n"_s, u"fallback-title"_s ), u"Frontmatter title"_s );
  QCOMPARE( QgsAiRulesSkillsStore::titleFromMarkdown( u"\n# Heading title\n\nBody"_s, u"fallback-title"_s ), u"Heading title"_s );
  QCOMPARE( QgsAiRulesSkillsStore::titleFromMarkdown( u"\nFirst useful line\n\nBody"_s, u"fallback-title"_s ), u"First useful line"_s );
  QCOMPARE( QgsAiRulesSkillsStore::titleFromMarkdown( QString(), u"fallback-title"_s ), u"Fallback Title"_s );
}

void TestQgsAiRulesSkillsStore::writeAndReadRuleRoundTrip()
{
  QgsAiRuleInfo info;
  info.slug = u"new-rule"_s;
  info.name = u"New Rule"_s;
  info.description = u"A freshly created rule"_s;
  info.globs = { u"*.gpkg"_s };
  info.alwaysApply = true;
  info.enabled = true;

  QString error;
  QVERIFY2( mStore->writeRule( u".strata/rules"_s, info, u"Body content here."_s, &error ), qPrintable( error ) );

  const QList<QgsAiRuleInfo> rules = mStore->listRules( u".strata/rules"_s );
  QCOMPARE( rules.size(), 1 );
  QCOMPARE( rules.first().slug, u"new-rule"_s );
  QCOMPARE( rules.first().name, u"New Rule"_s );
  QCOMPARE( rules.first().description, u"A freshly created rule"_s );
  QCOMPARE( rules.first().globs, QStringList( { u"*.gpkg"_s } ) );
  QVERIFY( rules.first().alwaysApply );
  QCOMPARE( mStore->readRuleBody( rules.first() ), u"Body content here."_s );
}

void TestQgsAiRulesSkillsStore::updateRuleOverwritesInPlace()
{
  QgsAiRuleInfo info;
  info.slug = u"toggle-rule"_s;
  info.name = u"Toggle Rule"_s;
  info.alwaysApply = true;

  QString error;
  QVERIFY( mStore->writeRule( u".strata/rules"_s, info, u"v1"_s, &error ) );

  info.alwaysApply = false;
  info.description = u"Updated description"_s;
  QVERIFY( mStore->writeRule( u".strata/rules"_s, info, u"v2"_s, &error ) );

  const QList<QgsAiRuleInfo> rules = mStore->listRules( u".strata/rules"_s );
  QCOMPARE( rules.size(), 1 );
  QVERIFY( !rules.first().alwaysApply );
  QCOMPARE( rules.first().description, u"Updated description"_s );
  QCOMPARE( mStore->readRuleBody( rules.first() ), u"v2"_s );
}

void TestQgsAiRulesSkillsStore::deleteRuleRemovesFile()
{
  QgsAiRuleInfo info;
  info.slug = u"temp-rule"_s;
  info.name = u"Temp Rule"_s;

  QString error;
  QVERIFY( mStore->writeRule( u".strata/rules"_s, info, u"content"_s, &error ) );
  QCOMPARE( mStore->listRules( u".strata/rules"_s ).size(), 1 );

  const QgsAiRuleInfo rule = mStore->listRules( u".strata/rules"_s ).first();
  QVERIFY( mStore->deleteRule( rule, &error ) );
  QVERIFY( mStore->listRules( u".strata/rules"_s ).isEmpty() );
}

void TestQgsAiRulesSkillsStore::writeRuleRejectsEmptySlug()
{
  QgsAiRuleInfo info;
  info.name = u"No Slug"_s;

  QString error;
  QVERIFY( !mStore->writeRule( u".strata/rules"_s, info, u"content"_s, &error ) );
  QVERIFY( !error.isEmpty() );
}

void TestQgsAiRulesSkillsStore::listSkillsParsesFrontmatter()
{
  const QString skillDir = QDir( mWorkspaceDir->path() ).filePath( u".strata/skills/pdf-export"_s );
  QVERIFY( QDir().mkpath( skillDir ) );

  QFile file( QDir( skillDir ).filePath( u"SKILL.md"_s ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( QByteArrayLiteral(
    "---\n"
    "name: PDF export\n"
    "description: Use when the user asks for a print layout export\n"
    "---\n"
    "1. Open Layout Manager.\n"
    "2. Export as PDF.\n"
  ) );
  file.close();

  const QList<QgsAiSkillInfo> skills = mStore->listSkills( u".strata/skills"_s );
  QCOMPARE( skills.size(), 1 );
  const QgsAiSkillInfo &skill = skills.first();
  QCOMPARE( skill.slug, u"pdf-export"_s );
  QCOMPARE( skill.name, u"PDF export"_s );
  QCOMPARE( skill.description, u"Use when the user asks for a print layout export"_s );
  QVERIFY( skill.enabled );
  QVERIFY( mStore->readSkillBody( skill ).contains( u"Export as PDF."_s ) );
}

void TestQgsAiRulesSkillsStore::skillFrontmatterSupportsReferencesAndCustomProperties()
{
  const QString markdown = u"---\n"
                           "name: Agents SDK\n"
                           "description: Build stateful agents on Workers\n"
                           "references:\n"
                           "  - workers\n"
                           "  - agents-sdk\n"
                           "owner: platform\n"
                           "---\n\n"
                           "# Agents SDK\n\n"
                           "Prefer current Cloudflare docs.\n"_s;

  QString error;
  QVERIFY2( mStore->writeSkillMarkdown( u".strata/skills"_s, u"agents-sdk"_s, markdown, &error ), qPrintable( error ) );

  const QList<QgsAiSkillInfo> skills = mStore->listSkills( u".strata/skills"_s );
  QCOMPARE( skills.size(), 1 );
  const QgsAiSkillInfo &skill = skills.first();
  QCOMPARE( skill.slug, u"agents-sdk"_s );
  QCOMPARE( skill.name, u"Agents SDK"_s );
  QCOMPARE( skill.description, u"Build stateful agents on Workers"_s );

  const QgsAiMarkdownDocument document = QgsAiRulesSkillsStore::parseMarkdownDocument( mStore->readSkillMarkdown( skill ) );
  QCOMPARE( document.values( u"references"_s ), QStringList( { u"workers"_s, u"agents-sdk"_s } ) );
  QCOMPARE( document.value( u"owner"_s ), u"platform"_s );
  QCOMPARE( document.body, u"# Agents SDK\n\nPrefer current Cloudflare docs."_s );
}

void TestQgsAiRulesSkillsStore::writeAndReadSkillRoundTrip()
{
  QgsAiSkillInfo info;
  info.slug = u"data-cleanup"_s;
  info.name = u"Data cleanup"_s;
  info.description = u"Use when asked to clean attribute tables"_s;
  info.enabled = true;

  QString error;
  QVERIFY2( mStore->writeSkill( u".strata/skills"_s, info, u"Step 1. Step 2."_s, &error ), qPrintable( error ) );

  const QList<QgsAiSkillInfo> skills = mStore->listSkills( u".strata/skills"_s );
  QCOMPARE( skills.size(), 1 );
  QCOMPARE( skills.first().slug, u"data-cleanup"_s );
  QCOMPARE( skills.first().name, u"Data cleanup"_s );
  QCOMPARE( mStore->readSkillBody( skills.first() ), u"Step 1. Step 2."_s );
}

void TestQgsAiRulesSkillsStore::deleteSkillRemovesFolder()
{
  QgsAiSkillInfo info;
  info.slug = u"temp-skill"_s;
  info.name = u"Temp Skill"_s;
  info.description = u"Temporary"_s;

  QString error;
  QVERIFY( mStore->writeSkill( u".strata/skills"_s, info, u"content"_s, &error ) );
  const QgsAiSkillInfo skill = mStore->listSkills( u".strata/skills"_s ).first();

  QVERIFY( mStore->deleteSkill( skill, &error ) );
  QVERIFY( mStore->listSkills( u".strata/skills"_s ).isEmpty() );
  QVERIFY( !QDir( skill.folderPath ).exists() );
}

void TestQgsAiRulesSkillsStore::invalidPathIsRejectedForWrites()
{
  QgsAiRuleInfo info;
  // ".strata/rules" is two levels below the workspace root, so ".." x2 would only
  // cancel back down to the root; go up a third level to actually escape it.
  info.slug = u"../../../etc/escape"_s;
  info.name = u"Escape attempt"_s;

  QString error;
  QVERIFY( !mStore->writeRule( u".strata/rules"_s, info, u"content"_s, &error ) );
  QVERIFY( !error.isEmpty() );
}

void TestQgsAiRulesSkillsStore::missingWorkspaceIsRejected()
{
  QgsAiFileContextProvider noWorkspaceProvider { QString() };
  QgsAiRulesSkillsStore store( &noWorkspaceProvider );

  QVERIFY( store.listRules( u".strata/rules"_s ).isEmpty() );
  QVERIFY( store.listSkills( u".strata/skills"_s ).isEmpty() );

  QgsAiRuleInfo info;
  info.slug = u"any"_s;
  QString error;
  QVERIFY( !store.writeRule( u".strata/rules"_s, info, u"content"_s, &error ) );
  QVERIFY( !error.isEmpty() );
}

QGSTEST_MAIN( TestQgsAiRulesSkillsStore )
#include "testqgsairulesskillsstore.moc"
