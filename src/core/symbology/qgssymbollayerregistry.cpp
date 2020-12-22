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

QgsSymbolLayerRegistry::QgsSymbolLayerRegistry()
{
  // init registry with known symbol layers
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleLine" ), QObject::tr( "Simple Line" ), QgsSymbol::Line,
                      QgsSimpleLineSymbolLayer::create, QgsSimpleLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "MarkerLine" ), QObject::tr( "Marker Line" ), QgsSymbol::Line,
                      QgsMarkerLineSymbolLayer::create, QgsMarkerLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "HashLine" ), QObject::tr( "Hashed Line" ), QgsSymbol::Line,
                      QgsHashedLineSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "ArrowLine" ), QObject::tr( "Arrow" ), QgsSymbol::Line, QgsArrowSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleMarker" ), QObject::tr( "Simple Marker" ), QgsSymbol::Marker,
                      QgsSimpleMarkerSymbolLayer::create, QgsSimpleMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "FilledMarker" ), QObject::tr( "Filled Marker" ), QgsSymbol::Marker,
                      QgsFilledMarkerSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SvgMarker" ), QObject::tr( "SVG Marker" ), QgsSymbol::Marker,
                      QgsSvgMarkerSymbolLayer::create, QgsSvgMarkerSymbolLayer::createFromSld, QgsSvgMarkerSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RasterMarker" ), QObject::tr( "Raster Image Marker" ), QgsSymbol::Marker,
                      QgsRasterMarkerSymbolLayer::create, nullptr, QgsRasterFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "FontMarker" ), QObject::tr( "Font Marker" ), QgsSymbol::Marker,
                      QgsFontMarkerSymbolLayer::create, QgsFontMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "EllipseMarker" ), QObject::tr( "Ellipse Marker" ), QgsSymbol::Marker,
                      QgsEllipseSymbolLayer::create, QgsEllipseSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "VectorField" ), QObject::tr( "Vector Field Marker" ), QgsSymbol::Marker,
                      QgsVectorFieldSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "MaskMarker" ), QObject::tr( "Mask" ), QgsSymbol::Marker,
                      QgsMaskMarkerSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SimpleFill" ), QObject::tr( "Simple Fill" ), QgsSymbol::Fill,
                      QgsSimpleFillSymbolLayer::create, QgsSimpleFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "GradientFill" ), QObject::tr( "Gradient Fill" ), QgsSymbol::Fill,
                      QgsGradientFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "ShapeburstFill" ), QObject::tr( "Shapeburst Fill" ), QgsSymbol::Fill,
                      QgsShapeburstFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RasterFill" ), QObject::tr( "Raster Image Fill" ), QgsSymbol::Fill,
                      QgsRasterFillSymbolLayer::create, nullptr, QgsRasterFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "SVGFill" ), QObject::tr( "SVG Fill" ), QgsSymbol::Fill,
                      QgsSVGFillSymbolLayer::create, QgsSVGFillSymbolLayer::createFromSld, QgsSVGFillSymbolLayer::resolvePaths ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "CentroidFill" ), QObject::tr( "Centroid Fill" ), QgsSymbol::Fill,
                      QgsCentroidFillSymbolLayer::create, QgsCentroidFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "RandomMarkerFill" ), QObject::tr( "Random Marker Fill" ), QgsSymbol::Fill,
                      QgsRandomMarkerFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "LinePatternFill" ), QObject::tr( "Line Pattern Fill" ), QgsSymbol::Fill,
                      QgsLinePatternFillSymbolLayer::create, QgsLinePatternFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "PointPatternFill" ), QObject::tr( "Point Pattern Fill" ), QgsSymbol::Fill,
                      QgsPointPatternFillSymbolLayer::create, QgsPointPatternFillSymbolLayer::createFromSld ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( QStringLiteral( "GeometryGenerator" ), QObject::tr( "Geometry Generator" ), QgsSymbol::Hybrid,
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


QgsSymbolLayerAbstractMetadata *QgsSymbolLayerRegistry::symbolLayerMetadata( const QString &name ) const
{
  return mMetadata.value( name );
}

QgsSymbolLayer *QgsSymbolLayerRegistry::defaultSymbolLayer( QgsSymbol::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbol::Marker:
      return QgsSimpleMarkerSymbolLayer::create();

    case QgsSymbol::Line:
      return QgsSimpleLineSymbolLayer::create();

    case QgsSymbol::Fill:
      return QgsSimpleFillSymbolLayer::create();

    case QgsSymbol::Hybrid:
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

QStringList QgsSymbolLayerRegistry::symbolLayersForType( QgsSymbol::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerAbstractMetadata *>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type || it.value()->type() == QgsSymbol::Hybrid )
      lst.append( it.key() );
  }
  return lst;
}
