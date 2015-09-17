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

#include  <QVBoxLayout>

#include "qgscategorizedsymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2widget.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsrendererv2registry.h"
#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include "qgsgrasseditrenderer.h"
#include "qgsgrassprovider.h"

QgsGrassEditRenderer::QgsGrassEditRenderer()
    : QgsFeatureRendererV2( "grassEdit" )
    , mLineRenderer( 0 )
    , mMarkerRenderer( 0 )
{
  QHash<int, QColor> colors;
  colors.insert( QgsGrassProvider::TopoUndefined, QColor( 125, 125, 125 ) );
  colors.insert( QgsGrassProvider::TopoLine, QColor( Qt::black ) );
  colors.insert( QgsGrassProvider::TopoBoundary0, QColor( Qt::red ) );
  colors.insert( QgsGrassProvider::TopoBoundary1, QColor( 255, 125, 0 ) );
  colors.insert( QgsGrassProvider::TopoBoundary2, QColor( Qt::green ) );

  QHash<int, QString> labels;
  labels.insert( QgsGrassProvider::TopoUndefined, "Unknown type" );
  labels.insert( QgsGrassProvider::TopoLine, "Line" );
  labels.insert( QgsGrassProvider::TopoBoundary0, "Boundary (isolated)" );
  labels.insert( QgsGrassProvider::TopoBoundary1, "Boundary (area on one side)" );
  labels.insert( QgsGrassProvider::TopoBoundary2, "Boundary (areas on both sides)" );

  QgsCategoryList categoryList;

  foreach ( int value, colors.keys() )
  {
    QgsSymbolV2 * symbol = QgsSymbolV2::defaultSymbol( QGis::Line );
    symbol->setColor( colors.value( value ) );
    categoryList << QgsRendererCategoryV2( QVariant( value ), symbol, labels.value( value ) );
  }

  mLineRenderer = new QgsCategorizedSymbolRendererV2( "topo_symbol", categoryList );

  colors.clear();
  labels.clear();

  colors.insert( QgsGrassProvider::TopoPoint, QColor( 0, 0, 0 ) );
  colors.insert( QgsGrassProvider::TopoCentroidIn, QColor( 0, 255, 0 ) );
  colors.insert( QgsGrassProvider::TopoCentroidOut, QColor( 255, 0, 0 ) );
  colors.insert( QgsGrassProvider::TopoCentroidDupl, QColor( 255, 0, 255 ) );

  labels.insert( QgsGrassProvider::TopoPoint, "Point" );
  labels.insert( QgsGrassProvider::TopoCentroidIn, "Centroid in area" );
  labels.insert( QgsGrassProvider::TopoCentroidOut, "Centroid outside area" );
  labels.insert( QgsGrassProvider::TopoCentroidDupl, "Duplicate centroid" );

  categoryList.clear();

  foreach ( int value, colors.keys() )
  {
    QgsSymbolV2 * symbol = QgsSymbolV2::defaultSymbol( QGis::Point );
    symbol->setColor( colors.value( value ) );
    categoryList << QgsRendererCategoryV2( QVariant( value ), symbol, labels.value( value ) );
  }

  mMarkerRenderer = new QgsCategorizedSymbolRendererV2( "topo_symbol", categoryList );
}

QgsGrassEditRenderer::~QgsGrassEditRenderer()
{
}

void QgsGrassEditRenderer::setLineRenderer( QgsFeatureRendererV2 *renderer )
{
  delete mLineRenderer;
  mLineRenderer = renderer;
}
void QgsGrassEditRenderer::setMarkerRenderer( QgsFeatureRendererV2 *renderer )
{
  delete mMarkerRenderer;
  mMarkerRenderer = renderer;
}

