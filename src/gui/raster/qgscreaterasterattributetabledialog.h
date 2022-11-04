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
#include "qgis.h"
#include "ui_qgscreaterasterattributetabledialogbase.h"
#include <QDialog>

#define SIP_NO_FILE

class QgsRasterLayer;
class QgsMessageBar;


/**
 * \ingroup gui
 * \brief The QgsCreateRasterAttributeTableDialog dialog collects the information required to create a new raster attribute table and performs the creation when the dialog is accepted.
 * \warning Client code must check if the creation of attribute tables is supported by the raster layer by calling QgsRasterLayer::canCreateAttributeTable() before using this dialog.
 * \note Not available in Python bindings
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
    QgsCreateRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent = nullptr );

    /**
     * Returns the file path in case of VAT.DBF save option.
     */
    QString filePath( ) const;

    /**
     * Returns TRUE if the option to save to a file is selected.
     */
    bool saveToFile( ) const;

    /**
     * Returns TRUE if the option to open the newly created attribute table is checked.
     */
    bool openWhenDone( ) const;

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Sets the visibility of the "Open newly created raster attribute table" option to \a visible, the option is visible by default.
     */
    void setOpenWhenDoneVisible( bool visible );

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

#endif // QGSCREATERASTERATTRIBUTETABLEDIALOG_H
