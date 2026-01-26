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

#include "qgs3d.h"
#include "qgs3dicongenerator.h"
#include "qgs3dsymbolregistry.h"
#include "qgsapplication.h"
#include "qgsgoochmaterialwidget.h"
#include "qgsline3dsymbolwidget.h"
#include "qgsmaterialregistry.h"
#include "qgsmetalroughmaterialwidget.h"
#include "qgsnullmaterialwidget.h"
#include "qgsphongmaterialwidget.h"
#include "qgsphongtexturedmaterialwidget.h"
#include "qgspoint3dsymbolwidget.h"
#include "qgspolygon3dsymbolwidget.h"
#include "qgssimplelinematerialwidget.h"

void Qgs3DAppUtils::initialize()
{
  qgis::down_cast<Qgs3DSymbolMetadata *>( QgsApplication::symbol3DRegistry()->symbolMetadata( u"point"_s ) )->setWidgetFunction( QgsPoint3DSymbolWidget::create );
  qgis::down_cast<Qgs3DSymbolMetadata *>( QgsApplication::symbol3DRegistry()->symbolMetadata( u"line"_s ) )->setWidgetFunction( QgsLine3DSymbolWidget::create );
  qgis::down_cast<Qgs3DSymbolMetadata *>( QgsApplication::symbol3DRegistry()->symbolMetadata( u"polygon"_s ) )->setWidgetFunction( QgsPolygon3DSymbolWidget::create );

  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"null"_s ) )->setWidgetFunction( QgsNullMaterialWidget::create );
  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"phong"_s ) )->setWidgetFunction( QgsPhongMaterialWidget::create );
  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"phongtextured"_s ) )->setWidgetFunction( QgsPhongTexturedMaterialWidget::create );
  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"gooch"_s ) )->setWidgetFunction( QgsGoochMaterialWidget::create );
  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"simpleline"_s ) )->setWidgetFunction( QgsSimpleLineMaterialWidget::create );
  qgis::down_cast<QgsMaterialSettingsMetadata *>( Qgs3D::materialRegistry()->materialSettingsMetadata( u"metalrough"_s ) )->setWidgetFunction( QgsMetalRoughMaterialWidget::create );

  QgsStyleModel::setIconGenerator( new Qgs3DIconGenerator( QgsApplication::defaultStyleModel() ) );
}
