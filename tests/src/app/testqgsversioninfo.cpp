/***************************************************************************
  testqgsversioninfo.cpp
  ----------------------
  begin                : June 2026
***************************************************************************/

#include "qgsversioninfo.h"
#include "qgstest.h"

#include <QByteArray>
#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsVersionInfo : public QObject
{
    Q_OBJECT

  private slots:
    void versionCodeFromTag();
    void versionOrdering();
    void releaseDetailsFromGitHubReleases();
    void releaseDetailsFromInvalidGitHubReleases();
};

void TestQgsVersionInfo::versionCodeFromTag()
{
  bool ok = false;
  QCOMPARE( QgsVersionInfo::versionCodeFromTag( u"strata-v1.2.0"_s, &ok ), 10200 );
  QVERIFY( ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromTag( u"strata-v10.11.12"_s, &ok ), 101112 );
  QVERIFY( ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromTag( u"qgisai-v1.2.0"_s, &ok ), 0 );
  QVERIFY( !ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromTag( u"geoai-v1.2.0"_s, &ok ), 0 );
  QVERIFY( !ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromTag( u"strata-v1.2.0-rc1"_s, &ok ), 0 );
  QVERIFY( !ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromString( u"1.2"_s, &ok ), 0 );
  QVERIFY( !ok );
}

void TestQgsVersionInfo::versionOrdering()
{
  bool ok = false;
  const int current = QgsVersionInfo::versionCodeFromString( u"1.2.0"_s, &ok );
  QVERIFY( ok );

  QCOMPARE( QgsVersionInfo::versionCodeFromString( u"1.2.0"_s ), current );
  QVERIFY( QgsVersionInfo::versionCodeFromString( u"1.2.1"_s ) > current );
  QVERIFY( QgsVersionInfo::versionCodeFromString( u"1.1.9"_s ) < current );
}

void TestQgsVersionInfo::releaseDetailsFromGitHubReleases()
{
  const QByteArray content = R"json(
[
  {
    "tag_name": "qgisai-v9.9.9",
    "draft": false,
    "prerelease": false,
    "html_url": "https://example.com/wrong",
    "body": "ignored"
  },
  {
    "tag_name": "strata-v1.1.0",
    "draft": false,
    "prerelease": false,
    "html_url": "https://github.com/francemazzi/strata/releases/tag/strata-v1.1.0",
    "body": "older"
  },
  {
    "tag_name": "strata-v1.3.0",
    "draft": true,
    "prerelease": false,
    "html_url": "https://example.com/draft",
    "body": "draft"
  },
  {
    "tag_name": "strata-v1.4.0-rc1",
    "draft": false,
    "prerelease": true,
    "html_url": "https://example.com/prerelease",
    "body": "prerelease"
  },
  {
    "tag_name": "strata-v1.2.0",
    "draft": false,
    "prerelease": false,
    "html_url": "https://github.com/francemazzi/strata/releases/tag/strata-v1.2.0",
    "body": "latest"
  }
]
)json";

  QgsVersionInfo::ReleaseDetails details;
  QString error;
  QVERIFY( QgsVersionInfo::releaseDetailsFromGitHubReleases( content, details, &error ) );
  QVERIFY( error.isEmpty() );
  QCOMPARE( details.versionCode, 10200 );
  QCOMPARE( details.version, u"1.2.0"_s );
  QCOMPARE( details.url, u"https://github.com/francemazzi/strata/releases/tag/strata-v1.2.0"_s );
  QCOMPARE( details.body, u"latest"_s );
}

void TestQgsVersionInfo::releaseDetailsFromInvalidGitHubReleases()
{
  QgsVersionInfo::ReleaseDetails details;
  QString error;
  QVERIFY( !QgsVersionInfo::releaseDetailsFromGitHubReleases( QByteArray( "[bad json" ), details, &error ) );
  QVERIFY( !error.isEmpty() );

  error.clear();
  QVERIFY( !QgsVersionInfo::releaseDetailsFromGitHubReleases( QByteArray( R"json([{"tag_name":"geoai-v1.0.0","draft":false,"prerelease":false}])json" ), details, &error ) );
  QVERIFY( !error.isEmpty() );
}

QGSTEST_MAIN( TestQgsVersionInfo )
#include "testqgsversioninfo.moc"
