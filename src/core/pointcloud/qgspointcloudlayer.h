/***************************************************************************
                         qgspointcloudlayer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Martin Dobias and Peter Petrik
    email                : wonder dot sk at gmail dot com, zilolv at gmail dot com
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

#include "qgsmaplayer.h"

#include <QString>

class QgsPointCloudIndex;

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

    //! Returns the provider type for this layer
    QString providerType() const;

    QgsPointCloudIndex *pointCloudIndex() const SIP_SKIP;

  private: // Private methods
    /**
     * Returns TRUE if the provider is in read-only mode
     */
    bool isReadOnly() const override {return true;}

#ifdef SIP_RUN
    QgsPointCloudLayer( const QgsPointCloudLayer &rhs );
#endif

    QgsPointCloudIndex* mPointCloudIndex = nullptr;
};


#endif // QGSPOINTCLOUDPLAYER_H
