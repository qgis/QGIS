/***************************************************************************
    qgssymbollayerregistry.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerregistry.h"

#include "qgsarrowsymbollayer.h"
#include "qgsellipsesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsvectorfieldsymbollayer.h"
#include "qgsmasksymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsinterpolatedlinerenderer.h"

QgsSymbolLayerRegistry::QgsSymbolLayerRegistry()
{
  // init registry with known symbol layers
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleLine" ), QObject::tr( "Simple Line" ), Qgis::SymbolType::Line,
                      QgsSimpleLineSymbolLayer::create, QgsSimpleLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "MarkerLine" ), QObject::tr( "Marker Line" ), Qgis::SymbolType::Line,
                      QgsMarkerLineSymbolLayer::create, QgsMarkerLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "HashLine" ), QObject::tr( "Hashed Line" ), Qgis::SymbolType::Line,
                      QgsHashedLineSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "ArrowLine" ), QObject::tr( "Arrow" ), Qgis::SymbolType::Line,
                      QgsArrowSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "InterpolatedLine" ), QObject::tr( "Interpolated Line" ), Qgis::SymbolType::Line,
                      QgsInterpolatedLineSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RasterLine" ), QObject::tr( "Raster Line" ), Qgis::SymbolType::Line,
                      QgsRasterLineSymbolLayer::create, nullptr, QgsRasterLineSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "Lineburst" ), QObject::tr( "Lineburst" ), Qgis::SymbolType::Line,
                      QgsLineburstSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleMarker" ), QObject::tr( "Simple Marker" ), Qgis::SymbolType::Marker,
                      QgsSimpleMarkerSymbolLayer::create, QgsSimpleMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "FilledMarker" ), QObject::tr( "Filled Marker" ), Qgis::SymbolType::Marker,
                      QgsFilledMarkerSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SvgMarker" ), QObject::tr( "SVG Marker" ), Qgis::SymbolType::Marker,
                      QgsSvgMarkerSymbolLayer::create, QgsSvgMarkerSymbolLayer::createFromSld, QgsSvgMarkerSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RasterMarker" ), QObject::tr( "Raster Image Marker" ), Qgis::SymbolType::Marker,
                      QgsRasterMarkerSymbolLayer::create, nullptr, QgsRasterFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "AnimatedMarker" ), QObject::tr( "Animated Marker" ), Qgis::SymbolType::Marker,
                      QgsAnimatedMarkerSymbolLayer::create, nullptr, QgsAnimatedMarkerSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "FontMarker" ), QObject::tr( "Font Marker" ), Qgis::SymbolType::Marker,
                      QgsFontMarkerSymbolLayer::create, QgsFontMarkerSymbolLayer::createFromSld, nullptr, nullptr, QgsFontMarkerSymbolLayer::resolveFonts ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "EllipseMarker" ), QObject::tr( "Ellipse Marker" ), Qgis::SymbolType::Marker,
                      QgsEllipseSymbolLayer::create, QgsEllipseSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "VectorField" ), QObject::tr( "Vector Field Marker" ), Qgis::SymbolType::Marker,
                      QgsVectorFieldSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "MaskMarker" ), QObject::tr( "Mask" ), Qgis::SymbolType::Marker,
                      QgsMaskMarkerSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleFill" ), QObject::tr( "Simple Fill" ), Qgis::SymbolType::Fill,
                      QgsSimpleFillSymbolLayer::create, QgsSimpleFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "GradientFill" ), QObject::tr( "Gradient Fill" ), Qgis::SymbolType::Fill,
                      QgsGradientFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "ShapeburstFill" ), QObject::tr( "Shapeburst Fill" ), Qgis::SymbolType::Fill,
                      QgsShapeburstFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RasterFill" ), QObject::tr( "Raster Image Fill" ), Qgis::SymbolType::Fill,
                      QgsRasterFillSymbolLayer::create, nullptr, QgsRasterFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SVGFill" ), QObject::tr( "SVG Fill" ), Qgis::SymbolType::Fill,
                      QgsSVGFillSymbolLayer::create, QgsSVGFillSymbolLayer::createFromSld, QgsSVGFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "CentroidFill" ), QObject::tr( "Centroid Fill" ), Qgis::SymbolType::Fill,
                      QgsCentroidFillSymbolLayer::create, QgsCentroidFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RandomMarkerFill" ), QObject::tr( "Random Marker Fill" ), Qgis::SymbolType::Fill,
                      QgsRandomMarkerFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "LinePatternFill" ), QObject::tr( "Line Pattern Fill" ), Qgis::SymbolType::Fill,
                      QgsLinePatternFillSymbolLayer::create, QgsLinePatternFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "PointPatternFill" ), QObject::tr( "Point Pattern Fill" ), Qgis::SymbolType::Fill,
                      QgsPointPatternFillSymbolLayer::create, QgsPointPatternFillSymbolLayer::createFromSld ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "GeometryGenerator" ), QObject::tr( "Geometry Generator" ), Qgis::SymbolType::Hybrid,
                      QgsGeometryGeneratorSymbolLayer::create ) );
}

QgsSymbolLayerRegistry::~QgsSymbolLayerRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsSymbolLayerRegistry::addSymbolLayerType( QgsSymbolLayerAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->name() ) )
    return false;

  mMetadata[metadata->name()] = metadata;
  return true;
}

bool QgsSymbolLayerRegistry::removeSymbolLayerType( QgsSymbolLayerAbstractMetadata *metadata )
{
  if ( !metadata || !mMetadata.contains( metadata->name() ) )
    return false;

  metadata = mMetadata.take( metadata->name() );
  delete metadata;
  return true;
}


QgsSymbolLayerAbstractMetadata *QgsSymbolLayerRegistry::symbolLayerMetadata( const QString &name ) const
{
  return mMetadata.value( name );
}

QgsSymbolLayer *QgsSymbolLayerRegistry::defaultSymbolLayer( Qgis::SymbolType type )
{
  switch ( type )
  {
    case Qgis::SymbolType::Marker:
      return QgsSimpleMarkerSymbolLayer::create();

    case Qgis::SymbolType::Line:
      return QgsSimpleLineSymbolLayer::create();

    case Qgis::SymbolType::Fill:
      return QgsSimpleFillSymbolLayer::create();

    case Qgis::SymbolType::Hybrid:
      return nullptr;
  }

  return nullptr;
}


QgsSymbolLayer *QgsSymbolLayerRegistry::createSymbolLayer( const QString &name, const QVariantMap &properties ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayer( properties );
}

QgsSymbolLayer *QgsSymbolLayerRegistry::createSymbolLayerFromSld( const QString &name, QDomElement &element ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayerFromSld( element );
}

void QgsSymbolLayerRegistry::resolvePaths( const QString &name, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const
{
  if ( !mMetadata.contains( name ) )
    return;

  mMetadata[name]->resolvePaths( properties, pathResolver, saving );
}

void QgsSymbolLayerRegistry::resolveFonts( const QString &name, QVariantMap &properties, const QgsReadWriteContext &context ) const
{
  if ( !mMetadata.contains( name ) )
    return;

  mMetadata[name]->resolveFonts( properties, context );
}

QStringList QgsSymbolLayerRegistry::symbolLayersForType( Qgis::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerAbstractMetadata *>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type || it.value()->type() == Qgis::SymbolType::Hybrid )
      lst.append( it.key() );
  }
  return lst;
}
