/***************************************************************************
  qgs3dapputils.cpp
  --------------------------------------
  Date                 : July 2020
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

#include "qgs3dapputils.h"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgs3d.h"
#include "qgsmaterialregistry.h"
#include "qgspoint3dsymbolwidget.h"
#include "qgspolygon3dsymbolwidget.h"
#include "qgsline3dsymbolwidget.h"
#include "qgsphongmaterialwidget.h"
#include "qgsphongtexturedmaterialwidget.h"
#include "qgsgoochmaterialwidget.h"
#include "qgssimplelinematerialwidget.h"
#include "qgsnullmaterialwidget.h"
#include "qgs3dicongenerator.h"

void Qgs3DAppUtils::initialize()
{
  qgis::down_cast< Qgs3DSymbolMetadata * >( QgsApplication::symbol3DRegistry()->symbolMetadata( QStringLiteral( "point" ) ) )->setWidgetFunction( QgsPoint3DSymbolWidget::create );
  qgis::down_cast< Qgs3DSymbolMetadata * >( QgsApplication::symbol3DRegistry()->symbolMetadata( QStringLiteral( "line" ) ) )->setWidgetFunction( QgsLine3DSymbolWidget::create );
  qgis::down_cast< Qgs3DSymbolMetadata * >( QgsApplication::symbol3DRegistry()->symbolMetadata( QStringLiteral( "polygon" ) ) )->setWidgetFunction( QgsPolygon3DSymbolWidget::create );

  qgis::down_cast< QgsMaterialSettingsMetadata * >( Qgs3D::materialRegistry()->materialSettingsMetadata( QStringLiteral( "null" ) ) )->setWidgetFunction( QgsNullMaterialWidget::create );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( Qgs3D::materialRegistry()->materialSettingsMetadata( QStringLiteral( "phong" ) ) )->setWidgetFunction( QgsPhongMaterialWidget::create );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( Qgs3D::materialRegistry()->materialSettingsMetadata( QStringLiteral( "phongtextured" ) ) )->setWidgetFunction( QgsPhongTexturedMaterialWidget::create );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( Qgs3D::materialRegistry()->materialSettingsMetadata( QStringLiteral( "gooch" ) ) )->setWidgetFunction( QgsGoochMaterialWidget::create );
  qgis::down_cast< QgsMaterialSettingsMetadata * >( Qgs3D::materialRegistry()->materialSettingsMetadata( QStringLiteral( "simpleline" ) ) )->setWidgetFunction( QgsSimpleLineMaterialWidget::create );

  QgsStyleModel::setIconGenerator( new Qgs3DIconGenerator( QgsApplication::defaultStyleModel() ) );
}
