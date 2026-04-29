#ifndef QGSAIREVIEWDIALOG_H
#define QGSAIREVIEWDIALOG_H

#include "qgis_app.h"
#include "qgsaimodels.h"

#include <QDialog>
#include <QList>

class QTabWidget;
class QCheckBox;

/**
 * Modal dialog used to review and approve a single QgsAiPatchProposal coming from
 * the AI assistant. Each hunk is displayed in its own tab with side-by-side
 * before/after panels and an "include this hunk" checkbox.
 *
 * The dialog has three terminal actions:
 *
 * - **Accept all**       → exec() returns QDialog::Accepted, acceptedHunkIndexes() empty (meaning "all")
 * - **Accept selected**  → returns QDialog::Accepted, acceptedHunkIndexes() lists the selected indexes
 * - **Reject**           → returns QDialog::Rejected
 *
 * The dialog does NOT apply the proposal itself; the caller decides what to do
 * with the returned indexes (typically `QgsAiReviewPatchEngine::acceptHunks` or
 * `acceptProposal`).
 */
class APP_EXPORT QgsAiReviewDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit QgsAiReviewDialog( const QgsAiPatchProposal &proposal, QWidget *parent = nullptr );

    /**
     * Returns the indexes of the hunks the user explicitly selected when accepting.
     * If empty after Accept, the user clicked "Accept all" and the caller should
     * apply the entire proposal.
     */
    QList<int> acceptedHunkIndexes() const { return mAcceptedHunkIndexes; }

  private slots:
    void onAcceptAll();
    void onAcceptSelected();
    void onReject();

  private:
    QString hunkSummary( const QgsAiPatchHunk &hunk ) const;

    QgsAiPatchProposal mProposal;
    QTabWidget *mTabs = nullptr;
    QList<QCheckBox *> mHunkChecks;
    QList<int> mAcceptedHunkIndexes;
};

#endif // QGSAIREVIEWDIALOG_H
