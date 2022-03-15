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

class QgsPointCloudLayerRenderer;

#include "qgspointclouddataprovider.h"
#include "qgsmaplayer.h"
#include "qgis_core.h"

#include <QString>
#include <memory>

class QgsPointCloudRenderer;
class QgsPointCloudLayerElevationProperties;

/**
 * \ingroup core
 *
 * \brief Represents a map layer supporting display of point clouds
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
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext = QgsCoordinateTransformContext( ) )
        : transformContext( transformContext )
      {}

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

      //! Set to TRUE if the default layer style should be loaded
      bool loadDefaultStyle = true;

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

      /**
       * Set to TRUE if point cloud index generation should be skipped.
       */
      bool skipIndexGeneration = false;
    };


    /**
     * Constructor - creates a point cloud layer
     */
    explicit QgsPointCloudLayer( const QString &uri = QString(),
                                 const QString &baseName = QString(),
                                 const QString &providerLib = QStringLiteral( "pointcloud" ),
                                 const QgsPointCloudLayer::LayerOptions &options = QgsPointCloudLayer::LayerOptions() );

    ~QgsPointCloudLayer() override;

    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs ) = delete;
    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer &operator=( QgsPointCloudLayer const &rhs ) = delete;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointCloudLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QgsPointCloudLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    QgsPointCloudDataProvider *dataProvider() override;
    const QgsPointCloudDataProvider *dataProvider() const override SIP_SKIP;

    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;

    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    bool readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) FINAL;

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                         StyleCategories categories = AllStyleCategories ) const override;
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const FINAL;

    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;

    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const override;
    QString decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const override;
    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) FINAL;
    QString htmlMetadata() const override;
    QgsMapLayerElevationProperties *elevationProperties() override;

    /**
     * Returns the attributes available from the layer.
     */
    QgsPointCloudAttributeCollection attributes() const;

    /**
     * Returns the total number of points available in the layer.
     */
    qint64 pointCount() const;

    /**
     * Returns the 2D renderer for the point cloud.
     *
     * \see setRenderer()
     */
    QgsPointCloudRenderer *renderer();

    /**
     * Returns the 2D renderer for the point cloud.
     * \note not available in Python bindings
     *
     * \see setRenderer()
     */
    const QgsPointCloudRenderer *renderer() const SIP_SKIP;

    /**
     * Sets the 2D \a renderer for the point cloud.
     *
     * Ownership of \a renderer is transferred to the layer.
     *
     * \see renderer()
     */
    void setRenderer( QgsPointCloudRenderer *renderer SIP_TRANSFER );

    /**
     * Sets the string used to define a subset of the layer
     * \param subset The subset string to be used in a \a QgsPointCloudExpression
     * \returns TRUE, when setting the subset string was successful, FALSE otherwise
     *
     * \since QGIS 3.26
     */
    bool setSubsetString( const QString &subset );

    /**
     * Returns the string used to define a subset of the layer.
     * \returns The subset string or null QString if not implemented by the provider
     *
     * \since QGIS 3.26
     */
    QString subsetString() const;

  signals:

    /**
     * Emitted when the layer's subset string has changed.
     *
     * \since QGIS 3.26
     */
    void subsetStringChanged();

  private slots:
    void onPointCloudIndexGenerationStateChanged( QgsPointCloudDataProvider::PointCloudIndexGenerationState state );
    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags ) override;

  private:

    bool isReadOnly() const override {return true;}

#ifdef SIP_RUN
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs );
#endif

    std::unique_ptr<QgsPointCloudDataProvider> mDataProvider;

    std::unique_ptr<QgsPointCloudRenderer> mRenderer;

    QgsPointCloudLayerElevationProperties *mElevationProperties = nullptr;
};


#endif // QGSPOINTCLOUDPLAYER_H
