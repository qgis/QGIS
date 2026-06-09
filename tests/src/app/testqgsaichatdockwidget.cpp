/***************************************************************************
  testqgsaichatdockwidget.cpp
  ---------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaichatdockwidget.h"
#include "ai/qgsaichathistorystore.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaimodelrouter.h"
#include "ai/qgsaireviewpatchengine.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QEvent>
#include <QFrame>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMetaObject>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
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
    void usesPaletteBasedCursorStyling();
    void doesNotDuplicateStreamedAssistantResponse();
    void rendersToolResultWithoutRawJson();
    void collapsesTechnicalCodeBlocks();
    void transcriptMessagesFitNarrowDockWithoutHorizontalScroll();
    void acceptingPlanSwitchesToAgentAndSendsPlan();
    void questionCardSendsStructuredAnswers();
    void layerIndexingConsentPolicy();
    void settingsDialogContainsManualIndexingControls();
    void historyMenuPromptsForSavedProjectWhenUnsaved();
    void historyMenuDoesNotShowWorkspaceHistoryForUnsavedProject();
    void historyMenuShowsOnlyCurrentProjectSessions();
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

void TestQgsAiChatDockWidget::usesPaletteBasedCursorStyling()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QTextEdit *input = dock.findChild<QTextEdit *>( u"aiPromptInput"_s );
  QToolButton *sendButton = dock.findChild<QToolButton *>( u"aiSendButton"_s );
  QToolButton *modePill = dock.findChild<QToolButton *>( u"aiModePill"_s );
  QVERIFY( input );
  QVERIFY( sendButton );
  QVERIFY( modePill );
  QVERIFY( input->styleSheet().contains( u"palette(base)"_s ) );
  QVERIFY( sendButton->styleSheet().contains( u"palette(highlight)"_s ) );
  QVERIFY( modePill->styleSheet().contains( u"palette(button)"_s ) );

  QgsAiChatMessage assistantMessage;
  assistantMessage.id = u"assistant-style"_s;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content = u"Styled response."_s;
  manager.messageAdded( assistantMessage );

  QFrame *message = dock.findChild<QFrame *>( u"aiMessage"_s );
  QVERIFY( message );
  QVERIFY( message->styleSheet().contains( u"palette(base)"_s ) );
  QVERIFY( message->styleSheet().contains( u"palette(mid)"_s ) );
  static const QRegularExpression hexColorRe( u"#[0-9a-fA-F]{3,8}\\b"_s );
  QVERIFY( !hexColorRe.match( message->styleSheet() ).hasMatch() );
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

void TestQgsAiChatDockWidget::transcriptMessagesFitNarrowDockWithoutHorizontalScroll()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );
  dock.resize( 300, 520 );
  dock.show();
  QApplication::processEvents();

  QgsAiChatMessage assistantMessage;
  assistantMessage.id = u"assistant-wide-table"_s;
  assistantMessage.role = QgsAiChatRole::Assistant;
  assistantMessage.content
    = u"Ecco l'elenco completo dei layer.\n\n"
      "| Nome layer | Tipo layer | Geometria | N. feature | CRS |\n"
      "| --- | --- | --- | ---: | --- |\n"
      "| dbgt_AB_CDA_AB_CDA_SUP_SR_nome_molto_lungo_senza_spazi | vector | MultiPolygonZWithVeryLongGeometryName | 19382 | EPSG:7791-long-crs-description-without-natural-breaks |\n"
      "| dbgt_ACC_PC_ACC_PC_POS_layer_con_nome_esteso | vector | Point | 41942 | non definito - percorso /Users/francesco/Sviluppo/frasma_lab/strata/tests_ai/Dati/strata_test_brescia.qgz |\n\n"
      "```json\n"
      "{\"project_file\":\"/Users/francesco/Sviluppo/frasma_lab/strata/tests_ai/Dati/strata_test_brescia.qgz\",\"layer_count\":118}\n"
      "```"_s;

  manager.messageAdded( assistantMessage );
  manager.responseChunkReceived( u"streaming-token-without-spaces-or-natural-breaks-EPSG7791-layer-name-dbgt_ACC_PC_ACC_PC_POS"_s );
  QCoreApplication::sendPostedEvents( nullptr, QEvent::DeferredDelete );
  QApplication::processEvents();

  QAbstractScrollArea *scrollArea = dock.findChild<QAbstractScrollArea *>( u"aiTranscriptScrollArea"_s );
  QWidget *container = dock.findChild<QWidget *>( u"aiTranscriptContainer"_s );
  QVERIFY( scrollArea );
  QVERIFY( container );
  QVERIFY( scrollArea->viewport() );
  QCOMPARE( scrollArea->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff );

  const int viewportWidth = scrollArea->viewport()->width();
  QVERIFY( viewportWidth > 0 );
  QVERIFY2( container->width() <= viewportWidth + 1, qPrintable( u"Transcript container width %1 exceeds viewport width %2"_s.arg( container->width() ).arg( viewportWidth ) ) );

  const QList<QFrame *> frames = container->findChildren<QFrame *>();
  bool checkedTranscriptFrame = false;
  for ( QFrame *frame : frames )
  {
    if ( frame->objectName() != "aiMessage"_L1 && frame->objectName() != "aiStreamingMessage"_L1 && frame->objectName() != "aiPlanCard"_L1 && frame->objectName() != "aiQuestionsCard"_L1 )
      continue;

    checkedTranscriptFrame = true;
    QVERIFY2( frame->width() <= viewportWidth + 1, qPrintable( u"%1 width %2 exceeds viewport width %3"_s.arg( frame->objectName() ).arg( frame->width() ).arg( viewportWidth ) ) );
  }
  QVERIFY( checkedTranscriptFrame );

  const QList<QTextEdit *> transcriptEdits = container->findChildren<QTextEdit *>();
  QVERIFY( !transcriptEdits.isEmpty() );
  for ( QTextEdit *edit : transcriptEdits )
  {
    QCOMPARE( edit->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff );
    QVERIFY( edit->lineWrapMode() != QTextEdit::NoWrap );
  }
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
  const QString key = u"strata/index/layer_indexing_consented"_s;
  const QString geoAiLegacyKey = u"geoai/index/layer_indexing_consented"_s;
  const QString qgisAiLegacyKey = u"qgis_ai/index/layer_indexing_consented"_s;
  const QVariant savedValue = settings.value( key );
  const QVariant savedGeoAiLegacyValue = settings.value( geoAiLegacyKey );
  const QVariant savedQgisAiLegacyValue = settings.value( qgisAiLegacyKey );

  settings.remove( key );
  settings.remove( geoAiLegacyKey );
  settings.remove( qgisAiLegacyKey );
  QVERIFY( QgsAiChatDockWidget::requiresLayerIndexingConsent() );

  settings.setValue( geoAiLegacyKey, true );
  QVERIFY( !QgsAiChatDockWidget::requiresLayerIndexingConsent() );
  settings.remove( geoAiLegacyKey );

  settings.setValue( qgisAiLegacyKey, true );
  QVERIFY( !QgsAiChatDockWidget::requiresLayerIndexingConsent() );
  settings.remove( qgisAiLegacyKey );

  QgsAiChatDockWidget::recordLayerIndexingConsent();
  QVERIFY( !QgsAiChatDockWidget::requiresLayerIndexingConsent() );

  if ( savedValue.isValid() )
    settings.setValue( key, savedValue );
  else
    settings.remove( key );

  if ( savedGeoAiLegacyValue.isValid() )
    settings.setValue( geoAiLegacyKey, savedGeoAiLegacyValue );
  else
    settings.remove( geoAiLegacyKey );

  if ( savedQgisAiLegacyValue.isValid() )
    settings.setValue( qgisAiLegacyKey, savedQgisAiLegacyValue );
  else
    settings.remove( qgisAiLegacyKey );
}

void TestQgsAiChatDockWidget::settingsDialogContainsManualIndexingControls()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsSettings settings;
  const QString openAiKey = u"ai/provider/openai/apiKey"_s;
  const QString legacyEmbeddingProviderKey = u"ai/embeddings/provider"_s;
  const QString embeddingProviderKey = u"strata/index/embedding_provider"_s;
  const QString automaticIndexingKey = u"strata/index/automatic"_s;
  const QString layerIndexingKey = u"strata/index/enable_layer_indexing"_s;
  const bool hadOpenAiKey = settings.contains( openAiKey );
  const QVariant savedOpenAiKey = settings.value( openAiKey );
  const bool hadLegacyEmbeddingProvider = settings.contains( legacyEmbeddingProviderKey );
  const QVariant savedLegacyEmbeddingProvider = settings.value( legacyEmbeddingProviderKey );
  const bool hadEmbeddingProvider = settings.contains( embeddingProviderKey );
  const QVariant savedEmbeddingProvider = settings.value( embeddingProviderKey );
  const bool hadAutomaticIndexing = settings.contains( automaticIndexingKey );
  const QVariant savedAutomaticIndexing = settings.value( automaticIndexingKey );
  const bool hadLayerIndexing = settings.contains( layerIndexingKey );
  const QVariant savedLayerIndexing = settings.value( layerIndexingKey );
  settings.setValue( openAiKey, u"sk-old-test-key"_s );
  settings.setValue( legacyEmbeddingProviderKey, u"openai"_s );
  settings.remove( embeddingProviderKey );
  settings.setValue( automaticIndexingKey, true );
  settings.setValue( layerIndexingKey, true );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiLocalEmbeddingProvider embeddingProvider;
  QgsAiWorkspaceIndex workspaceIndex( &contextProvider, &embeddingProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setWorkspaceIndex( &workspaceIndex );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  bool inspected = false;
  bool controlsFound = false;
  bool layerIndexingChecked = true;
  bool layerIndexingEnabled = true;
  bool localStatusFound = false;
  bool defaultProviderSelected = false;
  QTimer::singleShot( 0, [&inspected, &controlsFound, &layerIndexingChecked, &layerIndexingEnabled, &localStatusFound, &defaultProviderSelected]() {
    QDialog *settingsDialog = qobject_cast<QDialog *>( QApplication::activeModalWidget() );
    if ( settingsDialog )
    {
      QCheckBox *layerIndexing = settingsDialog->findChild<QCheckBox *>( u"aiEnableLayerIndexingCheckBox"_s );
      QComboBox *providerCombo = settingsDialog->findChild<QComboBox *>( u"aiEmbeddingProviderComboBox"_s );
      QLabel *statusLabel = settingsDialog->findChild<QLabel *>( u"aiEmbeddingProviderStatusLabel"_s );
      controlsFound = layerIndexing
                      && providerCombo
                      && settingsDialog->findChild<QCheckBox *>( u"aiAutomaticIndexingCheckBox"_s )
                      && settingsDialog->findChild<QPushButton *>( u"aiRebuildWorkspaceIndexButton"_s )
                      && settingsDialog->findChild<QPushButton *>( u"aiRebuildLayerIndexButton"_s );
      if ( layerIndexing )
      {
        layerIndexingChecked = layerIndexing->isChecked();
        layerIndexingEnabled = layerIndexing->isEnabled();
      }
      defaultProviderSelected = providerCombo && providerCombo->currentData().toString() == QgsAiEmbeddingProviderRegistry::defaultProviderId();
      localStatusFound = statusLabel && statusLabel->text().contains( u"without an API key"_s, Qt::CaseInsensitive );
      settingsDialog->reject();
    }
    inspected = true;
  } );

  const bool invoked = QMetaObject::invokeMethod( &dock, "openProviderSettings", Qt::DirectConnection );

  if ( hadOpenAiKey )
    settings.setValue( openAiKey, savedOpenAiKey );
  else
    settings.remove( openAiKey );
  if ( hadLegacyEmbeddingProvider )
    settings.setValue( legacyEmbeddingProviderKey, savedLegacyEmbeddingProvider );
  else
    settings.remove( legacyEmbeddingProviderKey );
  if ( hadEmbeddingProvider )
    settings.setValue( embeddingProviderKey, savedEmbeddingProvider );
  else
    settings.remove( embeddingProviderKey );
  if ( hadAutomaticIndexing )
    settings.setValue( automaticIndexingKey, savedAutomaticIndexing );
  else
    settings.remove( automaticIndexingKey );
  if ( hadLayerIndexing )
    settings.setValue( layerIndexingKey, savedLayerIndexing );
  else
    settings.remove( layerIndexingKey );

  QVERIFY( invoked );
  QVERIFY( inspected );
  QVERIFY( controlsFound );
  QVERIFY( localStatusFound );
  QVERIFY( defaultProviderSelected );
  QVERIFY( layerIndexingChecked );
  QVERIFY( layerIndexingEnabled );
}

void TestQgsAiChatDockWidget::historyMenuPromptsForSavedProjectWhenUnsaved()
{
  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( QString {} );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );

  QToolButton *historyButton = dock.findChild<QToolButton *>( u"aiHistoryButton"_s );
  QVERIFY( historyButton );
  QVERIFY( historyButton->menu() );

  QVERIFY( QMetaObject::invokeMethod( &dock, "rebuildHistoryMenu", Qt::DirectConnection ) );

  const QList<QAction *> actions = historyButton->menu()->actions();
  QCOMPARE( actions.size(), 1 );
  QVERIFY( actions.first()->text().contains( u"QGIS project"_s, Qt::CaseInsensitive ) );
  QVERIFY( !actions.first()->isEnabled() );
}

void TestQgsAiChatDockWidget::historyMenuDoesNotShowWorkspaceHistoryForUnsavedProject()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setHistoryStore( &store );

  QVERIFY( store.createSession( u"legacy-workspace-chat"_s, u"Legacy workspace chat"_s, u"planner"_s ) );
  manager.resetProjectChatHistoryScope();

  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );
  QToolButton *historyButton = dock.findChild<QToolButton *>( u"aiHistoryButton"_s );
  QVERIFY( historyButton );
  QVERIFY( historyButton->menu() );

  QVERIFY( QMetaObject::invokeMethod( &dock, "rebuildHistoryMenu", Qt::DirectConnection ) );

  const QList<QAction *> actions = historyButton->menu()->actions();
  QCOMPARE( actions.size(), 1 );
  QVERIFY( actions.first()->text().contains( u"QGIS project"_s, Qt::CaseInsensitive ) );
  QVERIFY( !actions.first()->text().contains( u"Legacy workspace chat"_s ) );
  QVERIFY( !actions.first()->isEnabled() );
}

void TestQgsAiChatDockWidget::historyMenuShowsOnlyCurrentProjectSessions()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiModelRouter router;
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiChatHistoryStore store( &contextProvider );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );
  manager.setHistoryStore( &store );

  const QString projectScope1 = QgsAiAgentSessionManager::chatHistoryScopeKeyForProjectFile( tempDir.filePath( u"one.qgz"_s ) );
  const QString projectScope2 = QgsAiAgentSessionManager::chatHistoryScopeKeyForProjectFile( tempDir.filePath( u"two.qgz"_s ) );

  manager.setProjectChatHistoryScopeKey( projectScope1 );
  manager.sendUserMessage( u"project alpha"_s );
  manager.setProjectChatHistoryScopeKey( projectScope2 );
  manager.sendUserMessage( u"project beta"_s );

  QgsAiChatDockWidget dock( &manager, &router, &reviewEngine );
  QToolButton *historyButton = dock.findChild<QToolButton *>( u"aiHistoryButton"_s );
  QVERIFY( historyButton );
  QVERIFY( historyButton->menu() );

  QVERIFY( QMetaObject::invokeMethod( &dock, "rebuildHistoryMenu", Qt::DirectConnection ) );
  QStringList projectTwoLabels;
  for ( QAction *action : historyButton->menu()->actions() )
    projectTwoLabels << action->text();
  const QString projectTwoMenu = projectTwoLabels.join( '\n' );
  QVERIFY( projectTwoMenu.contains( u"project beta"_s ) );
  QVERIFY( !projectTwoMenu.contains( u"project alpha"_s ) );

  manager.setProjectChatHistoryScopeKey( projectScope1 );
  QVERIFY( QMetaObject::invokeMethod( &dock, "rebuildHistoryMenu", Qt::DirectConnection ) );
  QStringList projectOneLabels;
  for ( QAction *action : historyButton->menu()->actions() )
    projectOneLabels << action->text();
  const QString projectOneMenu = projectOneLabels.join( '\n' );
  QVERIFY( projectOneMenu.contains( u"project alpha"_s ) );
  QVERIFY( !projectOneMenu.contains( u"project beta"_s ) );
}

QGSTEST_MAIN( TestQgsAiChatDockWidget )
#include "testqgsaichatdockwidget.moc"
