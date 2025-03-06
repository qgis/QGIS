/***************************************************************************
  qgsdbimportvectorlayerdialog.h
  --------------------------------------
  Date                 : March 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDBIMPORTVECTORLAYERDIALOG_H
#define QGSDBIMPORTVECTORLAYERDIALOG_H

#include "qgis_gui.h"
#include "qgis.h"
#include "qgsmimedatautils.h"
#include "ui_qgsdbimportvectorlayerdialog.h"

class QgsAbstractDatabaseProviderConnection;
class QgsVectorLayerExporterTask;

#define SIP_NO_FILE

/**
 * \class QgsDbImportVectorLayerDialog
 * \ingroup gui
 *
 * \brief A generic dialog for customising vector layer import options for database connections.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsDbImportVectorLayerDialog : public QDialog, private Ui::QgsDbImportVectorLayerDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDbImportVectorLayerDialog.
     *
     * Ownership of \a connection is transferred to the dialog.
     */
    QgsDbImportVectorLayerDialog( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent = nullptr );
    ~QgsDbImportVectorLayerDialog() override;

    /**
     * Sets the destination \a schema for the new table.
     */
    void setDestinationSchema( const QString &schema );

    /**
     * Sets the source table \a uri.
     */
    void setSourceUri( const QgsMimeDataUtils::Uri &uri );

    /**
     * Returns the destination schema.
     */
    QString schema() const;

    /**
     * Returns the destination table name.
     */
    QString tableName() const;

    /**
     * Returns the optional comment to use for the new table.
     */
    QString tableComment() const;

    /**
     * Creates a new exporter task to match the settings defined in the dialog.
     */
    std::unique_ptr<QgsVectorLayerExporterTask> createExporterTask( const QVariantMap &extraProviderOptions = QVariantMap() );

  private slots:
    void sourceLayerComboChanged();
    void doImport();
    void setSourceLayer( QgsVectorLayer *layer );

  private:
    std::unique_ptr< QgsVectorLayer > mOwnedSource;
    QPointer< QgsVectorLayer > mSourceLayer;

    int mBlockSourceLayerChanges = 0;

    std::unique_ptr< QgsAbstractDatabaseProviderConnection > mConnection;
};

#endif // QGSDBIMPORTVECTORLAYERDIALOG_H
