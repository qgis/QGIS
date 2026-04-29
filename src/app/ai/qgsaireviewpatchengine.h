#ifndef QGSAIREVIEWPATCHENGINE_H
#define QGSAIREVIEWPATCHENGINE_H

#include "qgis_app.h"
#include "qgsaimodels.h"

#include <QMap>
#include <QObject>
#include <QStringList>

class QgsAiFileContextProvider;

class APP_EXPORT QgsAiReviewPatchEngine : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiReviewPatchEngine( QObject *parent = nullptr );

    /**
     * Sets the workspace file context provider used to validate hunk paths before
     * any disk write or read. When set, paths outside the workspace are rejected
     * by acceptProposal/acceptHunks. Caller retains ownership.
     */
    void setContextProvider( QgsAiFileContextProvider *provider ) { mContextProvider = provider; }

    QString registerProposal( const QgsAiPatchProposal &proposal );
    QList<QgsAiPatchProposal> pendingProposals() const;
    bool hasPendingProposal( const QString &proposalId ) const;
    QgsAiPatchProposal proposalById( const QString &proposalId ) const { return mPendingProposals.value( proposalId ); }

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
        bool wasMissing = false; //!< True if the file did not exist before the patch was applied (create-file undo removes it).
    };

    struct AppliedPatch
    {
        QString proposalId;
        QList<FileBackup> backups;
    };

    bool applyProposalInternal( const QgsAiPatchProposal &proposal, const QList<int> *hunkIndexes, AppliedPatch &appliedPatch, QString *errorMessage ) const;
    bool validateHunkPath( const QgsAiPatchHunk &hunk, QString *errorMessage ) const;
    static QString proposalDiff( const QgsAiPatchProposal &proposal );

    QgsAiFileContextProvider *mContextProvider = nullptr;
    QMap<QString, QgsAiPatchProposal> mPendingProposals;
    QList<AppliedPatch> mAppliedPatches;
    QStringList mAuditTrail;
};

#endif // QGSAIREVIEWPATCHENGINE_H
