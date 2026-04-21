/***************************************************************************
  qgsnullmaterial3dhandler.cpp
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnullmaterial3dhandler.h"

#include "qgshighlightmaterial.h"
#include "qgsmaterial.h"

#include <QMap>
#include <QString>

using namespace Qt::StringLiterals;


QgsMaterial *QgsNullMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  if ( context.isHighlighted() )
    return new QgsHighlightMaterial( technique );

  return nullptr;
}

QMap<QString, QString> QgsNullMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

void QgsNullMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{}

bool QgsNullMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{
  return true;
}
