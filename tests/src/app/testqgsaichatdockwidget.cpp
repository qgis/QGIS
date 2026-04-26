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
#include "qgstest.h"

#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTextEdit>

class TestQgsAiChatDockWidget : public QObject
{
    Q_OBJECT

  private slots:
    void hasRuntimeWidgets();
    void doesNotDuplicateStreamedAssistantResponse();
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

  QLabel *runtimeLabel = dock.findChild<QLabel *>( QStringLiteral( "aiRuntimeStatusLabel" ) );
  QPushButton *cancelButton = dock.findChild<QPushButton *>( QStringLiteral( "aiCancelRequestButton" ) );
  QVERIFY( runtimeLabel );
  QVERIFY( cancelButton );
  QVERIFY( !cancelButton->isEnabled() );
  QVERIFY( runtimeLabel->text().contains( QStringLiteral( "idle" ), Qt::CaseInsensitive ) );
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
  assistantMessage.content = QStringLiteral( "Ciao! Come posso aiutarti con QGIS oggi?" );

  manager.responseChunkReceived( QStringLiteral( "Ciao! Come posso " ) );
  manager.responseChunkReceived( QStringLiteral( "aiutarti con QGIS oggi?" ) );
  QCOMPARE( transcript->toPlainText(), QStringLiteral( "[assistant] Ciao! Come posso aiutarti con QGIS oggi?" ) );

  manager.messageAdded( assistantMessage );
  QCOMPARE( transcript->toPlainText(), QStringLiteral( "[assistant] Ciao! Come posso aiutarti con QGIS oggi?" ) );
}

QGSTEST_MAIN( TestQgsAiChatDockWidget )
#include "testqgsaichatdockwidget.moc"
