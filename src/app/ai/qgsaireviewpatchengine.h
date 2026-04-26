#ifndef QGSAIREVIEWPATCHENGINE_H
#define QGSAIREVIEWPATCHENGINE_H

#include "qgsaimodels.h"
#include "qgis_app.h"

#include <QObject>
#include <QMap>
#include <QStringList>

class APP_EXPORT QgsAiReviewPatchEngine : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiReviewPatchEngine( QObject *parent = nullptr );

    QString registerProposal( const QgsAiPatchProposal &proposal );
    QList<QgsAiPatchProposal> pendingProposals() const;
    bool hasPendingProposal( const QString &proposalId ) const;

    QString previewProposalDiff( const QString &proposalId ) const;
    bool acceptProposal( const QString &proposalId, QString *errorMessage = nullptr );
    bool acceptHunks( const QString &proposalId, const QList<int> &hunkIndexes, QString *errorMessage = nullptr );
    bool rejectProposal( const QString &proposalId );
    bool undoLastApply( QString *errorMessage = nullptr );

    QStringList auditTrail() const { return mAuditTrail; }

  signals:
    void proposalAdded( const QString &proposalId );
    void proposalAccepted( const QString &proposalId );
    void proposalRejected( const QString &proposalId );

  private:
    struct FileBackup
    {
      QString filePath;
      QString originalContent;
    };

    struct AppliedPatch
    {
      QString proposalId;
      QList<FileBackup> backups;
    };

    bool applyProposalInternal( const QgsAiPatchProposal &proposal, const QList<int> *hunkIndexes, AppliedPatch &appliedPatch, QString *errorMessage ) const;
    static QString proposalDiff( const QgsAiPatchProposal &proposal );

    QMap<QString, QgsAiPatchProposal> mPendingProposals;
    QList<AppliedPatch> mAppliedPatches;
    QStringList mAuditTrail;
};

#endif // QGSAIREVIEWPATCHENGINE_H
