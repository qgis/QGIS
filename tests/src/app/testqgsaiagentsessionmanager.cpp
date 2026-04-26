/***************************************************************************
  testqgsaiagentsessionmanager.cpp
  --------------------------------
  begin                : April 2026
***************************************************************************/

#include "ai/qgsaiagentsessionmanager.h"
#include "ai/qgsaifilecontextprovider.h"
#include "ai/qgsaireviewpatchengine.h"
#include "qgstest.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

class TestQgsAiAgentSessionManager : public QObject
{
    Q_OBJECT

  private slots:
    void createsPatchProposalFromCommand();
    void blocksContextOutsideWorkspace();
    void findsWorkspaceFilesForMentions();
    void allowsExplicitExternalAttachmentContext();
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

void TestQgsAiAgentSessionManager::findsWorkspaceFilesForMentions()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );

  QVERIFY( QDir( tempDir.path() ).mkpath( QStringLiteral( "src/app" ) ) );
  QFile file( tempDir.filePath( QStringLiteral( "src/app/main.cpp" ) ) );
  QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
  file.write( "int main() { return 0; }\n" );
  file.close();

  QgsAiFileContextProvider contextProvider( tempDir.path() );
  const QStringList candidates = contextProvider.workspaceFileCandidates( QStringLiteral( "main" ), 5 );
  QVERIFY( candidates.contains( QStringLiteral( "src/app/main.cpp" ) ) );
  QCOMPARE( contextProvider.resolveWorkspaceFile( QStringLiteral( "src/app/main.cpp" ) ), QDir::cleanPath( file.fileName() ) );
}

void TestQgsAiAgentSessionManager::allowsExplicitExternalAttachmentContext()
{
  QTemporaryDir workspaceDir;
  QVERIFY( workspaceDir.isValid() );
  QTemporaryDir externalDir;
  QVERIFY( externalDir.isValid() );

  QFile externalFile( externalDir.filePath( QStringLiteral( "data.txt" ) ) );
  QVERIFY( externalFile.open( QIODevice::WriteOnly | QIODevice::Text ) );
  externalFile.write( "external data\n" );
  externalFile.close();

  QgsAiFileContextProvider contextProvider( workspaceDir.path() );
  QgsAiReviewPatchEngine reviewEngine;
  QgsAiAgentSessionManager manager( nullptr, &contextProvider, &reviewEngine );

  QgsAiChatContextFile contextFile;
  contextFile.filePath = externalFile.fileName();
  contextFile.allowExternal = true;

  QSignalSpy stateSpy( &manager, &QgsAiAgentSessionManager::requestStateChanged );
  QSignalSpy messageSpy( &manager, &QgsAiAgentSessionManager::messageAdded );
  manager.sendUserMessage( QStringLiteral( "hello" ), QList<QgsAiChatContextFile>() << contextFile );

  QVERIFY( stateSpy.isEmpty() );
  QVERIFY( messageSpy.count() >= 2 );
  QVERIFY( manager.history().last().content.contains( QStringLiteral( "No provider" ), Qt::CaseInsensitive ) );
}

QGSTEST_MAIN( TestQgsAiAgentSessionManager )
#include "testqgsaiagentsessionmanager.moc"
