/***************************************************************************
  qgslabelinggui.h
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

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

#include "qgspallabeling.h"

class QgsLabelingGui : public QDialog, private Ui::QgsLabelingGuiBase
{
    Q_OBJECT

  public:
    QgsLabelingGui( QgsPalLabeling* lbl, QgsVectorLayer* layer, QWidget* parent );
    ~QgsLabelingGui();

    QgsPalLayerSettings layerSettings();

  public slots:
    void changeTextColor();
    void changeTextFont();
    void showEngineConfigDialog();
    void changeBufferColor();

    void updateUi();
    void updatePreview();
    void updateOptions();

  protected:
    void populatePlacementMethods();
    void populateFieldNames();
    void updateFont( QFont font );

  private:
    QgsPalLabeling* mLBL;
    QgsVectorLayer* mLayer;
};

#endif
