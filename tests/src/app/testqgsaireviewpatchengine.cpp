/***************************************************************************
  testqgsaireviewpatchengine.cpp
  ------------------------------
  begin                : April 2026
  copyright            : (C) 2026
***************************************************************************/

#include "ai/qgsaireviewpatchengine.h"
#include "qgstest.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

class TestQgsAiReviewPatchEngine : public QObject
{
    Q_OBJECT

  private slots:
    void applyAndUndoProposal();
    void rejectProposal();
};

void TestQgsAiReviewPatchEngine::applyAndUndoProposal()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  const QString filePath = tempDir.filePath( QStringLiteral( "sample.txt" ) );

  {
    QFile file( filePath );
    QVERIFY( file.open( QIODevice::WriteOnly | QIODevice::Text ) );
    QTextStream stream( &file );
    stream << "alpha\nbeta\ngamma\n";
  }

  QgsAiPatchHunk hunk;
  hunk.filePath = filePath;
  hunk.originalText = QStringLiteral( "beta" );
  hunk.replacementText = QStringLiteral( "beta-updated" );

  QgsAiPatchProposal proposal;
  proposal.title = QStringLiteral( "Replace beta token" );
  proposal.hunks << hunk;

  QgsAiReviewPatchEngine engine;
  const QString proposalId = engine.registerProposal( proposal );
  QVERIFY( !proposalId.isEmpty() );

  QString error;
  QVERIFY2( engine.acceptProposal( proposalId, &error ), qPrintable( error ) );

  {
    QFile file( filePath );
    QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    const QString content = QString::fromUtf8( file.readAll() );
    QVERIFY( content.contains( QStringLiteral( "beta-updated" ) ) );
  }

  QVERIFY2( engine.undoLastApply( &error ), qPrintable( error ) );
  {
    QFile file( filePath );
    QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
    const QString content = QString::fromUtf8( file.readAll() );
    QVERIFY( content.contains( QStringLiteral( "beta" ) ) );
    QVERIFY( !content.contains( QStringLiteral( "beta-updated" ) ) );
  }
}

void TestQgsAiReviewPatchEngine::rejectProposal()
{
  QgsAiPatchProposal proposal;
  proposal.title = QStringLiteral( "No-op" );

  QgsAiReviewPatchEngine engine;
  const QString proposalId = engine.registerProposal( proposal );
  QVERIFY( engine.hasPendingProposal( proposalId ) );
  QVERIFY( engine.rejectProposal( proposalId ) );
  QVERIFY( !engine.hasPendingProposal( proposalId ) );
}

QGSTEST_MAIN( TestQgsAiReviewPatchEngine )
#include "testqgsaireviewpatchengine.moc"
