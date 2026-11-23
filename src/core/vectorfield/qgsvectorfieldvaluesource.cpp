/***************************************************************************
    qgsvectorfieldvaluesource.cpp
    -----------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldvaluesource.h"

#include "qgsrasterinterface.h"

QgsVectorFieldValueSource::~QgsVectorFieldValueSource() = default;

QVector<QgsPointXY> QgsVectorFieldValueSource::seedPoints( const QgsRectangle &extent ) const
{
  Q_UNUSED( extent )
  return {};
}

bool QgsVectorFieldValueSource::nativeLayout( QgsPointXY &origin, double &spacingX, double &spacingY ) const
{
  Q_UNUSED( origin )
  Q_UNUSED( spacingX )
  Q_UNUSED( spacingY )
  return false;
}

void QgsVectorFieldValueSource::setSamplingWindow( double width, double height )
{
  Q_UNUSED( width )
  Q_UNUSED( height )
}

std::unique_ptr<QgsRasterInterface> QgsVectorFieldValueSource::magnitudeSource( const QgsRenderContext &context, QSize size ) const
{
  Q_UNUSED( context )
  Q_UNUSED( size )
  return nullptr;
}
