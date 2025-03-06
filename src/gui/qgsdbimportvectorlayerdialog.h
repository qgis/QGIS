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
    QgsDbImportVectorLayerDialog( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent = nullptr );
    ~QgsDbImportVectorLayerDialog() override;

    void setDestinationSchema( const QString &schema );

    void setSourceUri( const QgsMimeDataUtils::Uri &uri );

    QString schema() const;
    QString tableName() const;
    QString tableComment() const;

    std::unique_ptr<QgsVectorLayerExporterTask> createExporterTask( const QVariantMap &extraProviderOptions = QVariantMap() );

    void setSourceLayer( QgsVectorLayer *layer );
  private slots:
    void sourceLayerComboChanged();
    void doImport();

  private:
    std::unique_ptr< QgsVectorLayer > mOwnedSource;
    QPointer< QgsVectorLayer > mSourceLayer;

    int mBlockSourceLayerChanges = 0;

    std::unique_ptr< QgsAbstractDatabaseProviderConnection > mConnection;
};

#endif // QGSDBIMPORTVECTORLAYERDIALOG_H
