/***************************************************************************
                         qgstiledmeshlayer.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSTILEDMESHLAYER_H
#define QGSTILEDMESHLAYER_H

#include "qgis_core.h"
#include "qgsmaplayer.h"
#include "qgstiledmeshdataprovider.h"

/**
 * \ingroup core
 *
 * \brief Represents a map layer supporting display of tiled mesh objects.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshLayer : public QgsMapLayer
{
    Q_OBJECT

  public:

    /**
     * Setting options for loading tiled mesh layers.
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
    };

    /**
     * Constructor for QgsTiledMeshLayer.
     */
    explicit QgsTiledMeshLayer( const QString &uri = QString(),
                                const QString &baseName = QString(),
                                const QString &provider = QString(),
                                const QgsTiledMeshLayer::LayerOptions &options = QgsTiledMeshLayer::LayerOptions() );

    ~QgsTiledMeshLayer() override;

    //! QgsTiledMeshLayer cannot be copied.
    QgsTiledMeshLayer( const QgsTiledMeshLayer &other ) = delete;
    //! QgsTiledMeshLayer cannot be copied.
    QgsTiledMeshLayer &operator=( QgsTiledMeshLayer const &other ) = delete;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsTiledMeshLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QgsTiledMeshLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsTiledMeshDataProvider *dataProvider() override;
    const QgsTiledMeshDataProvider *dataProvider() const override SIP_SKIP;
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
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

  private slots:
    void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags ) override;

  private:

    bool isReadOnly() const override;

#ifdef SIP_RUN
    QgsTiledMeshLayer( const QgsTiledMeshLayer &rhs );
#endif

    std::unique_ptr<QgsTiledMeshDataProvider> mDataProvider;

    LayerOptions mLayerOptions;
};


#endif // QGSTILEDMESHLAYER_H
