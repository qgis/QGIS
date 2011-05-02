/***************************************************************************
                         qgscomposerlegenditemdialog.h
                         -----------------------------
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERLEGENDLAYERSDIALOG_H
#define QGSCOMPOSERLEGENDLAYERSDIALOG_H

#include "ui_qgscomposerlegendlayersdialogbase.h"
#include "qgsmaplayer.h"

/** \ingroup MapComposer
 * A dialog to add new layers to the legend.
 * */
class QgsComposerLegendLayersDialog: private Ui::QgsComposerLegendLayersDialogBase, public QDialog
{
  public:
    QgsComposerLegendLayersDialog( QList<QgsMapLayer*> layers, QWidget* parent = 0 );
    ~QgsComposerLegendLayersDialog();
    QgsMapLayer* selectedLayer();

  private:
    QgsComposerLegendLayersDialog(); //forbidden

    /**Stores the relation between items and map layer pointers. */
    QMap<QListWidgetItem*, QgsMapLayer*> mItemLayerMap;
};

#endif //QGSCOMPOSERLEGENDITEMDIALOG_H
