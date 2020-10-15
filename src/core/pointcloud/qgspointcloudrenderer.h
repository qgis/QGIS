/***************************************************************************
                         qgspointcloudrenderer.h
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

#ifndef QGSPOINTCLOUDRENDERER_H
#define QGSPOINTCLOUDRENDERER_H

#include "qgis_core.h"
#include "qgsmaplayerrenderer.h"
#include "qgsreadwritecontext.h"

#include <QDomElement>
#include <QString>

class QgsRenderContext;
class QgsPointCloudLayer;

/**
 * \ingroup core
 *
 * Represents a 2D renderer of point cloud data
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRenderer: public QgsMapLayerRenderer
{
  public:

    explicit QgsPointCloudRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context );
    ~QgsPointCloudRenderer();

    bool render() override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

  protected:
    QgsPointCloudLayer *mLayer = nullptr;
};


#endif // QGSPOINTCLOUDRENDERER_H
