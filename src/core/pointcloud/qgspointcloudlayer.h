/***************************************************************************
                         qgspointcloudlayer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYER_H
#define QGSPOINTCLOUDLAYER_H

class QgsPointCloudRenderer;

#include "qgspointclouddataprovider.h"
#include "qgsmaplayer.h"
#include "qgis_core.h"

#include <QString>
#include <memory>

/**
 * \ingroup core
 *
 * Represents a map layer supporting display of point clouds
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudLayer : public QgsMapLayer
{
    Q_OBJECT
  public:

    /**
     * Setting options for loading point cloud layers.
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions with optional \a transformContext.
       * \note transformContext argument was added in QGIS 3.8
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext( ) )
        : transformContext( transformContext )
      {}

      QgsCoordinateTransformContext transformContext;

      /**
       * Controls whether the layer is allowed to have an invalid/unknown CRS.
       *
       * If TRUE, then no validation will be performed on the layer's CRS and the layer
       * layer's crs() may be invalid() (i.e. the layer will have no georeferencing available
       * and will be treated as having purely numerical coordinates).
       *
       * If FALSE (the default), the layer's CRS will be validated using QgsCoordinateReferenceSystem::validate(),
       * which may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the
       * layer.
       */
      bool skipCrsValidation = false;
    };


    /**
     * Constructor - creates a point cloud layer
     */
    explicit QgsPointCloudLayer( const QString &path = QString(),
                                 const QString &baseName = QString(),
                                 const QString &providerLib = QStringLiteral( "pointcloud" ),
                                 const QgsPointCloudLayer::LayerOptions &options = QgsPointCloudLayer::LayerOptions() );

    ~QgsPointCloudLayer() override;

    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs ) = delete;
    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer &operator=( QgsPointCloudLayer const &rhs ) = delete;

    QgsPointCloudLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    QgsPointCloudDataProvider *dataProvider() override;
    const QgsPointCloudDataProvider *dataProvider() const override SIP_SKIP;

    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;

    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                         StyleCategories categories = AllStyleCategories ) const override;

    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;
    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) override;

  private: // Private methods
    bool loadDataSource( const QString &providerLib, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags );

    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

#ifdef SIP_RUN
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs );
#endif

    std::unique_ptr<QgsPointCloudDataProvider> mDataProvider;

    //! Renderer assigned to the layer to draw map
    std::unique_ptr<QgsPointCloudRenderer> mRenderer;
};


#endif // QGSPOINTCLOUDPLAYER_H
