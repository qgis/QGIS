/***************************************************************************
                     qgsogrprovidermetadata.h
begin                : June 2021
copyright            : (C) 2021 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDERMETADATA_H
#define QGSOGRPROVIDERMETADATA_H

#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * Entry point for registration of the OGR data provider
 * \since QGIS 3.10
 */
class QgsOgrProviderMetadata final: public QgsProviderMetadata
{
  public:

    QgsOgrProviderMetadata();

    void initProvider() override;
    void cleanupProvider() override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QString filters( FilterType type ) override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    ProviderCapabilities providerCapabilities() const override;
    bool uriIsBlocklisted( const QString &uri ) const override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    QStringList sidecarFilesForUri( const QString &uri ) const override;
    Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> &oldToNewAttrIdxMap,
      QString &errorMessage,
      const QMap<QString, QVariant> *options ) override;

    // -----
    bool styleExists( const QString &uri, const QString &styleId, QString &errorCause ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                    const QString &styleName, const QString &styleDescription,
                    const QString &uiFileContent, bool useAsDefault, QString &errCause ) override;
    bool deleteStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                    QStringList &descriptions, QString &errCause ) override;
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    bool saveLayerMetadata( const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage ) final;

    // -----
    QgsTransaction *createTransaction( const QString &connString ) override;

    // QgsProviderMetadata interface
  public:
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) override;

  protected:

    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;

};

///@endcond
#endif // QGSOGRPROVIDERMETADATA_H
