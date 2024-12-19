/***************************************************************************
  qgsrasterattributetableaddrowdialog.h - QgsRasterAttributeTableAddRowDialog

 ---------------------
 begin                : 18.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLEADDROWDIALOG_H
#define QGSRASTERATTRIBUTETABLEADDROWDIALOG_H

#include <QDialog>
#include "qgis_gui.h"
#include "qgis.h"
#include "ui_qgsrasterattributetableaddrowdialogbase.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief The QgsRasterAttributeTableAddColumnDialog class collects options to add a new row to a raster attribute table.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsRasterAttributeTableAddRowDialog : public QDialog, private Ui::QgsRasterAttributeTableAddRowDialogBase
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsRasterAttributeTableAddRowDialog
     * \param parent optional parent
     */
    QgsRasterAttributeTableAddRowDialog( QWidget *parent = nullptr );

    /**
     * Returns TRUE if the desired insertion position for the new row is after the currently selected row, FALSE if the insertion point is before.
     */
    bool insertAfter() const;


};

#endif // QGSRASTERATTRIBUTETABLEADDROWDIALOG_H
