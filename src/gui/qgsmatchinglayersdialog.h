/***************************************************************************
    qgslayoutlegendlayersdialog.h
    -----------------------------
    begin                : March 2020
    copyright            : (C) 2020 by roya0045
    email                : roya0045 at users dot noreply dot github dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMATCHINGLAYERSDIALOG_H
#define QGSMATCHINGLAYERSDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutlegendlayersdialogbase.h"

class QgsMapLayer;
class QgsMapLayerProxyModel;

/**
 * \ingroup gui
 * A dialog to add new layers to the legend.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsMatchingLayersDialog: public QDialog, private Ui::QgsLayoutLegendLayersDialogBase
{
    Q_OBJECT

  public:
    //! constructor
    QgsMatchingLayersDialog( QgsMapLayer *layer = nullptr, QWidget *parent = nullptr );

    //! Returns the list of selected layers
    QList< QgsMapLayer * > selectedLayers() const;

  private slots:

    void filterVisible( bool enabled );

  private:

    QgsMapLayerProxyModel *mModel = nullptr;
};

#endif //QGSLAYOUTLEGENDLAYERSDIALOG_H
