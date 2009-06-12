/***************************************************************************
  labelinggui.h
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

#ifndef LabelingGUI_H
#define LabelingGUI_H

#include <QDialog>
#include <ui_labelingguibase.h>

//class PalLabeling;
class QgsVectorLayer;
//struct PalLabeling::LayerSettings;

#include "pallabeling.h"

class LabelingGui : public QDialog, private Ui::LabelingGuiBase
{
  Q_OBJECT

  public:
    LabelingGui( PalLabeling* lbl, QString layerId, QWidget* parent );
    ~LabelingGui();

    LayerSettings layerSettings();

  public slots:
    void changeTextColor();
    void changeTextFont();
    void showEngineConfigDialog();

  protected:
    void populatePlacementMethods();
    void populateFieldNames();
    void updateFontPreview(QFont font);

    QgsVectorLayer* layer();

  private:
    PalLabeling* mLBL;
    QString mLayerId;
};

#endif
