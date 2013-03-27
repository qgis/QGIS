class QgsAttributeTableView : QTableView
{
%TypeHeaderCode
#include <qgsattributetableview.h>
%End

  public:
    QgsAttributeTableView( QWidget* parent = 0 );
    virtual ~QgsAttributeTableView();

    /**
     * Autocreates the models
     * @param layerCache The {@link QgsVectorLayerCache} to use ( as backend )
     * @param canvas     The {@link QgsMapCanvas} to use ( for the currently visible features filter )
     *
     * @deprecated
     */
    void setCanvasAndLayerCache( QgsMapCanvas *canvas, QgsVectorLayerCache *layerCache );

  protected:
    /**
     * Saves geometry to the settings on close
     * @param event not used
     */
    void closeEvent( QCloseEvent *event );

    void contextMenuEvent( QContextMenuEvent* );

  signals:
    void willShowContextMenu( QMenu* menu, QModelIndex atIndex );

    void finished();
    // void progress( int i, bool &cancel );
};
