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
#include "qgsgeometrygeneratorsymbollayer.h"

QgsSymbolLayerRegistry::QgsSymbolLayerRegistry()
{
  // init registry with known symbol layers
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleLine", QObject::tr( "Simple line" ), QgsSymbol::Line,
                      QgsSimpleLineSymbolLayer::create, QgsSimpleLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "MarkerLine", QObject::tr( "Marker line" ), QgsSymbol::Line,
                      QgsMarkerLineSymbolLayer::create, QgsMarkerLineSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "ArrowLine", QObject::tr( "Arrow" ), QgsSymbol::Line, QgsArrowSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleMarker", QObject::tr( "Simple marker" ), QgsSymbol::Marker,
                      QgsSimpleMarkerSymbolLayer::create, QgsSimpleMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "FilledMarker", QObject::tr( "Filled marker" ), QgsSymbol::Marker,
                      QgsFilledMarkerSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SvgMarker", QObject::tr( "SVG marker" ), QgsSymbol::Marker,
                      QgsSvgMarkerSymbolLayer::create, QgsSvgMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "FontMarker", QObject::tr( "Font marker" ), QgsSymbol::Marker,
                      QgsFontMarkerSymbolLayer::create, QgsFontMarkerSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "EllipseMarker", QObject::tr( "Ellipse marker" ), QgsSymbol::Marker,
                      QgsEllipseSymbolLayer::create, QgsEllipseSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "VectorField", QObject::tr( "Vector field marker" ), QgsSymbol::Marker,
                      QgsVectorFieldSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleFill", QObject::tr( "Simple fill" ), QgsSymbol::Fill,
                      QgsSimpleFillSymbolLayer::create, QgsSimpleFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "GradientFill", QObject::tr( "Gradient fill" ), QgsSymbol::Fill,
                      QgsGradientFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "ShapeburstFill", QObject::tr( "Shapeburst fill" ), QgsSymbol::Fill,
                      QgsShapeburstFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "RasterFill", QObject::tr( "Raster image fill" ), QgsSymbol::Fill,
                      QgsRasterFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SVGFill", QObject::tr( "SVG fill" ), QgsSymbol::Fill,
                      QgsSVGFillSymbolLayer::create, QgsSVGFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "CentroidFill", QObject::tr( "Centroid fill" ), QgsSymbol::Fill,
                      QgsCentroidFillSymbolLayer::create, QgsCentroidFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "LinePatternFill", QObject::tr( "Line pattern fill" ), QgsSymbol::Fill,
                      QgsLinePatternFillSymbolLayer::create, QgsLinePatternFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "PointPatternFill", QObject::tr( "Point pattern fill" ), QgsSymbol::Fill,
                      QgsPointPatternFillSymbolLayer::create, QgsPointPatternFillSymbolLayer::createFromSld ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "GeometryGenerator", QObject::tr( "Geometry generator" ), QgsSymbol::Hybrid,
                      QgsGeometryGeneratorSymbolLayer::create ) );
}

QgsSymbolLayerRegistry::~QgsSymbolLayerRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsSymbolLayerRegistry::addSymbolLayerType( QgsSymbolLayerAbstractMetadata* metadata )
{
  if ( !metadata || mMetadata.contains( metadata->name() ) )
    return false;

  mMetadata[metadata->name()] = metadata;
  return true;
}


QgsSymbolLayerAbstractMetadata* QgsSymbolLayerRegistry::symbolLayerMetadata( const QString& name ) const
{
  return mMetadata.value( name );
}

QgsSymbolLayerRegistry* QgsSymbolLayerRegistry::instance()
{
  static QgsSymbolLayerRegistry mInstance;
  return &mInstance;
}

QgsSymbolLayer* QgsSymbolLayerRegistry::defaultSymbolLayer( QgsSymbol::SymbolType type )
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


QgsSymbolLayer* QgsSymbolLayerRegistry::createSymbolLayer( const QString& name, const QgsStringMap& properties ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayer( properties );
}

QgsSymbolLayer* QgsSymbolLayerRegistry::createSymbolLayerFromSld( const QString& name, QDomElement& element ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  return mMetadata[name]->createSymbolLayerFromSld( element );
}

QStringList QgsSymbolLayerRegistry::symbolLayersForType( QgsSymbol::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerAbstractMetadata*>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type || it.value()->type() == QgsSymbol::Hybrid )
      lst.append( it.key() );
  }
  return lst;
}
