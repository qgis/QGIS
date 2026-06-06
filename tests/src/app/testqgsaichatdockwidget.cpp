/***************************************************************************
  testqgsaichatdockwidget.cpp
  ---------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaichatdockwidget.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/qgsaireviewpatchengine.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMetaObject>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QToolButton>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>
#include <QVariantMap>

using namespace Qt::StringLiterals;

class TestQgsAiChatDockWidget : public QObject
{
    Q_OBJECT

  private slots:
    void hasRuntimeWidgets();
    void doesNotDuplicateStreamedAssistantResponse();
    void rendersToolResultWithoutRawJson();
    void layerIndexingConsentPolicy();
    void settingsDialogContainsManualIndexingControls();
    void historyMenuPromptsForWorkspaceRootWhenUnset();
};

void TestQgsAiChatDockWidget::hasRuntimeWidgets()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QLabel *runtimeLabel = dock.findChild<QLabel *>( u"aiRuntimeStatusLabel"_s );
  QPushButton *cancelButton = dock.findChild<QPushButton *>( u"aiCancelRequestButton"_s );
  QVERIFY( runtimeLabel );
  QVERIFY( cancelButton );
  QVERIFY( !cancelButton->isEnabled() );
  QVERIFY( runtimeLabel->text().contains( u"idle"_s, Qt::CaseInsensitive ) );
}

void TestQgsAiChatDockWidget::doesNotDuplicateStreamedAssistantResponse()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QTextEdit *transcript = nullptr;
  const QList<QTextEdit *> textEdits = dock.findChildren<QTextEdit *>();
  for ( QTextEdit *textEdit : textEdits )
  {
    if ( textEdit->isReadOnly() )
    {
      transcript = textEdit;
      break;
    }
  }
  QVERIFY( transcript );

  QgsAiChatMessage assistantMessage;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = u"Ciao! Come posso aiutarti con QGIS oggi?"_s;

  manager.responseChunkReceived( u"Ciao! Come posso "_s );
  manager.responseChunkReceived( u"aiutarti con QGIS oggi?"_s );
  QCOMPARE( transcript->toPlainText(), u"[assistant] Ciao! Come posso aiutarti con QGIS oggi?"_s );

  manager.messageAdded( assistantMessage );
  QCOMPARE( transcript->toPlainText(), u"[assistant] Ciao! Come posso aiutarti con QGIS oggi?"_s );
}

void TestQgsAiChatDockWidget::rendersToolResultWithoutRawJson()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QTextEdit *transcript = nullptr;
  const QList<QTextEdit *> textEdits = dock.findChildren<QTextEdit *>();
  for ( QTextEdit *textEdit : textEdits )
  {
    if ( textEdit->isReadOnly() )
    {
      transcript = textEdit;
      break;
    }
  }
  QVERIFY( transcript );

  QJsonObject output;
  output.insert( u"status"_s, u"ok"_s );
  output.insert( u"http_status"_s, 200 );
  output.insert( u"bytes_written"_s, 1234 );
  output.insert( u"dest_path"_s, tempDir.filePath( u"data/pomponesco.geojson"_s ) );

  QgsAiChatMessage toolMessage;
  toolMessage.role = QgsAiChatRole::Tool;
  toolMessage.content = QString::fromUtf8( QJsonDocument( output ).toJson( QJsonDocument::Compact ) );
  toolMessage.metadata.insert( u"tool_name"_s, u"download_file"_s );
  QVariantMap args;
  args.insert( u"url"_s, u"https://overpass-api.de/api/interpreter?data=secret-query"_s );
  toolMessage.metadata.insert( u"tool_args"_s, args );

  manager.messageAdded( toolMessage );
  const QString plain = transcript->toPlainText();
  QVERIFY( plain.contains( u"download_file"_s ) );
  QVERIFY( plain.contains( u"overpass-api.de"_s ) );
  QVERIFY( plain.contains( u"data/pomponesco.geojson"_s ) );
  QVERIFY( !plain.contains( u"{\"status\""_s ) );
  QVERIFY( !plain.contains( u"secret-query"_s ) );
}

void TestQgsAiChatDockWidget::layerIndexingConsentPolicy()
{
  // Round-trip the single key in the user's QSettings without redirecting the
  // global path (which would break sibling tests that read other AI settings).
  QSettings settings;
  const QString key = u"geoai/index/layer_indexing_consented"_s;
  const QString legacyKey = u"qgis_ai/index/layer_indexing_consented"_s;
  const QVariant savedValue = settings.value( key );
  const QVariant savedLegacyValue = settings.value( legacyKey );

  settings.remove( key );
  settings.remove( legacyKey );
  QVERIFY( QgsAiChatDockWidget::requiresLayerIndexingConsent() );

  settings.setValue( legacyKey, true );
  QVERIFY( !QgsAiChatDockWidget::requiresLayerIndexingConsent() );
  settings.remove( legacyKey );

  QgsAiChatDockWidget::recordLayerIndexingConsent();
  QVERIFY( !QgsAiChatDockWidget::requiresLayerIndexingConsent() );

  if ( savedValue.isValid() )
    settings.setValue( key, savedValue );
  else
    settings.remove( key );

  if ( savedLegacyValue.isValid() )
    settings.setValue( legacyKey, savedLegacyValue );
  else
    settings.remove( legacyKey );
}

void TestQgsAiChatDockWidget::settingsDialogContainsManualIndexingControls()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  bool inspected = false;
  bool controlsFound = false;
  QTimer::singleShot( 0, [&inspected, &controlsFound]() {
    QDialog *settingsDialog = qobject_cast<QDialog *>( QApplication::activeModalWidget() );
    if ( settingsDialog )
    {
      controlsFound = settingsDialog->findChild<QCheckBox *>( u"aiEnableLayerIndexingCheckBox"_s )
                      && settingsDialog->findChild<QPushButton *>( u"aiRebuildWorkspaceIndexButton"_s )
                      && settingsDialog->findChild<QPushButton *>( u"aiRebuildLayerIndexButton"_s );
      settingsDialog->reject();
    }
    inspected = true;
  } );

  QVERIFY( QMetaObject::invokeMethod( &dock, "openProviderSettings", Qt::DirectConnection ) );
  QVERIFY( inspected );
  QVERIFY( controlsFound );
}

void TestQgsAiChatDockWidget::historyMenuPromptsForWorkspaceRootWhenUnset()
{
  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider;
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( &router, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QToolButton *historyButton = dock.findChild<QToolButton *>( u"aiHistoryButton"_s );
  QVERIFY( historyButton );
  QVERIFY( historyButton->menu() );

  QVERIFY( QMetaObject::invokeMethod( &dock, "rebuildHistoryMenu", Qt::DirectConnection ) );

  const QList<QAction *> actions = historyButton->menu()->actions();
  QCOMPARE( actions.size(), 1 );
  QVERIFY( actions.first()->text().contains( u"workspace root"_s, Qt::CaseInsensitive ) );
  QVERIFY( !actions.first()->isEnabled() );
}

QGSTEST_MAIN( TestQgsAiChatDockWidget )
#include "testqgsaichatdockwidget.moc"
