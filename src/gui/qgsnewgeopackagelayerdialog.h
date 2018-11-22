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

/**
 * \ingroup gui
 * Dialog to set up parameters to create a new GeoPackage layer, and on accept() to create it and add it to the layers */
class GUI_EXPORT QgsNewGeoPackageLayerDialog: public QDialog, private Ui::QgsNewGeoPackageLayerDialogBase
{
    Q_OBJECT

  public:

    //! Behavior to use when an existing geopackage already exists
    enum OverwriteBehavior
    {
      Prompt, //!< Prompt user for action
      Overwrite, //!< Overwrite whole geopackage
      AddNewLayer, //!< Keep existing contents and add new layer
    };

    //! Constructor
    QgsNewGeoPackageLayerDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets the \a crs value for the new layer in the dialog.
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the database path
     * \since QGIS 3.0
     */
    QString databasePath() const { return mDatabase->filePath(); }

    /**
     * Sets the initial database \a path
     * \since QGIS 3.0
     */
    void setDatabasePath( const QString &path ) { mDatabase->setFilePath( path ); }

    /**
     * Sets the database path widgets to a locked and read-only mode.
     * \since QGIS 3.0
     */
    void lockDatabasePath();

    /**
     * Sets the \a behavior to use when a path to an existing geopackage file is used.
     *
     * The default behavior is to prompt the user for an action to take.
     *
     * \since QGIS 3.0
     */
    void setOverwriteBehavior( OverwriteBehavior behavior );

    /**
     * Sets whether a newly created layer should automatically be added to the current project.
     * Defaults to true.
     *
     * \since QGIS 3.6
     */
    void setAddToProject( bool addToProject );

  private slots:
    void mAddAttributeButton_clicked();
    void mRemoveAttributeButton_clicked();
    void mFieldTypeBox_currentIndexChanged( int index );
    void mGeometryTypeBox_currentIndexChanged( int index );
    void mTableNameEdit_textChanged( const QString &text );
    void mTableNameEdit_textEdited( const QString &text );
    void mLayerIdentifierEdit_textEdited( const QString &text );
    void fieldNameChanged( const QString & );
    void selectionChanged();
    void checkOk();

    void showHelp();
    void buttonBox_accepted();
    void buttonBox_rejected();

  private:
    bool apply();

    QPushButton *mOkButton = nullptr;
    QString mCrsId;
    bool mTableNameEdited = false;
    bool mLayerIdentifierEdited = false;
    OverwriteBehavior mBehavior = Prompt;
    bool mAddToProject = true;
};

#endif // QGSNEWVECTORLAYERDIALOG_H
