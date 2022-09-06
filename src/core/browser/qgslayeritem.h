/***************************************************************************
                             qgslayeritem.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERITEM_H
#define QGSLAYERITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgsdataitem.h"
#include "qgslayermetadata.h"

/**
 * \ingroup core
 * \brief Item that represents a layer that can be opened with one of the providers
*/
class CORE_EXPORT QgsLayerItem : public QgsDataItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayerItem.
     */
    QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType, const QString &providerKey );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayerItem: \"%1\" %2>" ).arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    // --- reimplemented from QgsDataItem ---

    bool equal( const QgsDataItem *other ) override;

    bool hasDragEnabled() const override { return true; }

    QgsMimeDataUtils::UriList mimeUris() const override;

    // --- New virtual methods for layer item derived classes ---

    //! Returns QgsMapLayerType
    QgsMapLayerType mapLayerType() const;

    /**
     * Returns the layer item type corresponding to a QgsMapLayer \a layer.
     * \since QGIS 3.6
     */
    static Qgis::BrowserLayerType typeFromMapLayer( QgsMapLayer *layer );

    //! Returns layer uri or empty string if layer cannot be created
    QString uri() const { return mUri; }

    //! Returns provider key
    QString providerKey() const { return mProviderKey; }

    /**
     * Returns the supported CRS
     *  \since QGIS 2.8
     */
    QStringList supportedCrs() const { return mSupportedCRS; }

    /**
     * Returns the supported formats
     *  \since QGIS 2.8
     */
    QStringList supportedFormats() const { return mSupportFormats; }

    /**
     * Returns comments of the layer
     * \since QGIS 2.12
     */
    virtual QString comments() const { return QString(); }

    /**
     * Returns the string representation of the given \a layerType
     * \since QGIS 3
     */
    static QString layerTypeAsString( Qgis::BrowserLayerType layerType );

    /**
     * Returns the icon name of the given \a layerType
     * \since QGIS 3
     */
    static QString iconName( Qgis::BrowserLayerType layerType );

    /**
     * Delete this layer item
     * Use QgsDataItemGuiProvider::deleteLayer instead
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool deleteLayer() SIP_DEPRECATED;

  protected:
    //! The URI
    QString mUri;
    //! The layer type
    Qgis::BrowserLayerType mLayerType;
    //! The list of supported CRS
    QStringList mSupportedCRS;
    //! The list of supported formats
    QStringList mSupportFormats;

  public:

    /**
     * Returns the icon for a vector layer whose geometry type is provided.
     * \since QGIS 3.18
     */
    static QIcon iconForWkbType( QgsWkbTypes::Type type );

    static QIcon iconPoint();
    static QIcon iconLine();
    static QIcon iconPolygon();
    static QIcon iconTable();
    static QIcon iconRaster();
    static QIcon iconDefault();
    //! Returns icon for mesh layer type
    static QIcon iconMesh();
    //! Returns icon for vector tile layer
    static QIcon iconVectorTile();
    //! Returns icon for point cloud layer
    static QIcon iconPointCloud();
    //! \returns the layer name
    virtual QString layerName() const { return name(); }
    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

    /**
     * Returns layer's metadata, it may be a default constructed metadata
     * if metadata is not explicitly set.
     * \since QGIS 3.28
     */
    const QgsLayerMetadata &layerMetadata() const;

    /**
     * Set layer's \a metadata.
     * \since QGIS 3.28
     */
    void setLayerMetadata( const QgsLayerMetadata &metadata );

  private:

    QgsLayerMetadata mLayerMetadata;

};

#endif // QGSLAYERITEM_H


