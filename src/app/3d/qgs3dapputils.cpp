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
#include "qgspointcloudrenderer.h"
#include "qgspointcloud3dsymbol.h"
#include "qgspointcloudlayer3drenderer.h"
#include "qgspointcloudrgbrenderer.h"
#include "qgspointcloudattributebyramprenderer.h"
#include "qgspointcloudclassifiedrenderer.h"

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

std::unique_ptr<QgsPointCloudLayer3DRenderer> Qgs3DAppUtils::convert2dPointCloudRendererTo3d( QgsPointCloudRenderer *renderer )
{
  if ( !renderer )
    return nullptr;

  std::unique_ptr< QgsPointCloud3DSymbol > symbol3D;
  if ( renderer->type() == QLatin1String( "ramp" ) )
  {
    const QgsPointCloudAttributeByRampRenderer *renderer2d = dynamic_cast< const QgsPointCloudAttributeByRampRenderer * >( renderer );
    symbol3D = std::make_unique< QgsColorRampPointCloud3DSymbol >();
    QgsColorRampPointCloud3DSymbol *symbol = static_cast< QgsColorRampPointCloud3DSymbol * >( symbol3D.get() );
    symbol->setAttribute( renderer2d->attribute() );
    symbol->setColorRampShaderMinMax( renderer2d->minimum(), renderer2d->maximum() );
    symbol->setColorRampShader( renderer2d->colorRampShader() );
  }
  else if ( renderer->type() == QLatin1String( "rgb" ) )
  {
    const QgsPointCloudRgbRenderer *renderer2d = dynamic_cast< const QgsPointCloudRgbRenderer * >( renderer );
    symbol3D = std::make_unique< QgsRgbPointCloud3DSymbol >();
    QgsRgbPointCloud3DSymbol *symbol = static_cast< QgsRgbPointCloud3DSymbol * >( symbol3D.get() );
    symbol->setRedAttribute( renderer2d->redAttribute() );
    symbol->setGreenAttribute( renderer2d->greenAttribute() );
    symbol->setBlueAttribute( renderer2d->blueAttribute() );

    symbol->setRedContrastEnhancement( renderer2d->redContrastEnhancement() ? new QgsContrastEnhancement( *renderer2d->redContrastEnhancement() ) : nullptr );
    symbol->setGreenContrastEnhancement( renderer2d->greenContrastEnhancement() ? new QgsContrastEnhancement( *renderer2d->greenContrastEnhancement() ) : nullptr );
    symbol->setBlueContrastEnhancement( renderer2d->blueContrastEnhancement() ? new QgsContrastEnhancement( *renderer2d->blueContrastEnhancement() ) : nullptr );
  }
  else if ( renderer->type() == QLatin1String( "classified" ) )
  {

    const QgsPointCloudClassifiedRenderer *renderer2d = dynamic_cast< const QgsPointCloudClassifiedRenderer * >( renderer );
    symbol3D = std::make_unique< QgsClassificationPointCloud3DSymbol >();
    QgsClassificationPointCloud3DSymbol *symbol = static_cast< QgsClassificationPointCloud3DSymbol * >( symbol3D.get() );
    symbol->setAttribute( renderer2d->attribute() );
    symbol->setCategoriesList( renderer2d->categories() );
  }

  if ( symbol3D )
  {
    std::unique_ptr< QgsPointCloudLayer3DRenderer > renderer3D = std::make_unique< QgsPointCloudLayer3DRenderer >();
    renderer3D->setSymbol( symbol3D.release() );
    return renderer3D;
  }
  return nullptr;
}
