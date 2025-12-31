/***************************************************************************
    qgsgrasseditrenderer.cpp
                             -------------------
    begin                : February, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrasseditrenderer.h"

#include "qgscategorizedsymbolrenderer.h"
#include "qgscategorizedsymbolrendererwidget.h"
#include "qgsfeature.h"
#include "qgsgrassvectormap.h"
#include "qgslinesymbollayer.h"
#include "qgslogger.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsrendererregistry.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"

#include <QVBoxLayout>

#include "moc_qgsgrasseditrenderer.cpp"

QgsGrassEditRenderer::QgsGrassEditRenderer()
  : QgsFeatureRenderer( u"grassEdit"_s )
{
  QHash<int, QColor> colors;
  //colors.insert( QgsGrassVectorMap::TopoUndefined, QColor( 125, 125, 125 ) );
  colors.insert( QgsGrassVectorMap::TopoLine, QColor( Qt::black ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryError, QColor( Qt::red ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryErrorLeft, QColor( 255, 125, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryErrorRight, QColor( 255, 125, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoBoundaryOk, QColor( Qt::green ) );

  QHash<int, QString> labels;
  //labels.insert( QgsGrassVectorMap::TopoUndefined, "Unknown type" );
  labels.insert( QgsGrassVectorMap::TopoLine, u"Line"_s );
  labels.insert( QgsGrassVectorMap::TopoBoundaryError, u"Boundary (topological error on both sides)"_s );
  labels.insert( QgsGrassVectorMap::TopoBoundaryErrorLeft, u"Boundary (topological error on the left side)"_s );
  labels.insert( QgsGrassVectorMap::TopoBoundaryErrorRight, u"Boundary (topological error on the right side)"_s );
  labels.insert( QgsGrassVectorMap::TopoBoundaryOk, u"Boundary (correct)"_s );

  QgsCategoryList categoryList;

  // first/last vertex marker to distinguish vertices from nodes
  QgsMarkerLineSymbolLayer *firstVertexMarkerLine = new QgsMarkerLineSymbolLayer( false );
  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer( Qgis::MarkerShape::Cross2, 2 );
  markerSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  markerSymbolLayer->setStrokeColor( QColor( 255, 0, 0 ) );
  markerSymbolLayer->setStrokeWidth( 0.5 );
  QgsSymbolLayerList markerLayers;
  markerLayers << markerSymbolLayer;
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( markerLayers );
  firstVertexMarkerLine->setSubSymbol( markerSymbol );
  firstVertexMarkerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
  QgsMarkerLineSymbolLayer *lastVertexMarkerLine = static_cast<QgsMarkerLineSymbolLayer *>( firstVertexMarkerLine->clone() );
  lastVertexMarkerLine->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
  for ( int value : colors.keys() )
  {
    QgsSymbol *symbol = QgsSymbol::defaultSymbol( Qgis::GeometryType::Line );
    symbol->setColor( colors.value( value ) );
    symbol->appendSymbolLayer( firstVertexMarkerLine->clone() );
    symbol->appendSymbolLayer( lastVertexMarkerLine->clone() );
    categoryList << QgsRendererCategory( QVariant( value ), symbol, labels.value( value ) );
  }
  delete firstVertexMarkerLine;
  delete lastVertexMarkerLine;
  mLineRenderer = new QgsCategorizedSymbolRenderer( u"topo_symbol"_s, categoryList );

  colors.clear();
  labels.clear();

  colors.insert( QgsGrassVectorMap::TopoPoint, QColor( 0, 255, 255 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidIn, QColor( 0, 255, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidOut, QColor( 255, 0, 0 ) );
  colors.insert( QgsGrassVectorMap::TopoCentroidDupl, QColor( 255, 0, 255 ) );

  labels.insert( QgsGrassVectorMap::TopoPoint, u"Point"_s );
  labels.insert( QgsGrassVectorMap::TopoCentroidIn, u"Centroid in area"_s );
  labels.insert( QgsGrassVectorMap::TopoCentroidOut, u"Centroid outside area"_s );
  labels.insert( QgsGrassVectorMap::TopoCentroidDupl, u"Duplicate centroid"_s );

  categoryList.clear();

  for ( int value : colors.keys() )
  {
    QgsSymbol *symbol = QgsSymbol::defaultSymbol( Qgis::GeometryType::Point );
    symbol->setColor( colors.value( value ) );
    categoryList << QgsRendererCategory( QVariant( value ), symbol, labels.value( value ) );
  }

  mMarkerRenderer = new QgsCategorizedSymbolRenderer( u"topo_symbol"_s, categoryList );
}

QgsGrassEditRenderer::~QgsGrassEditRenderer()
{
  delete mMarkerRenderer;
  delete mLineRenderer;
}

void QgsGrassEditRenderer::setLineRenderer( QgsFeatureRenderer *renderer )
{
  delete mLineRenderer;
  mLineRenderer = renderer;
}
void QgsGrassEditRenderer::setMarkerRenderer( QgsFeatureRenderer *renderer )
{
  delete mMarkerRenderer;
  mMarkerRenderer = renderer;
}

QgsSymbol *QgsGrassEditRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const
{
  int symbolCode = feature.attribute( u"topo_symbol"_s ).toInt();
  QgsDebugMsgLevel( QString( "fid = %1 symbolCode = %2" ).arg( feature.id() ).arg( symbolCode ), 3 );

  QgsSymbol *symbol = nullptr;
  if ( symbolCode == QgsGrassVectorMap::TopoPoint || symbolCode == QgsGrassVectorMap::TopoCentroidIn || symbolCode == QgsGrassVectorMap::TopoCentroidOut || symbolCode == QgsGrassVectorMap::TopoCentroidDupl || symbolCode == QgsGrassVectorMap::TopoNode0 || symbolCode == QgsGrassVectorMap::TopoNode1 || symbolCode == QgsGrassVectorMap::TopoNode2 )
  {
    symbol = mMarkerRenderer->symbolForFeature( feature, context );
  }
  else if ( symbolCode == QgsGrassVectorMap::TopoLine || symbolCode == QgsGrassVectorMap::TopoBoundaryError || symbolCode == QgsGrassVectorMap::TopoBoundaryErrorLeft || symbolCode == QgsGrassVectorMap::TopoBoundaryErrorRight || symbolCode == QgsGrassVectorMap::TopoBoundaryOk )
  {
    symbol = mLineRenderer->symbolForFeature( feature, context );
  }
  else
  {
    // should not happen
    QgsDebugError( "unknown symbol code" );
  }

  if ( symbol )
  {
    QgsDebugMsgLevel( "color = " + symbol->color().name(), 3 );
  }
  else
  {
    QgsDebugMsgLevel( "no symbol", 3 );
  }

  return symbol;
}

Qgis::FeatureRendererFlags QgsGrassEditRenderer::flags() const
{
  Qgis::FeatureRendererFlags res;
  if ( mLineRenderer->flags().testFlag( Qgis::FeatureRendererFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  if ( mMarkerRenderer->flags().testFlag( Qgis::FeatureRendererFlag::AffectsLabeling ) )
    res.setFlag( Qgis::FeatureRendererFlag::AffectsLabeling );
  return res;
}

void QgsGrassEditRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  // TODO better
  //QgsFields topoFields;
  //topoFields.append( QgsField( "topo_symbol", QVariant::Int, "int" ) );
  mLineRenderer->startRender( context, fields );
  mMarkerRenderer->startRender( context, fields );
}

void QgsGrassEditRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  mLineRenderer->stopRender( context );
  mMarkerRenderer->stopRender( context );
}

QSet<QString> QgsGrassEditRenderer::usedAttributes( const QgsRenderContext &context ) const
{
  return mLineRenderer->usedAttributes( context );
}

QgsFeatureRenderer *QgsGrassEditRenderer::clone() const
{
  QgsGrassEditRenderer *r = new QgsGrassEditRenderer();
  if ( mLineRenderer )
  {
    r->mLineRenderer = mLineRenderer->clone();
  }
  if ( mMarkerRenderer )
  {
    r->mMarkerRenderer = mMarkerRenderer->clone();
  }
  return r;
}

QgsSymbolList QgsGrassEditRenderer::symbols( QgsRenderContext &context ) const
{
  return mLineRenderer->symbols( context );
}

QString QgsGrassEditRenderer::dump() const
{
  return u"GRASS edit renderer"_s;
}

QDomElement QgsGrassEditRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( u"type"_s, u"grassEdit"_s );

  QDomElement lineElem = doc.createElement( u"line"_s );
  rendererElem.appendChild( lineElem );
  lineElem.appendChild( mLineRenderer->save( doc, context ) );

  QDomElement pointElem = doc.createElement( u"marker"_s );
  rendererElem.appendChild( pointElem );
  pointElem.appendChild( mMarkerRenderer->save( doc, context ) );

  return rendererElem;
}


QgsFeatureRenderer *QgsGrassEditRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  QgsGrassEditRenderer *renderer = new QgsGrassEditRenderer();

  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QDomElement elem = childElem.firstChildElement();
    if ( !elem.isNull() )
    {
      QString rendererType = elem.attribute( u"type"_s );
      QgsDebugMsgLevel( "childElem.tagName() = " + childElem.tagName() + " rendererType = " + rendererType, 2 );
      QgsRendererAbstractMetadata *meta = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
      if ( meta )
      {
        QgsFeatureRenderer *subRenderer = meta->createRenderer( elem, context );
        if ( subRenderer )
        {
          QgsDebugMsgLevel( "renderer created : " + renderer->type(), 2 );
          if ( childElem.tagName() == "line"_L1 )
          {
            renderer->setLineRenderer( subRenderer );
          }
          else if ( childElem.tagName() == "marker"_L1 )
          {
            renderer->setMarkerRenderer( subRenderer );
          }
        }
      }
    }
    childElem = childElem.nextSiblingElement();
  }
  return renderer;
}

//--------------------------------------- QgsGrassEditRendererWidget --------------------------------------------

QgsRendererWidget *QgsGrassEditRendererWidget::create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
{
  return new QgsGrassEditRendererWidget( layer, style, renderer );
}

QgsGrassEditRendererWidget::QgsGrassEditRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer )
  : QgsRendererWidget( layer, style )
{
  mRenderer = dynamic_cast<QgsGrassEditRenderer *>( renderer->clone() );
  if ( !mRenderer )
  {
    mRenderer = new QgsGrassEditRenderer();
  }

  QVBoxLayout *layout = new QVBoxLayout( this );

  mLineRendererWidget = QgsCategorizedSymbolRendererWidget::create( layer, style, mRenderer->lineRenderer()->clone() );
  layout->addWidget( mLineRendererWidget );

  mPointRendererWidget = QgsCategorizedSymbolRendererWidget::create( layer, style, mRenderer->pointRenderer()->clone() );
  layout->addWidget( mPointRendererWidget );
}

QgsGrassEditRendererWidget::~QgsGrassEditRendererWidget()
{
  delete mRenderer;
}

QgsFeatureRenderer *QgsGrassEditRendererWidget::renderer()
{
  mRenderer->setLineRenderer( mLineRendererWidget->renderer()->clone() );
  mRenderer->setMarkerRenderer( mPointRendererWidget->renderer()->clone() );
  return mRenderer;
}
