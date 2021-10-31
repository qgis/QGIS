/***************************************************************************
           qgsvirtuallayerprovider.h Virtual layer data provider
begin                : Jan, 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUAL_LAYER_PROVIDER_H
#define QGSVIRTUAL_LAYER_PROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsconfig.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsvirtuallayerdefinition.h"
#include "qgsvirtuallayersqlitehelper.h"

#include "qgsprovidermetadata.h"

class QgsVirtualLayerFeatureIterator;

class QgsVirtualLayerProvider final: public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    static const QString VIRTUAL_LAYER_KEY;
    static const QString VIRTUAL_LAYER_DESCRIPTION;
    static const QString VIRTUAL_LAYER_QUERY_VIEW;

    /**
     * Constructor of the vector provider
     * \param uri uniform resource locator (URI) for a dataset
     * \param options generic data provider options
     */
    explicit QgsVirtualLayerProvider( QString const &uri, const ProviderOptions &coordinateTransformContext, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QgsRectangle extent() const override;
    QString subsetString() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    QgsFields fields() const override;
    bool isValid() const override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QString name() const override;
    QString description() const override;
    QgsAttributeList pkAttributeIndexes() const override;
    QSet<QgsMapLayerDependency> dependencies() const override;
    bool cancelReload() override;

    static QString providerKey();

  private:

    // file on disk
    QString mPath;

    QgsScopedSqlite mSqlite;

    // underlying vector layers
    struct SourceLayer
    {
      SourceLayer() = default;
      SourceLayer( QgsVectorLayer *l, const QString &n = QString() )
        : layer( l )
        , name( n )
      {}
      SourceLayer( const QString &p, const QString &s, const QString &n, const QString &e = QStringLiteral( "UTF-8" ) )
        : name( n )
        , source( s )
        , provider( p )
        , encoding( e )
      {}
      // non-null if it refers to a live layer
      QgsVectorLayer *layer = nullptr;
      QString name;
      // non-empty if it is an embedded layer
      QString source;
      QString provider;
      QString encoding;
    };
    typedef QVector<SourceLayer> SourceLayers;
    SourceLayers mLayers;


    bool mValid = true;

    QString mTableName;

    QgsCoordinateReferenceSystem mCrs;

    QgsVirtualLayerDefinition mDefinition;

    QString mSubset;

    void resetSqlite();

    mutable bool mCachedStatistics = false;
    mutable qint64 mFeatureCount = 0;
    mutable QgsRectangle mExtent;

    void updateStatistics() const;

    bool openIt();
    bool createIt();
    bool loadSourceLayers();
    void createVirtualTable( QgsVectorLayer *vlayer, const QString &name );

    /**
     * Opens or creates file
    */
    void reloadProviderData() override;

    friend class QgsVirtualLayerFeatureSource;

  private slots:
    void invalidateStatistics();

};

class QgsVirtualLayerProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsVirtualLayerProviderMetadata();
    QgsVirtualLayerProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
};

// clazy:excludeall=qstring-allocations

#endif
