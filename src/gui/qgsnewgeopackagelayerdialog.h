/***************************************************************************
                         qgsnewgeopackagelayerdialog.h
                             -------------------
    begin                : April 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWGEOPACKAGELAYERDIALOG_H
#define QGSNEWGEOPACKAGELAYERDIALOG_H

#include "ui_qgsnewgeopackagelayerdialogbase.h"
#include "qgsguiutils.h"

#include "qgis.h"
#include "qgis_gui.h"

/** \ingroup gui
 * Dialog to set up parameters to create a new GeoPackage layer, and on accept() to create it and add it to the layers */
class GUI_EXPORT QgsNewGeoPackageLayerDialog: public QDialog, private Ui::QgsNewGeoPackageLayerDialogBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsNewGeoPackageLayerDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    ~QgsNewGeoPackageLayerDialog();

    /**
     * Sets the \a crs value for the new layer in the dialog.
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the database path
     * \since QGIS 3.0
     */
    QString databasePath() const { return mDatabaseEdit->text(); }

    /**
     * Sets the the database \a path
     * \since QGIS 3.0
     */
    void setDatabasePath( const QString &path ) { mDatabaseEdit->setText( path ); }

  private slots:
    void on_mAddAttributeButton_clicked();
    void on_mRemoveAttributeButton_clicked();
    void on_mFieldTypeBox_currentIndexChanged( int index );
    void on_mGeometryTypeBox_currentIndexChanged( int index );
    void on_mSelectDatabaseButton_clicked();
    void on_mDatabaseEdit_textChanged( const QString &text );
    void on_mTableNameEdit_textChanged( const QString &text );
    void on_mTableNameEdit_textEdited( const QString &text );
    void on_mLayerIdentifierEdit_textEdited( const QString &text );
    void fieldNameChanged( const QString & );
    void selectionChanged();
    void checkOk();

    void showHelp();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

  private:
    bool apply();

    QPushButton *mOkButton = nullptr;
    QString mCrsId;
    bool mTableNameEdited;
    bool mLayerIdentifierEdited;
};

#endif // QGSNEWVECTORLAYERDIALOG_H
