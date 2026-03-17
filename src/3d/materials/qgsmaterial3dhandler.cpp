/***************************************************************************
  qgsmaterial3dhandler.cpp
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterial3dhandler.h"

#include <QString>

using namespace Qt::StringLiterals;


QByteArray QgsAbstractMaterial3DHandler::dataDefinedVertexColorsAsByte( const QgsAbstractMaterialSettings *settings, const QgsExpressionContext &expressionContext ) const
{
  Q_UNUSED( settings )
  Q_UNUSED( expressionContext )
  return QByteArray();
}

void QgsAbstractMaterial3DHandler::applyDataDefinedToGeometry( const QgsAbstractMaterialSettings *settings, Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const
{
  Q_UNUSED( settings )
  Q_UNUSED( geometry )
  Q_UNUSED( vertexCount )
  Q_UNUSED( dataDefinedBytes )
}

int QgsAbstractMaterial3DHandler::dataDefinedByteStride( const QgsAbstractMaterialSettings * ) const
{
  return 0;
}