QgsSymbolV2* QgsGrassEditRenderer::symbolForFeature( QgsFeature& feature, QgsRenderContext& context )
{
  int symbolCode = feature.attribute( "topo_symbol" ).toInt();
  QgsDebugMsgLevel( QString( "fid = %1 symbolCode = %2" ).arg( feature.id() ).arg( symbolCode ), 3 );

  QgsSymbolV2* symbol = 0;
  if ( symbolCode == QgsGrassProvider::TopoPoint || symbolCode == QgsGrassProvider::TopoCentroidIn ||
       symbolCode == QgsGrassProvider::TopoCentroidOut || symbolCode == QgsGrassProvider::TopoCentroidDupl ||
       symbolCode == QgsGrassProvider::TopoNode0 || symbolCode == QgsGrassProvider::TopoNode1 ||
       symbolCode == QgsGrassProvider::TopoNode2 )
  {
    symbol = mMarkerRenderer->symbolForFeature( feature, context );
  }
  else if ( symbolCode == QgsGrassProvider::TopoLine || symbolCode == QgsGrassProvider::TopoBoundary0 ||
            symbolCode == QgsGrassProvider::TopoBoundary1 || symbolCode == QgsGrassProvider::TopoBoundary2 )
  {
    symbol = mLineRenderer->symbolForFeature( feature, context );
  }
  else
  {
    // should not happen
    QgsDebugMsg( "unknown symbol code" );
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

void QgsGrassEditRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  Q_UNUSED( fields );
  // TODO better
  //QgsFields topoFields;
  //topoFields.append( QgsField( "topo_symbol", QVariant::Int, "int" ) );
  mLineRenderer->startRender( context, fields );
  mMarkerRenderer->startRender( context, fields );
}

void QgsGrassEditRenderer::stopRender( QgsRenderContext& context )
{
  mLineRenderer->stopRender( context );
  mMarkerRenderer->stopRender( context );
}

QList<QString> QgsGrassEditRenderer::usedAttributes()
{
  return mLineRenderer->usedAttributes();
}

QgsFeatureRendererV2* QgsGrassEditRenderer::clone() const
{
  QgsGrassEditRenderer* r = new QgsGrassEditRenderer();
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

QgsSymbolV2List QgsGrassEditRenderer::symbols( QgsRenderContext& context )
{
  return mLineRenderer->symbols( context );
}

QString QgsGrassEditRenderer::dump() const
{
  return "GRASS edit renderer";
}

QDomElement QgsGrassEditRenderer::save( QDomDocument& doc )
{
  QgsDebugMsg( "entered" );
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "grassEdit" );

  QDomElement lineElem = doc.createElement( "line" );
  rendererElem.appendChild( lineElem );
  lineElem.appendChild( mLineRenderer->save( doc ) );

  QDomElement pointElem = doc.createElement( "marker" );
  rendererElem.appendChild( pointElem );
  pointElem.appendChild( mMarkerRenderer->save( doc ) );

  return rendererElem;
}


QgsFeatureRendererV2* QgsGrassEditRenderer::create( QDomElement& element )
{
  QgsDebugMsg( "entered" );
  QgsGrassEditRenderer *renderer = new QgsGrassEditRenderer();

  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QDomElement elem = childElem.firstChildElement();
    if ( !elem.isNull() )
    {
      QString rendererType = elem.attribute( "type" );
      QgsDebugMsg( "childElem.tagName() = " + childElem.tagName() + " rendererType = " + rendererType );
      QgsRendererV2AbstractMetadata* meta = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
      if ( meta )
      {
        QgsFeatureRendererV2* subRenderer = meta->createRenderer( elem );
        if ( subRenderer )
        {
          QgsDebugMsg( "renderer created : " + renderer->type() );
          if ( childElem.tagName() == "line" )
          {
            renderer->setLineRenderer( subRenderer );
          }
          else if ( childElem.tagName() == "marker" )
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

QgsRendererV2Widget* QgsGrassEditRendererWidget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  QgsDebugMsg( "entered" );
  return new QgsGrassEditRendererWidget( layer, style, renderer );
}

QgsGrassEditRendererWidget::QgsGrassEditRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
    , mRenderer( 0 )
{
  QgsDebugMsg( "entered" );
  mRenderer = dynamic_cast<QgsGrassEditRenderer*>( renderer->clone() );
  if ( !mRenderer )
  {
    return;
  }

  QVBoxLayout* layout = new QVBoxLayout( this );

  mLineRendererWidget = QgsCategorizedSymbolRendererV2Widget::create( layer, style, mRenderer->lineRenderer()->clone() );
  layout->addWidget( mLineRendererWidget );

  mPointRendererWidget = QgsCategorizedSymbolRendererV2Widget::create( layer, style, mRenderer->pointRenderer()->clone() );
  layout->addWidget( mPointRendererWidget );
}

QgsGrassEditRendererWidget::~QgsGrassEditRendererWidget()
{
  QgsDebugMsg( "entered" );
  delete mRenderer;
}

QgsFeatureRendererV2* QgsGrassEditRendererWidget::renderer()
{
  QgsDebugMsg( "entered" );
  mRenderer->setLineRenderer( mLineRendererWidget->renderer()->clone() );
  mRenderer->setMarkerRenderer( mPointRendererWidget->renderer()->clone() );
  return mRenderer;
}



