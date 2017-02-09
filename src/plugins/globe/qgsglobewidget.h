/***************************************************************************
    qgsglobewidget.h
    ---------------------
    begin                : August 2010
    copyright            : (C) 2016 Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDockWidget>

class QgisInterface;
class QgsGlobePlugin;
class QMenu;

class QgsGlobeWidget : public QDockWidget
{
    Q_OBJECT
  public:
    QgsGlobeWidget( QgisInterface *iface, QWidget* parent = 0 );
    QStringList getSelectedLayers() const;

  signals:
    void layersChanged();
    void showSettings();
    void refresh();
    void syncExtent();

  private:
    QgisInterface* mQgisIface;
    QMenu* mLayerSelectionMenu;

    void contextMenuEvent( QContextMenuEvent * e ) override;

  private slots:
    void updateLayerSelectionMenu();
};
