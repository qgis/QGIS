/***************************************************************************
    qgsgrouplayer.h
    ----------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGROUPLAYER_H
#define QGSGROUPLAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerref.h"

class QgsGroupLayerDataProvider;
class QgsPaintEffect;

/**
 * \ingroup core
 *
 * \brief A map layer which consists of a set of child layers, where all component layers are rendered as a single
 * flattened object during map renders.
 *
 * Child layers are never owned by QgsGroupLayer. References to layers in a group are stored as weak
 * references only, which wills be automatically cleaned up whenever the linked child layer is deleted.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsGroupLayer : public QgsMapLayer
{
    Q_OBJECT

  public:

    /**
     * Setting options for loading group layers.
     * \since QGIS 3.16
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext )
        : transformContext( transformContext )
      {}

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

    };

    /**
     * Constructor for a new QgsGroupLayer with the specified layer \a name.
     *
     * The \a options argument specifies load-time layer options.
     */
    QgsGroupLayer( const QString &name, const QgsGroupLayer::LayerOptions &options );
    ~QgsGroupLayer() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsGroupLayer: '%1'>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QgsGroupLayer *clone() const override SIP_FACTORY;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;
    QgsRectangle extent() const override;
    void setTransformContext( const QgsCoordinateTransformContext &context ) override;
    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &, StyleCategories categories = AllStyleCategories ) const override;
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;
    QgsDataProvider *dataProvider() override;
    const QgsDataProvider *dataProvider() const override SIP_SKIP;
    QString htmlMetadata() const override;
    void resolveReferences( QgsProject *project ) override;

    /**
     * Sets the child \a layers contained by the group.
     *
     * This method does not take ownership of the layers, but rather assigns them to the group. Layers should be already added to
     * the parent QgsProject wherever appropriate.
     *
     * \see childLayers()
    */
    void setChildLayers( const QList< QgsMapLayer * > &layers );

    /**
     * Returns the child layers contained by the group.
     *
     * \see setChildLayers()
     */
    QList< QgsMapLayer * > childLayers() const;

    /**
     * Returns the current paint effect for the group layer.
     * \see setPaintEffect()
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint \a effect for the renderer.
     *
     * Ownership is transferred to the renderer.
     *
     * \see paintEffect()
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

  private:

    QgsGroupLayerDataProvider *mDataProvider = nullptr;
    QgsCoordinateTransformContext mTransformContext;

    QList< QgsMapLayerRef > mChildren;
    std::unique_ptr< QgsPaintEffect > mPaintEffect;

};

#ifndef SIP_RUN
///@cond PRIVATE

/**
 * A minimal data provider for group layers.
 *
 * \since QGIS 3.24
 */
class QgsGroupLayerDataProvider : public QgsDataProvider
{
    Q_OBJECT

  public:
    QgsGroupLayerDataProvider( const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags );
    void setCrs( const QgsCoordinateReferenceSystem &crs );
    QgsCoordinateReferenceSystem crs() const override;
    QString name() const override;
    QString description() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;

  private:

    QgsCoordinateReferenceSystem mCrs;

};
///@endcond
#endif

#endif // QGSGROUPLAYER_H
