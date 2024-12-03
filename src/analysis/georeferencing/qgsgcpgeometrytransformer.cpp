/***************************************************************************
                             qgsgcpgeometrytransformer.cpp
                             ----------------------
    begin                : February 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsgcpgeometrytransformer.h"
#include "qgsgeometry.h"

QgsGcpGeometryTransformer::QgsGcpGeometryTransformer( QgsGcpTransformerInterface *gcpTransformer )
  : mGcpTransformer( gcpTransformer )
{
}

QgsGcpGeometryTransformer::QgsGcpGeometryTransformer( QgsGcpTransformerInterface::TransformMethod method, const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates )
  : mGcpTransformer( QgsGcpTransformerInterface::createFromParameters( method, sourceCoordinates, destinationCoordinates ) )
{
}

QgsGcpGeometryTransformer::~QgsGcpGeometryTransformer() = default;

bool QgsGcpGeometryTransformer::transformPoint( double &x, double &y, double &, double & )
{
  if ( !mGcpTransformer )
    return false;

  return mGcpTransformer->transform( x, y );
}

QgsGeometry QgsGcpGeometryTransformer::transform( const QgsGeometry &geometry, bool &ok, QgsFeedback *feedback )
{
  ok = false;
  if ( geometry.isNull() )
  {
    ok = true;
    return QgsGeometry();
  }

  std::unique_ptr<QgsAbstractGeometry> res( geometry.constGet()->clone() );

  ok = res->transform( this, feedback );

  return QgsGeometry( std::move( res ) );
}

QgsGcpTransformerInterface *QgsGcpGeometryTransformer::gcpTransformer() const
{
  return mGcpTransformer.get();
}

void QgsGcpGeometryTransformer::setGcpTransformer( QgsGcpTransformerInterface *gcpTransformer )
{
  mGcpTransformer.reset( gcpTransformer );
}
