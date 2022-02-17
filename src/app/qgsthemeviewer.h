/***************************************************************************
  qgsthemeviewer.h
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Alex RL
  Email                : ping me on github
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTHEMEVIEWER_H
#define QGSTHEMEVIEWER_H

#include <QObject>
#include <QWidget>
#include "qgslayertreeview.h"

class QMimeData;
class QContextMenuEvent;

/**
 * QgsThemeViewe class: QgsLayerTreeView with custom drag & drop handling to interact with QgsThemeManagerWidget
 * \since QGIS 3.20
 */

class QgsThemeViewer :  public QgsLayerTreeView
{
    Q_OBJECT
  public:
    explicit QgsThemeViewer( QWidget *parent = nullptr );

    //! List supported drop actions
    Qt::DropActions supportedDropActions() const;


    //! Overridden setModel() from base class. Only QgsLayerTreeModel is an acceptable model.
    //void setModel( QAbstractItemModel *model ) override;


  signals:

    //! Used by QgsThemeManagerWidget to trigger the import of layers
    void layersAdded();

    //! Used by QgsThemeManagerWidget to trigger the removal of layers
    void layersDropped();

    void showMenu( const QPoint &pos );

  protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;

  private:

    QStringList mimeTypes() const;

    QMimeData *mimeData() const;

    void dragEnterEvent( QDragEnterEvent *event ) override;

    //! Call emit layersAdded if drop incoming from the layers widget
    void dropEvent( QDropEvent *event ) override;

    //! Prevent any outdrag and loss of layers when attemptint to move or select them.
    void startDrag( Qt::DropActions ) override;


};

#endif // QGSTHEMEVIEWER_H
