/**
 * The QgsLayerTreeViewDefaultActions class serves as a factory of actions
 * that can be used together with a layer tree view.
 *
 * @see QgsLayerTreeView
 * @note added in 2.4
 */
class QgsLayerTreeViewDefaultActions : QObject
{
%TypeHeaderCode
#include <qgslayertreeviewdefaultactions.h>
%End

  public:
    QgsLayerTreeViewDefaultActions( QgsLayerTreeView* view );

    QAction* actionAddGroup( QObject* parent = 0 ) /Factory/;
    QAction* actionRemoveGroupOrLayer( QObject* parent = 0 ) /Factory/;
    QAction* actionShowInOverview( QObject* parent = 0 ) /Factory/;
    QAction* actionRenameGroupOrLayer( QObject* parent = 0 ) /Factory/;
    QAction* actionShowFeatureCount( QObject* parent = 0 ) /Factory/;

    QAction* actionZoomToLayer( QgsMapCanvas* canvas, QObject* parent = 0 ) /Factory/;
    QAction* actionZoomToGroup( QgsMapCanvas* canvas, QObject* parent = 0 ) /Factory/;
    // TODO: zoom to selected

    QAction* actionMakeTopLevel( QObject* parent = 0 ) /Factory/;
    QAction* actionGroupSelected( QObject* parent = 0 ) /Factory/;
    //! Action to enable/disable mutually exclusive flag of a group (only one child node may be checked)
    //! @note added in 2.12
    QAction* actionMutuallyExclusiveGroup( QObject* parent = 0 ) /Factory/;

    void zoomToLayer( QgsMapCanvas* canvas );
    void zoomToGroup( QgsMapCanvas* canvas );

  public slots:
    void showInOverview();

  protected slots:
    void addGroup();
    void removeGroupOrLayer();
    void renameGroupOrLayer();
    void showFeatureCount();
    void zoomToLayer();
    void zoomToGroup();
    void makeTopLevel();
    void groupSelected();
    //! Slot to enable/disable mutually exclusive group flag
    //! @note added in 2.12
    void mutuallyExclusiveGroup();

  protected:
    void zoomToLayers( QgsMapCanvas* canvas, const QList<QgsMapLayer*>& layers );

    QString uniqueGroupName( QgsLayerTreeGroup* parentGroup );
};
