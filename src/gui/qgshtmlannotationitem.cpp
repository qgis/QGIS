/***************************************************************************
                              qgshtmlannotationitem.h
                              ------------------------
  begin                : February 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshtmlannotationitem.h"
#include "qgsattributeeditor.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"
#include "qgsnetworkaccessmanager.h"

#include <QDomElement>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QSettings>
#include <QWidget>


QgsHtmlAnnotationItem::QgsHtmlAnnotationItem( QgsMapCanvas* canvas, QgsVectorLayer* vlayer, bool hasFeature, int feature )
    : QgsAnnotationItem( canvas )
    , mWidgetContainer( 0 )
    , mWebView( 0 )
    , mVectorLayer( vlayer )
    , mHasAssociatedFeature( hasFeature )
    , mFeatureId( feature )
{
  mWebView = new QWebView();
  mWebView->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );

  mWidgetContainer = new QGraphicsProxyWidget( this );
  mWidgetContainer->setWidget( mWebView );

  QObject::connect( mWebView->page()->mainFrame(), SIGNAL( javaScriptWindowObjectCleared() ), this, SLOT( javascript() ) );

  if ( mVectorLayer && mMapCanvas )
  {
    QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
  }

  setFeatureForMapPosition();
}

QgsHtmlAnnotationItem::~QgsHtmlAnnotationItem()
{
  delete mWebView;
}

void QgsHtmlAnnotationItem::setHTMLPage( const QString& htmlFile )
{
  QFile file( htmlFile );
  mHtmlFile = htmlFile;
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    mHtmlSource = "";
  }
  else
  {
    QTextStream in( &file );
    in.setCodec( "UTF-8" );
    mHtmlSource = in.readAll();
  }

  file.close();
  setFeatureForMapPosition();
}

void QgsHtmlAnnotationItem::setMapPosition( const QgsPoint& pos )
{
  QgsAnnotationItem::setMapPosition( pos );
  setFeatureForMapPosition();
}

void QgsHtmlAnnotationItem::paint( QPainter * painter )
{
  Q_UNUSED( painter );
}

void QgsHtmlAnnotationItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  if ( !painter || !mWidgetContainer )
  {
    return;
  }

  drawFrame( painter );
  if ( mMapPositionFixed )
  {
    drawMarkerSymbol( painter );
  }

  mWidgetContainer->setGeometry( QRectF( mOffsetFromReferencePoint.x() + mFrameBorderWidth / 2.0, mOffsetFromReferencePoint.y()
                                         + mFrameBorderWidth / 2.0, mFrameSize.width() - mFrameBorderWidth, mFrameSize.height()
                                         - mFrameBorderWidth ) );
  if ( data( 1 ).toString() == "composer" )
  {
    mWidgetContainer->widget()->render( painter, mOffsetFromReferencePoint.toPoint() );
  }

  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

QSizeF QgsHtmlAnnotationItem::minimumFrameSize() const
{
  if ( mWebView )
  {
    QSizeF widgetMinSize = mWebView->minimumSize();
    return QSizeF( 2 * mFrameBorderWidth + widgetMinSize.width(), 2 * mFrameBorderWidth + widgetMinSize.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

void QgsHtmlAnnotationItem::writeXML( QDomDocument& doc ) const
{
  QDomElement documentElem = doc.documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }

  QDomElement formAnnotationElem = doc.createElement( "HtmlAnnotationItem" );
  if ( mVectorLayer )
  {
    formAnnotationElem.setAttribute( "vectorLayer", mVectorLayer->id() );
  }
  formAnnotationElem.setAttribute( "hasFeature", mHasAssociatedFeature );
  formAnnotationElem.setAttribute( "feature", mFeatureId );
  formAnnotationElem.setAttribute( "htmlfile", htmlPage() );

  _writeXML( doc, formAnnotationElem );
  documentElem.appendChild( formAnnotationElem );
}

void QgsHtmlAnnotationItem::readXML( const QDomDocument& doc, const QDomElement& itemElem )
{
  mVectorLayer = 0;
  if ( itemElem.hasAttribute( "vectorLayer" ) )
  {
    mVectorLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( itemElem.attribute( "vectorLayer", "" ) ) );
    if ( mVectorLayer )
    {
      QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
    }
  }
  mHasAssociatedFeature = itemElem.attribute( "hasFeature", "0" ).toInt();
  mFeatureId = itemElem.attribute( "feature", "0" ).toInt();
  mHtmlFile = itemElem.attribute( "htmlfile", "" );
  QDomElement annotationElem = itemElem.firstChildElement( "AnnotationItem" );
  if ( !annotationElem.isNull() )
  {
    _readXML( doc, annotationElem );
  }

  if ( mWebView )
  {
    setHTMLPage( mHtmlFile );
  }
  updateVisibility();
}

void QgsHtmlAnnotationItem::setFeatureForMapPosition()
{
  if ( !mVectorLayer || !mMapCanvas )
  {
    return;
  }

  QSettings settings;
  double halfIdentifyWidth = QgsMapTool::searchRadiusMU( mMapCanvas );
  QgsRectangle searchRect( mMapPosition.x() - halfIdentifyWidth, mMapPosition.y() - halfIdentifyWidth,
                           mMapPosition.x() + halfIdentifyWidth, mMapPosition.y() + halfIdentifyWidth );

  QgsFeatureIterator fit = mVectorLayer->getFeatures( QgsFeatureRequest().setFilterRect( searchRect ).setFlags( QgsFeatureRequest::NoGeometry | QgsFeatureRequest::ExactIntersect ) );

  QgsFeature currentFeature;
  QgsFeatureId currentFeatureId = 0;
  bool featureFound = false;

  while ( fit.nextFeature( currentFeature ) )
  {
    currentFeatureId = currentFeature.id();
    featureFound = true;
    break;
  }

  mHasAssociatedFeature = featureFound;
  mFeatureId = currentFeatureId;
  mFeature = currentFeature;

  QString newtext = QgsExpression::replaceExpressionText( mHtmlSource, &mFeature, vectorLayer() );
  mWebView->setHtml( newtext );
}

void QgsHtmlAnnotationItem::updateVisibility()
{
  bool visible = true;
  if ( mVectorLayer && mMapCanvas )
  {
    visible = mMapCanvas->layers().contains( mVectorLayer );
  }
  setVisible( visible );
}

void QgsHtmlAnnotationItem::javascript()
{
  QWebFrame *frame = mWebView->page()->mainFrame();
  frame->addToJavaScriptWindowObject( "canvas", mMapCanvas );
  frame->addToJavaScriptWindowObject( "layer", mVectorLayer );
}



