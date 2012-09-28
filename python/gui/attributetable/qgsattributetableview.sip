class QgsAttributeTableView : QTableView
{
%TypeHeaderCode
#include <qgsattributetableview.h>
%End

  public:
    QgsAttributeTableView( QWidget* parent = 0 );
    virtual ~QgsAttributeTableView();

    /**
     * Sets the layer
     * @param canvas canvas pointer
     * @param layer layer pointer
     */
    void setCanvasAndLayer( QgsMapCanvas *canvas, QgsVectorLayer *layer );

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
