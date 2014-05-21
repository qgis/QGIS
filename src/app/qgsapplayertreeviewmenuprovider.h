#ifndef QGSAPPLAYERTREEVIEWMENUPROVIDER_H
#define QGSAPPLAYERTREEVIEWMENUPROVIDER_H

#include <QObject>

#include "qgslayertreeview.h"

class QgsMapCanvas;


class QgsAppLayerTreeViewMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
  public:
    QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView* view, QgsMapCanvas* canvas );

    QMenu* createContextMenu();

  protected:
    QgsLayerTreeView* mView;
    QgsMapCanvas* mCanvas;
};


#endif // QGSAPPLAYERTREEVIEWMENUPROVIDER_H
