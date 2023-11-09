/***************************************************************************
  qgslayout3dmapwidget.h
  --------------------------------------
  Date                 : August 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUT3DMAPWIDGET_H
#define QGSLAYOUT3DMAPWIDGET_H

#include "qgis_gui.h"
#include "qgslayoutitemwidget.h"
#include "ui_qgslayout3dmapwidgetbase.h"
#include "qgslayoutitem3dmap.h"

class QgsLayoutItem3DMap;
class Qgs3DMapCanvasWidget;

class QgsLayout3DMapWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayout3DMapWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsLayout3DMapWidget( QgsLayoutItem3DMap *map3D );

    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    void updateCameraPoseWidgetsFromItem();

  private slots:
    void copy3DMapSettings( Qgs3DMapCanvasWidget *widget );
    void copyCameraPose( Qgs3DMapCanvasWidget *widget );
    void updateCameraPose();

  private:
    QPointer< QgsLayoutItem3DMap > mMap3D;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
    QMenu *mMenu3DCanvases = nullptr;
    QMenu *mMenu3DCanvasesPose = nullptr;
};

#endif // QGSLAYOUT3DMAPWIDGET_H
