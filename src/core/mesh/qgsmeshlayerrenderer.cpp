/***************************************************************************
                         qgsmeshlayerrenderer.cpp
                         ------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
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

#include "qgsmeshlayerrenderer.h"

#include "qgsrenderer.h"
#include "qgsrendercontext.h"
#include "qgsmeshlayer.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfield.h"
#include "qgstriangularmesh.h"
#include "qgspointxy.h"

#include <QPicture>


QgsMeshLayerRenderer::QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id() )
  , mContext( context )
{
  // make copies for mesh data
  Q_ASSERT( layer->nativeMesh() );
  Q_ASSERT( layer->triangularMesh() );
  mNativeMesh = *( layer->nativeMesh() );
  mTriangularMesh = *( layer->triangularMesh() );

  // make copies for symbols
  if ( layer->nativeMeshSymbol() )
  {
    mNativeMeshSymbol = layer->nativeMeshSymbol()->clone();
  }

  if ( layer->triangularMeshSymbol() )
  {
    mTriangularMeshSymbol = layer->triangularMeshSymbol()->clone();
  }
}


QgsMeshLayerRenderer::~QgsMeshLayerRenderer()
{
  if ( mNativeMeshSymbol )
    delete mNativeMeshSymbol;

  if ( mTriangularMeshSymbol )
    delete mTriangularMeshSymbol;
}



bool QgsMeshLayerRenderer::render()
{
  renderMesh( mNativeMeshSymbol, mNativeMesh.faces ); // native mesh
  renderMesh( mTriangularMeshSymbol, mTriangularMesh.triangles() ); // triangular mesh

  return true;
}

void QgsMeshLayerRenderer::renderMesh( QgsSymbol *symbol, const QVector<QgsMeshFace> &faces )
{
  if ( !symbol )
    return;

  QgsFields fields;
  QgsSingleSymbolRenderer renderer( symbol->clone() );
  renderer.startRender( mContext, fields );

  for ( int i = 0; i < faces.size(); ++i )
  {
    if ( mContext.renderingStopped() )
      break;

    const QgsMeshFace &face = faces[i];
    QgsFeature feat;
    feat.setFields( fields );
    QVector<QgsPointXY> ring;
    for ( int j = 0; j < face.size(); ++j )
    {
      int vertex_id = face[j];
      Q_ASSERT( vertex_id < mTriangularMesh.vertices().size() ); //Triangular mesh vertices contains also native mesh vertices
      const QgsPoint &vertex = mTriangularMesh.vertices()[vertex_id];
      ring.append( vertex );
    }
    QgsPolygonXY polygon;
    polygon.append( ring );
    QgsGeometry geom = QgsGeometry::fromPolygonXY( polygon );
    feat.setGeometry( geom );
    renderer.renderFeature( feat, mContext );
  }

  renderer.stopRender( mContext );
}
