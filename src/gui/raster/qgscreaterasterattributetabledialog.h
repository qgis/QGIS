/***************************************************************************
  qgscreaterasterattributetabledialog.h - QgsCreateRasterAttributeTableDialog

 ---------------------
 begin                : 13.10.2022
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
#ifndef QGSCREATERASTERATTRIBUTETABLEDIALOG_H
#define QGSCREATERASTERATTRIBUTETABLEDIALOG_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgscreaterasterattributetabledialogbase.h"
#include <QDialog>

#ifndef SIP_RUN
class QgsRasterLayer;
#endif

/**
 * \ingroup gui
 * \brief The QgsCreateRasterAttributeTableDialog dialog collects the information required to create a new raster attribute table.
 * \warning Client code must check if the creation of attribute tables is supported by the raster layer by calling QgsRasterLayer::canCreateAttributeTable() before using this dialog.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsCreateRasterAttributeTableDialog : public QDialog, private Ui::QgsCreateRasterAttributeTableDialogBase
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsCreateRasterAttributeTableDialog.
     * \param rasterLayer the raster layer, must be suitable for creating a new raster attribute table
     * \param parent optional parent
     */
    QgsCreateRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the file path in case of VAT.DBF save option.
     */
    QString filePath( ) const;

    /**
     * Returns TRUE if the option to open the newly created attribute table is checked.
     */
    bool openWhenDone( ) const;

  private slots:

    void updateButtons();

  private:

    QgsRasterLayer *mRasterLayer = nullptr;
};

#endif // QGSCREATERASTERATTRIBUTETABLEDIALOG_H
