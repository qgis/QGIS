/***************************************************************************
  qgsloadrasterattributetabledialog.h - QgsLoadRasterAttributeTableDialog

 ---------------------
 begin                : 21.10.2022
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
#ifndef QGSLOADRASTERATTRIBUTETABLEDIALOG_H
#define QGSLOADRASTERATTRIBUTETABLEDIALOG_H


#include "qgis_gui.h"
#include "qgis.h"
#include "ui_qgsloadrasterattributetabledialogbase.h"
#include <QDialog>

#define SIP_NO_FILE

class QgsRasterLayer;
class QgsMessageBar;


/**
 * \ingroup gui
 * \brief The QgsLoadRasterAttributeTableDialog dialog let the user select a VAT.DBF file
 * and associate the resulting raster attribute table with a raster band.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsLoadRasterAttributeTableDialog: public QDialog, private Ui::QgsLoadRasterAttributeTableDialogBase
{
    Q_OBJECT

  public:

    /**
     * Creates a new QgsCreateRasterAttributeTableDialog.
     * \param rasterLayer the raster layer, must be suitable for creating a new raster attribute table
     * \param parent optional parent
     */
    QgsLoadRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent = nullptr );

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Sets the visibility of the "Open newly created raster attribute table" option to \a visible, the option is visible by default.
     */
    void setOpenWhenDoneVisible( bool visible );

    /**
     * Returns TRUE if the option to open the newly created attribute table is checked.
     */
    bool openWhenDone( ) const;

    /**
     * Returns the raster band associated to the raster attribute table.
     */
    int rasterBand( );

    /**
     * Returns the file path to VAT.DBF.
     */
    QString filePath( ) const;

    // QDialog interface
  public slots:

    void accept() override;

  private slots:

    void updateButtons();

    void notify( const QString &title, const QString &message, Qgis::MessageLevel level = Qgis::MessageLevel::Info );

  private:

    QgsRasterLayer *mRasterLayer = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSLOADRASTERATTRIBUTETABLEDIALOG_H
