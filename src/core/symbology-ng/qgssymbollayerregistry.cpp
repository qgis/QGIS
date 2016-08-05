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
#include "qgsellipsesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgsvectorfieldsymbollayer.h"
#include "qgsgeometrygeneratorsymbollayerv2.h"

QgsSymbolLayerRegistry::QgsSymbolLayerRegistry()
{
  // init registry with known symbol layers
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleLine", QObject::tr( "Simple line" ), QgsSymbolV2::Line,
                      QgsSimpleLineSymbolLayerV2::create, QgsSimpleLineSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "MarkerLine", QObject::tr( "Marker line" ), QgsSymbolV2::Line,
                      QgsMarkerLineSymbolLayerV2::create, QgsMarkerLineSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "ArrowLine", QObject::tr( "Arrow" ), QgsSymbolV2::Line, QgsArrowSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleMarker", QObject::tr( "Simple marker" ), QgsSymbolV2::Marker,
                      QgsSimpleMarkerSymbolLayerV2::create, QgsSimpleMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "FilledMarker", QObject::tr( "Filled marker" ), QgsSymbolV2::Marker,
                      QgsFilledMarkerSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SvgMarker", QObject::tr( "SVG marker" ), QgsSymbolV2::Marker,
                      QgsSvgMarkerSymbolLayerV2::create, QgsSvgMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "FontMarker", QObject::tr( "Font marker" ), QgsSymbolV2::Marker,
                      QgsFontMarkerSymbolLayerV2::create, QgsFontMarkerSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "EllipseMarker", QObject::tr( "Ellipse marker" ), QgsSymbolV2::Marker,
                      QgsEllipseSymbolLayerV2::create, QgsEllipseSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "VectorField", QObject::tr( "Vector field marker" ), QgsSymbolV2::Marker,
                      QgsVectorFieldSymbolLayer::create ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "SimpleFill", QObject::tr( "Simple fill" ), QgsSymbolV2::Fill,
                      QgsSimpleFillSymbolLayerV2::create, QgsSimpleFillSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "GradientFill", QObject::tr( "Gradient fill" ), QgsSymbolV2::Fill,
                      QgsGradientFillSymbolLayerV2::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "ShapeburstFill", QObject::tr( "Shapeburst fill" ), QgsSymbolV2::Fill,
                      QgsShapeburstFillSymbolLayerV2::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "RasterFill", QObject::tr( "Raster image fill" ), QgsSymbolV2::Fill,
                      QgsRasterFillSymbolLayer::create ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "SVGFill", QObject::tr( "SVG fill" ), QgsSymbolV2::Fill,
                      QgsSVGFillSymbolLayer::create, QgsSVGFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "CentroidFill", QObject::tr( "Centroid fill" ), QgsSymbolV2::Fill,
                      QgsCentroidFillSymbolLayerV2::create, QgsCentroidFillSymbolLayerV2::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "LinePatternFill", QObject::tr( "Line pattern fill" ), QgsSymbolV2::Fill,
                      QgsLinePatternFillSymbolLayer::create, QgsLinePatternFillSymbolLayer::createFromSld ) );
  addSymbolLayerType( new QgsSymbolLayerMetadata( "PointPatternFill", QObject::tr( "Point pattern fill" ), QgsSymbolV2::Fill,
                      QgsPointPatternFillSymbolLayer::create, QgsPointPatternFillSymbolLayer::createFromSld ) );

  addSymbolLayerType( new QgsSymbolLayerMetadata( "GeometryGenerator", QObject::tr( "Geometry generator" ), QgsSymbolV2::Hybrid,
                      QgsGeometryGeneratorSymbolLayerV2::create ) );
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

QgsSymbolLayer* QgsSymbolLayerRegistry::defaultSymbolLayer( QgsSymbolV2::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbolV2::Marker:
      return QgsSimpleMarkerSymbolLayerV2::create();

    case QgsSymbolV2::Line:
      return QgsSimpleLineSymbolLayerV2::create();

    case QgsSymbolV2::Fill:
      return QgsSimpleFillSymbolLayerV2::create();

    case QgsSymbolV2::Hybrid:
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

QStringList QgsSymbolLayerRegistry::symbolLayersForType( QgsSymbolV2::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerAbstractMetadata*>::ConstIterator it = mMetadata.constBegin();
  for ( ; it != mMetadata.constEnd(); ++it )
  {
    if ( it.value()->type() == type || it.value()->type() == QgsSymbolV2::Hybrid )
      lst.append( it.key() );
  }
  return lst;
}
