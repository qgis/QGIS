/***************************************************************************
  testqgsaipythonapprovaldialog.cpp
  ---------------------------------
  begin                : June 2026
***************************************************************************/

#include "ai/qgsaiauditlog.h"
#include "ai/qgsaipythonapprovaldialog.h"
#include "qgsapplication.h"
#include "qgstest.h"

#include <QCryptographicHash>
#include <QFile>
#include <QLabel>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryDir>

using namespace Qt::StringLiterals;

class TestQgsAiPythonApprovalDialog : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void riskMarkersDetectNetworkAndSubprocess();
    void riskMarkersEmptyForBenignCode();
    void dialogShowsBannerAndChips();
    void auditLogAppendWritesLine();
};

void TestQgsAiPythonApprovalDialog::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiPythonApprovalDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiPythonApprovalDialog::riskMarkersDetectNetworkAndSubprocess()
{
  const QString code = u"import requests\nimport subprocess\nwith open('/tmp/x', 'w') as f:\n    f.write(eval('1+1'))\n"_s;
  const QStringList markers = QgsAiPythonApprovalDialog::detectRiskMarkers( code );
  QVERIFY2( markers.contains( u"network access"_s ), qPrintable( markers.join( ',' ) ) );
  QVERIFY( markers.contains( u"process execution"_s ) );
  QVERIFY( markers.contains( u"file write/delete"_s ) );
  QVERIFY( markers.contains( u"dynamic code execution"_s ) );
}

void TestQgsAiPythonApprovalDialog::riskMarkersEmptyForBenignCode()
{
  const QString code = u"from qgis.core import QgsProject\nlayers = QgsProject.instance().mapLayers()\nprint(len(layers))\n"_s;
  QVERIFY( QgsAiPythonApprovalDialog::detectRiskMarkers( code ).isEmpty() );
}

void TestQgsAiPythonApprovalDialog::dialogShowsBannerAndChips()
{
  QgsAiPythonApprovalDialog dialog( u"test"_s, u"import socket\nprint('hi')"_s, nullptr );

  QLabel *banner = dialog.findChild<QLabel *>( u"aiPythonWarningBanner"_s );
  QVERIFY( banner );
  QVERIFY( banner->text().contains( u"unsandboxed"_s ) );

  QLabel *risks = dialog.findChild<QLabel *>( u"aiPythonRiskMarkersLabel"_s );
  QVERIFY( risks );
  QVERIFY2( risks->text().contains( u"network access"_s ), qPrintable( risks->text() ) );

  // Benign code: no chip row at all.
  QgsAiPythonApprovalDialog benignDialog( u"test"_s, u"print('hi')"_s, nullptr );
  QVERIFY( !benignDialog.findChild<QLabel *>( u"aiPythonRiskMarkersLabel"_s ) );
}

void TestQgsAiPythonApprovalDialog::auditLogAppendWritesLine()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString auditPath = tempDir.filePath( u"audit.log"_s );
  QgsAiAuditLog::setFilePathOverride( auditPath );
  const auto cleanup = qScopeGuard( []() { QgsAiAuditLog::setFilePathOverride( QString() ); } );

  const QString code = u"print('hello')\nimport os"_s;
  QgsAiAuditLog::append( u"run_python"_s, code );
  QgsAiAuditLog::append( u"download_file"_s, u"https://example.com/a.geojson?token=secret -> /tmp/private/a.geojson"_s );

  QFile file( auditPath );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QStringList lines = QString::fromUtf8( file.readAll() ).trimmed().split( '\n' );
  QCOMPARE( lines.size(), 2 );

  const QString expectedDigest = QString::fromLatin1( QCryptographicHash::hash( code.toUtf8(), QCryptographicHash::Sha256 ).toHex() );
  QVERIFY2( lines.at( 0 ).contains( u"| run_python |"_s ), qPrintable( lines.at( 0 ) ) );
  QVERIFY( lines.at( 0 ).contains( u"sha256=%1"_s.arg( expectedDigest ) ) );
  QVERIFY( lines.at( 0 ).contains( u"code_chars=24"_s ) );
  QVERIFY( !lines.at( 0 ).contains( u"print('hello')"_s ) );
  QVERIFY( lines.at( 1 ).contains( u"| download_file |"_s ) );
  QVERIFY( lines.at( 1 ).contains( u"host=example.com"_s ) );
  QVERIFY( lines.at( 1 ).contains( u"path=/a.geojson"_s ) );
  QVERIFY( lines.at( 1 ).contains( u"dest_file=a.geojson"_s ) );
  QVERIFY( !lines.at( 1 ).contains( u"token=secret"_s ) );
  QVERIFY( !lines.at( 1 ).contains( u"/tmp/private"_s ) );
}

QGSTEST_MAIN( TestQgsAiPythonApprovalDialog )
#include "testqgsaipythonapprovaldialog.moc"
