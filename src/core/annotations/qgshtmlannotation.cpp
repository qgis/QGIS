/***************************************************************************
                              qgshtmlannotation.h
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

#include "qgshtmlannotation.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsexpression.h"
#include "qgsnetworkaccessmanager.h"
#include "qgswebpage.h"
#include "qgswebframe.h"

#include <QDomElement>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QSettings>
#include <QWidget>


QgsHtmlAnnotation::QgsHtmlAnnotation(QObject* parent, QgsVectorLayer* vlayer, bool hasFeature, int feature )
    : QgsAnnotation( parent )
    , mWebPage( nullptr )
    , mVectorLayer( vlayer )
    , mHasAssociatedFeature( hasFeature )
    , mFeatureId( feature )
{
  mWebPage = new QgsWebPage();
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
  mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );

  connect( mWebPage->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &QgsHtmlAnnotation::javascript );

  if ( mVectorLayer )
  {
    connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
#if 0
    QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
#endif
  }

  setFeatureForMapPosition();
}

QgsHtmlAnnotation::~QgsHtmlAnnotation()
{}

void QgsHtmlAnnotation::setSourceFile( const QString& htmlFile )
{
  QFile file( htmlFile );
  mHtmlFile = htmlFile;
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    mHtmlSource = QLatin1String( "" );
  }
  else
  {
    QTextStream in( &file );
    in.setCodec( "UTF-8" );
    mHtmlSource = in.readAll();
  }

  file.close();
  setFeatureForMapPosition();
  emit appearanceChanged();
}
#if 0
void QgsHtmlAnnotation::setMapPosition( const QgsPoint& pos )
{
  QgsAnnotationItem::setMapPosition( pos );
  setFeatureForMapPosition();
}
#endif
void QgsHtmlAnnotation::renderAnnotation( QgsRenderContext& context, QSizeF size ) const
{
  if ( !context.painter() )
  {
    return;
  }

  mWebPage->setViewportSize( size.toSize() );
  mWebPage->mainFrame()->render( context.painter() );
}

QSizeF QgsHtmlAnnotation::minimumFrameSize() const
{
  if ( mWebPage )
  {
    QSizeF widgetMinSize = QSizeF( 0, 0 ); // mWebPage->minimumSize();
    return QSizeF( 2 * frameBorderWidth() + widgetMinSize.width(), 2 * frameBorderWidth() + widgetMinSize.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

void QgsHtmlAnnotation::writeXml( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement formAnnotationElem = doc.createElement( QStringLiteral( "HtmlAnnotationItem" ) );
  if ( mVectorLayer )
  {
    formAnnotationElem.setAttribute( QStringLiteral( "vectorLayer" ), mVectorLayer->id() );
  }
  formAnnotationElem.setAttribute( QStringLiteral( "hasFeature" ), mHasAssociatedFeature );
  formAnnotationElem.setAttribute( QStringLiteral( "feature" ), mFeatureId );
  formAnnotationElem.setAttribute( QStringLiteral( "htmlfile" ), sourceFile() );

  _writeXml( formAnnotationElem, doc );
  elem.appendChild( formAnnotationElem );
}

void QgsHtmlAnnotation::readXml( const QDomElement& itemElem, const QDomDocument& doc )
{
  mVectorLayer = nullptr;
  if ( itemElem.hasAttribute( QStringLiteral( "vectorLayer" ) ) )
  {
    mVectorLayer = dynamic_cast<QgsVectorLayer*>( QgsProject::instance()->mapLayer( itemElem.attribute( QStringLiteral( "vectorLayer" ), QLatin1String( "" ) ) ) );
    if ( mVectorLayer )
    {
      connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
#if 0
      QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
#endif
    }
  }
  mHasAssociatedFeature = itemElem.attribute( QStringLiteral( "hasFeature" ), QStringLiteral( "0" ) ).toInt();
  mFeatureId = itemElem.attribute( QStringLiteral( "feature" ), QStringLiteral( "0" ) ).toInt();
  mHtmlFile = itemElem.attribute( QStringLiteral( "htmlfile" ), QLatin1String( "" ) );
  QDomElement annotationElem = itemElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
  if ( !annotationElem.isNull() )
  {
    _readXml( annotationElem, doc );
  }

  if ( mWebPage )
  {
    setSourceFile( mHtmlFile );
  }
  updateVisibility();
}

void QgsHtmlAnnotation::setFeatureForMapPosition()
{
  QString newText;
  if ( mVectorLayer )
  {
    double halfIdentifyWidth = 0; // QgsMapTool::searchRadiusMU( mMapCanvas );
    QgsRectangle searchRect( mapPosition().x() - halfIdentifyWidth, mapPosition().y() - halfIdentifyWidth,
                             mapPosition().x() + halfIdentifyWidth, mapPosition().y() + halfIdentifyWidth );

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

    QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );
    context.setFeature( mFeature );
    newText = QgsExpression::replaceExpressionText( mHtmlSource, &context );
  }
  else
  {
    newText = mHtmlSource;
  }
  mWebPage->mainFrame()->setHtml( newText );
  emit appearanceChanged();
}

void QgsHtmlAnnotation::updateVisibility()
{
  bool visible = true;
#if 0
  if ( mVectorLayer && mMapCanvas )
  {
    visible = mMapCanvas->layers().contains( mVectorLayer );
  }
#endif
  setVisible( visible );
}

void QgsHtmlAnnotation::javascript()
{
  QWebFrame *frame = mWebPage->mainFrame();
  frame->addToJavaScriptWindowObject( QStringLiteral( "layer" ), mVectorLayer );
}



