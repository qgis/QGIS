/***************************************************************************
                         qgsmeshterraingenerator.h
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTERRAINGENERATOR_H
#define QGSMESHTERRAINGENERATOR_H

#include "qgsmesh3dsymbol.h"
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsterraingenerator.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Implementation of terrain generator that uses the Z values of a mesh layer to build a terrain
 * \since QGIS 3.12
 */
class _3D_EXPORT QgsMeshTerrainGenerator : public QgsTerrainGenerator
{
    Q_OBJECT
  public:
    /**
     * Creates a new instance of a QgsMeshTerrainGenerator object.
     */
    static QgsTerrainGenerator *create() SIP_FACTORY;

    //! Creates mesh terrain generator object
    QgsMeshTerrainGenerator();

    //! Returns the mesh layer to be used for terrain generation
    QgsMeshLayer *meshLayer() const;
    //! Sets the mesh layer to be used for terrain generation
    void setLayer( QgsMeshLayer *layer );

    //! Returns the symbol used to render the mesh as terrain
    QgsMesh3DSymbol *symbol() const;

    //! Sets the symbol used to render the mesh as terrain
    void setSymbol( QgsMesh3DSymbol *symbol SIP_TRANSFER );

    void setCrs( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &context ) override;

    QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override SIP_FACTORY;
    float rootChunkError( const Qgs3DMapSettings &map ) const override;
    void rootChunkHeightRange( float &hMin, float &hMax ) const override;
    QgsTerrainGenerator *clone() const override SIP_FACTORY;
    Type type() const override;
    QgsRectangle rootChunkExtent() const override;
    float heightAt( double x, double y, const Qgs3DRenderContext &context ) const override;

  private slots:
    void updateTriangularMesh();

  private:
    QPointer<QgsMeshLayer> mLayer;
    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateTransformContext mTransformContext;
    std::unique_ptr<QgsMesh3DSymbol> mSymbol;
    QgsTriangularMesh mTriangularMesh;
};

#endif // QGSMESHTERRAINGENERATOR_H
