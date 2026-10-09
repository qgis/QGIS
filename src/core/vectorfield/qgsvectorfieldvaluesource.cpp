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

std::unique_ptr<QgsRasterInterface> QgsVectorFieldValueSource::magnitudeSource( const QgsRenderContext &context, QSize size ) const
{
  Q_UNUSED( context )
  Q_UNUSED( size )
  return nullptr;
}
