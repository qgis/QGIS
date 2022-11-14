/***************************************************************************
  qgsrasterattributetableaddcolumndialog.h - QgsRasterAttributeTableAddColumnDialog

 ---------------------
 begin                : 10.10.2022
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
#ifndef QGSRASTERATTRIBUTETABLEADDCOLUMNDIALOG_H
#define QGSRASTERATTRIBUTETABLEADDCOLUMNDIALOG_H

#include <QDialog>
#include "qgis_gui.h"
#include "qgis.h"
#include "ui_qgsrasterattributetableaddcolumndialogbase.h"

#define SIP_NO_FILE

class QgsRasterAttributeTable;

/**
 * \ingroup gui
 * \brief The QgsRasterAttributeTableAddColumnDialog class collects options to add a new column to a raster attribute table.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsRasterAttributeTableAddColumnDialog : public QDialog, private Ui::QgsRasterAttributeTableAddColumnDialogBase
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsRasterAttributeTableAddColumnDialog
     * \param attributeTable the raster attribute table
     * \param parent optional parent
     */
    QgsRasterAttributeTableAddColumnDialog( QgsRasterAttributeTable *attributeTable, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the position where the new column (before) will be inserted.
     */
    int position() const;

    /**
     * Returns TRUE if the add color column option was checked.
     */
    bool isColor( ) const;

    /**
     * Returns TRUE if the add color ramp column option was checked.
     */
    bool isRamp( ) const;

    /**
     * Returns the new column name.
     */
    QString name( ) const;

    /**
     * Returns the new column name.
     */
    Qgis::RasterAttributeTableFieldUsage usage( ) const;

    /**
     * Returns the new column type.
     */
    QVariant::Type type( ) const;


  private:

    QgsRasterAttributeTable *mAttributeTable;

    void updateDialog();
};

#endif // QGSRASTERATTRIBUTETABLEADDCOLUMNDIALOG_H
