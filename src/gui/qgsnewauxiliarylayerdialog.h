/***************************************************************************
                         qgsnewauxiliarylayerdialog.h  -  description
                             -------------------
    begin                : Aug 28, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWAUXILIARYLAYERDIALOG_H
#define QGSNEWAUXILIARYLAYERDIALOG_H

#include "ui_qgsnewauxiliarylayerdialogbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 *
 * \brief A dialog to create a new auxiliary layer
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsNewAuxiliaryLayerDialog: public QDialog, private Ui::QgsNewAuxiliaryLayerDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor.
     *
     * \param layer The vector layer for which the auxiliary layer has to be created
     * \param parent Parent window
     */
    QgsNewAuxiliaryLayerDialog( QgsVectorLayer *layer, QWidget *parent = nullptr );

  protected:
    void accept() override;

    QgsVectorLayer *mLayer = nullptr;
};

#endif
