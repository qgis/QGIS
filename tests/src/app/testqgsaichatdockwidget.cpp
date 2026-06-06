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
#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMetaObject>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QString>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QVariantMap>

using namespace Qt::StringLiterals;

namespace
{
  QString transcriptText( const QgsAiChatDockWidget &dock )
  {
    QWidget *container = dock.findChild<QWidget *>( u"aiTranscriptContainer"_s );
    if ( !container )
      return QString();

    QStringList parts;
    const QList<QLabel *> labels = container->findChildren<QLabel *>();
    for ( QLabel *label : labels )
      parts << label->text();
    const QList<QTextEdit *> edits = container->findChildren<QTextEdit *>();
    for ( QTextEdit *edit : edits )
      parts << edit->toPlainText();
    return parts.join( '\n' );
  }

  QString visibleLabelText( const QgsAiChatDockWidget &dock )
  {
    QWidget *container = dock.findChild<QWidget *>( u"aiTranscriptContainer"_s );
    if ( !container )
      return QString();

    QStringList parts;
    const QList<QLabel *> labels = container->findChildren<QLabel *>();
    for ( QLabel *label : labels )
      parts << label->text();
    return parts.join( '\n' );
  }
} // namespace

class TestQgsAiChatDockWidget : public QObject
{
    Q_OBJECT

  private slots:
    void hasRuntimeWidgets();
    void doesNotDuplicateStreamedAssistantResponse();
    void rendersToolResultWithoutRawJson();
    void collapsesTechnicalCodeBlocks();
    void acceptingPlanSwitchesToAgentAndSendsPlan();
    void questionCardSendsStructuredAnswers();
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
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
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
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QgsAiChatMessage assistantMessage;
  assistantMessage.id = u"assistant-1"_s;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = u"Ciao! Come posso aiutarti con QGIS oggi?"_s;

  manager.responseChunkReceived( u"Ciao! Come posso "_s );
  manager.responseChunkReceived( u"aiutarti con QGIS oggi?"_s );
  QCOMPARE( transcriptText( dock ).count( u"Ciao! Come posso aiutarti con QGIS oggi?"_s ), 1 );

  manager.messageAdded( assistantMessage );
  QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QApplication::processEvents();
  QCOMPARE( transcriptText( dock ).count( u"Ciao! Come posso aiutarti con QGIS oggi?"_s ), 1 );
}

void TestQgsAiChatDockWidget::rendersToolResultWithoutRawJson()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

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
  const QString plain = transcriptText( dock );
  QVERIFY( plain.contains( u"download_file"_s ) );
  QVERIFY( plain.contains( u"overpass-api.de"_s ) );
  QVERIFY( plain.contains( u"data/pomponesco.geojson"_s ) );
  QVERIFY( !plain.contains( u"{\"status\""_s ) );
  QVERIFY( !plain.contains( u"secret-query"_s ) );
}

void TestQgsAiChatDockWidget::collapsesTechnicalCodeBlocks()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QgsAiChatMessage assistantMessage;
  assistantMessage.id = u"assistant-code"_s;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = u"Here is the result.\n```python\nprint('hidden')\n```\nDone."_s;

  manager.messageAdded( assistantMessage );

  QToolButton *toggle = dock.findChild<QToolButton *>( u"aiTechnicalToggle"_s );
  QTextEdit *details = dock.findChild<QTextEdit *>( u"aiTechnicalContent"_s );
  QVERIFY( toggle );
  QVERIFY( details );
  QVERIFY( !details->isVisible() );
  QVERIFY( details->toPlainText().contains( u"print('hidden')"_s ) );
  QVERIFY( !visibleLabelText( dock ).contains( u"print('hidden')"_s ) );
}

void TestQgsAiChatDockWidget::acceptingPlanSwitchesToAgentAndSendsPlan()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QgsAiChatMessage planMessage;
  planMessage.id = u"plan-1"_s;
  planMessage.role = QgsAiChatRole::Assistant;
  planMessage.content = u"<proposed_plan>\n1. Read files\n2. Patch UI\n</proposed_plan>"_s;
  planMessage.metadata.insert( u"ui_kind"_s, u"plan"_s );
  planMessage.metadata.insert( u"plan_markdown"_s, u"1. Read files\n2. Patch UI"_s );
  planMessage.metadata.insert( u"plan_status"_s, u"pending"_s );

  manager.messageAdded( planMessage );
  QPushButton *accept = dock.findChild<QPushButton *>( u"aiAcceptPlanButton"_s );
  QVERIFY( accept );
  accept->click();

  QCOMPARE( manager.activeAgent(), u"editor"_s );
  QVERIFY( !manager.history().isEmpty() );
  QVERIFY( manager.history().first().content.contains( u"Accepted plan"_s ) );
  QVERIFY( manager.history().first().content.contains( u"Patch UI"_s ) );
}

void TestQgsAiChatDockWidget::questionCardSendsStructuredAnswers()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QJsonObject option;
  option.insert( u"id"_s, u"default"_s );
  option.insert( u"label"_s, u"Default"_s );
  option.insert( u"description"_s, u"Recommended"_s );
  QJsonObject question;
  question.insert( u"id"_s, u"scope"_s );
  question.insert( u"type"_s, u"single"_s );
  question.insert( u"question"_s, u"Which scope?"_s );
  QJsonArray options;
  options.append( option );
  question.insert( u"options"_s, options );
  question.insert( u"allow_other"_s, true );
  QJsonObject payload;
  QJsonArray questions;
  questions.append( question );
  payload.insert( u"questions"_s, questions );

  QgsAiChatMessage questionsMessage;
  questionsMessage.id = u"questions-1"_s;
  questionsMessage.role = QgsAiChatRole::Assistant;
  questionsMessage.content = u"Need one decision.\n```qgis_ai_questions\n{}\n```"_s;
  questionsMessage.metadata.insert( u"ui_kind"_s, u"questions"_s );
  questionsMessage.metadata.insert( u"questions_json"_s, QString::fromUtf8( QJsonDocument( payload ).toJson( QJsonDocument::Compact ) ) );
  questionsMessage.metadata.insert( u"questions_status"_s, u"pending"_s );

  manager.messageAdded( questionsMessage );

  QRadioButton *optionButton = dock.findChild<QRadioButton *>( u"aiQuestionOption"_s );
  QPushButton *submit = dock.findChild<QPushButton *>( u"aiSubmitQuestionAnswersButton"_s );
  QVERIFY( optionButton );
  QVERIFY( submit );
  optionButton->setChecked( true );
  submit->click();

  QCOMPARE( manager.activeAgent(), u"planner"_s );
  QVERIFY( !manager.history().isEmpty() );
  QVERIFY( manager.history().first().content.contains( u"qgis_ai_answers"_s ) );
  QVERIFY( manager.history().first().content.contains( u"default"_s ) );
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
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
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
  QgsAiFileContextProvider contextProvider( QString {} );
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
