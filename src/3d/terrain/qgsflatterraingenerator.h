/***************************************************************************
  qgsflatterraingenerator.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFLATTERRAINGENERATOR_H
#define QGSFLATTERRAINGENERATOR_H

#include "qgis_3d.h"
#include "qgsterraingenerator.h"

#include <Qt3DExtras/QPlaneGeometry>

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief Terrain generator that creates a simple square flat area.
 *
 */
class _3D_EXPORT QgsFlatTerrainGenerator : public QgsTerrainGenerator
{
    Q_OBJECT
  public:
    /**
     * Creates a new instance of a QgsFlatTerrainGenerator object.
     */
    static QgsTerrainGenerator *create() SIP_FACTORY;

    QgsFlatTerrainGenerator() = default;

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;

    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle rootChunkExtent() const override;
    void setExtent( const QgsRectangle &extent ) override;
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    float heightAt( double x, double y, const Qgs3DRenderContext &context ) const override;

    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context ) override;
    QgsCoordinateReferenceSystem crs() const override { return mCrs; }

  private:
    void updateTilingScheme();

    QgsCoordinateReferenceSystem mCrs;
};


#endif // QGSFLATTERRAINGENERATOR_H
