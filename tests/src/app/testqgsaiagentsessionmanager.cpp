/***************************************************************************
  testqgsaiagentsessionmanager.cpp
  --------------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaireviewpatchengine.h"
#include "qgstest.h"

#include <QSignalSpy>
#include <QTemporaryDir>

class TestQgsAiAgentSessionManager : public QObject
{
    Q_OBJECT

  private slots:
    void createsPatchProposalFromCommand();
    void blocksContextOutsideWorkspace();
};

void TestQgsAiAgentSessionManager::createsPatchProposalFromCommand()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QSignalSpy proposalSpy( &manager, &QgsAiAgentSessionManager::proposalCreated );
  QSignalSpy messageSpy( &manager, &QgsAiAgentSessionManager::messageAdded );

  manager.sendUserMessage( QStringLiteral( "/patch path=%1\n<<<<\nold\n====\nnew\n>>>>" ).arg( tempDir.filePath( QStringLiteral( "a.txt" ) ) ) );
  QCOMPARE( proposalSpy.count(), 1 );
  QVERIFY( messageSpy.count() >= 2 );
}

void TestQgsAiAgentSessionManager::blocksContextOutsideWorkspace()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QSignalSpy stateSpy( &manager, &QgsAiAgentSessionManager::requestStateChanged );
  manager.sendUserMessage( QStringLiteral( "hello" ), QStringLiteral( "/etc/passwd" ) );
  QVERIFY( stateSpy.count() >= 1 );
  const QList<QVariant> args = stateSpy.takeLast();
  QCOMPARE( args.at( 0 ).toString(), QStringLiteral( "error" ) );
  QVERIFY( args.at( 1 ).toString().contains( QStringLiteral( "blocked" ), Qt::CaseInsensitive ) );
}

QGSTEST_MAIN( TestQgsAiAgentSessionManager )
#include "testqgsaiagentsessionmanager.moc"
