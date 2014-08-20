/***************************************************************************
  qgslabelinggui.h
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder dot sk at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QgsLabelingGUI_H
#define QgsLabelingGUI_H

#include <QDialog>
#include <ui_qgslabelingguibase.h>

class QgsVectorLayer;
class QgsMapCanvas;

#include "qgspallabeling.h"

class APP_EXPORT QgsLabelingGui : public QWidget, private Ui::QgsLabelingGuiBase
{
    Q_OBJECT

  public:
    QgsLabelingGui( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QWidget* parent );
    ~QgsLabelingGui();

    QgsPalLayerSettings layerSettings();
    void writeSettingsToLayer();

  public slots:
    void init();
    void apply();
    void showEngineConfigDialog();

  private:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mMapCanvas;

};

#endif


