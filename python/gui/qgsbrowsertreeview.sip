/**
 * The QgsBrowserTreeView class extends QTreeView with save/restore tree state functionality.
 *
 * @see QgsBrowserModel
 * @note added in 2.8
 */
class QgsBrowserTreeView: QTreeView
{
%TypeHeaderCode
#include <qgsbrowsertreeview.h>
%End

  public:
    QgsBrowserTreeView( QWidget *parent /TransferThis/ = 0 );
    ~QgsBrowserTreeView();

    virtual void setModel( QAbstractItemModel* model );
    virtual void showEvent( QShowEvent * e );
    virtual void hideEvent( QHideEvent * e );

    // returns true if at least one descendat is expanded, used in refresh
    bool hasExpandedDescendant( const QModelIndex& index ) const;

    // Set section where to store settings (because we have 2 browser dock widgets)
    void setSettingsSection( const QString & section );

  protected slots:
    virtual void rowsInserted( const QModelIndex & parentIndex, int start, int end );
};
