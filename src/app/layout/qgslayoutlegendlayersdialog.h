/***************************************************************************
    qgslayoutlegendlayersdialog.h
    -----------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTLEGENDLAYERSDIALOG_H
#define QGSLAYOUTLEGENDLAYERSDIALOG_H

#include "ui_qgslayoutlegendlayersdialogbase.h"

class QgsMapLayer;
class QgsMapLayerModel;

/**
 * \ingroup app
 * A dialog to add new layers to the legend.
 * */
class QgsLayoutLegendLayersDialog: public QDialog, private Ui::QgsLayoutLegendLayersDialogBase
{
    Q_OBJECT

  public:
    QgsLayoutLegendLayersDialog( QWidget *parent = nullptr );
    ~QgsLayoutLegendLayersDialog();

    QList< QgsMapLayer * > selectedLayers() const;

  private:
    QgsLayoutLegendLayersDialog() = delete;

    QgsMapLayerModel *mModel = nullptr;
};

#endif //QGSLAYOUTLEGENDLAYERSDIALOG_H
