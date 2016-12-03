/***************************************************************************
  qgsfieldkit.cpp - QgsFieldKit

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfieldkit.h"

QgsFieldKit::QgsFieldKit()
{
}

QgsFieldKit::~QgsFieldKit()
{
}

bool QgsFieldKit::supportsField( QgsVectorLayer* layer, int fieldIdx )
{
  return true;
}

QString QgsFieldKit::representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return "1";
}

QVariant QgsFieldKit::sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return 1;
}

Qt::AlignmentFlag QgsFieldKit::alignmentFlag( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const
{
  return Qt::AlignLeft;
}

QVariant QgsFieldKit::createCache( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config ) const
{
  return QVariant();
}
