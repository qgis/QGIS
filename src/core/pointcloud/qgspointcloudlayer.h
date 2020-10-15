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

class QgsPointCloudIndex;
class QgsPointCloudRenderer;

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
     * Constructor - creates a point cloud layer
     */
    explicit QgsPointCloudLayer( const QString &path = QString(), const QString &baseName = QString() );

    ~QgsPointCloudLayer() override;

    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs ) = delete;
    //! QgsPointCloudLayer cannot be copied.
    QgsPointCloudLayer &operator=( QgsPointCloudLayer const &rhs ) = delete;

    QgsPointCloudLayer *clone() const override SIP_FACTORY;
    QgsRectangle extent() const override;
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    bool readXml( const QDomNode &layerNode, QgsReadWriteContext &context ) override;

    bool writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

    bool readSymbology( const QDomNode &node, QString &errorMessage,
                        QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override;

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                         StyleCategories categories = AllStyleCategories ) const override;

    void setTransformContext( const QgsCoordinateTransformContext &transformContext ) override;
    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) override;

    QgsPointCloudIndex *pointCloudIndex() const SIP_SKIP;

  private: // Private methods
    bool loadDataSource();

    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

#ifdef SIP_RUN
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs );
#endif

    QgsPointCloudIndex *mPointCloudIndex = nullptr;

    //! Renderer assigned to the layer to draw map
    std::unique_ptr<QgsPointCloudRenderer> mRenderer;
};


#endif // QGSPOINTCLOUDPLAYER_H
