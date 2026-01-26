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

#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgswebframe.h"
#include "qgswebpage.h"

#include <QDir>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QSettings>
#include <QTextStream>
#include <QWidget>

#include "moc_qgshtmlannotation.cpp"

QgsHtmlAnnotation::QgsHtmlAnnotation( QObject *parent )
  : QgsAnnotation( parent )
{
  mWebPage = new QgsWebPage();
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
  mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );
  mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );

  // Make QWebPage transparent so that the background color of the annotation frame is used
  QPalette palette = mWebPage->palette();
  palette.setBrush( QPalette::Base, Qt::transparent );
  mWebPage->setPalette( palette );

  connect( mWebPage->mainFrame(), &QWebFrame::javaScriptWindowObjectCleared, this, &QgsHtmlAnnotation::javascript );
}

QgsHtmlAnnotation *QgsHtmlAnnotation::clone() const
{
  auto c = std::make_unique<QgsHtmlAnnotation>();
  copyCommonProperties( c.get() );
  c->setSourceFile( mHtmlFile );
  return c.release();
}

void QgsHtmlAnnotation::setSourceFile( const QString &htmlFile )
{
  QFile file( htmlFile );
  mHtmlFile = htmlFile;
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    mHtmlSource.clear();
  }
  else
  {
    QTextStream in( &file );
    mHtmlSource = in.readAll();
  }

  file.close();
  setAssociatedFeature( associatedFeature() );
  emit appearanceChanged();
}

void QgsHtmlAnnotation::setHtmlSource( const QString &htmlSource )
{
  mHtmlFile.clear();
  mHtmlSource = htmlSource;
  setAssociatedFeature( associatedFeature() );
  emit appearanceChanged();
}

void QgsHtmlAnnotation::renderAnnotation( QgsRenderContext &context, QSizeF size ) const
{
  if ( !context.painter() || ( context.feedback() && context.feedback()->isCanceled() ) )
  {
    return;
  }

  // scale painter back to 96 dpi, so layout prints match screen rendering
  const QgsScopedQPainterState painterState( context.painter() );
  const double scaleFactor = context.painter()->device()->logicalDpiX() / 96.0;
  context.painter()->scale( scaleFactor, scaleFactor );
  size /= scaleFactor;

  mWebPage->setViewportSize( size.toSize() );
  mWebPage->mainFrame()->render( context.painter() );
}

QSizeF QgsHtmlAnnotation::minimumFrameSize() const
{
  if ( mWebPage )
  {
    const QSizeF widgetMinSize = QSizeF( 0, 0 ); // mWebPage->minimumSize();
    return QSizeF( contentsMargin().left() + contentsMargin().right() + widgetMinSize.width(),
                   contentsMargin().top() + contentsMargin().bottom() + widgetMinSize.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

void QgsHtmlAnnotation::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement formAnnotationElem = doc.createElement( u"HtmlAnnotationItem"_s );
  if ( !mHtmlFile.isEmpty() )
  {
    formAnnotationElem.setAttribute( u"htmlfile"_s, sourceFile() );
  }
  else
  {
    formAnnotationElem.setAttribute( u"htmlsource"_s, mHtmlSource );
  }

  _writeXml( formAnnotationElem, doc, context );
  elem.appendChild( formAnnotationElem );
}

void QgsHtmlAnnotation::readXml( const QDomElement &itemElem, const QgsReadWriteContext &context )
{
  const QDomElement annotationElem = itemElem.firstChildElement( u"AnnotationItem"_s );
  if ( !annotationElem.isNull() )
  {
    _readXml( annotationElem, context );
  }

  // upgrade old layer
  if ( !mapLayer() && itemElem.hasAttribute( u"vectorLayer"_s ) )
  {
    setMapLayer( QgsProject::instance()->mapLayer( itemElem.attribute( u"vectorLayer"_s ) ) ); // skip-keyword-check
  }

  if ( mWebPage )
  {
    mHtmlFile = itemElem.attribute( u"htmlfile"_s, QString() );
    if ( !mHtmlFile.isEmpty() )
    {
      setSourceFile( mHtmlFile );
    }
    else
    {
      setHtmlSource( itemElem.attribute( u"htmlsource"_s, QString() ) );
    }
  }
}

void QgsHtmlAnnotation::setAssociatedFeature( const QgsFeature &feature )
{
  QgsAnnotation::setAssociatedFeature( feature );
  QString newText;
  QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( mapLayer() );
  if ( feature.isValid() && vectorLayer )
  {
    QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vectorLayer ) );
    context.setFeature( feature );
    newText = QgsExpression::replaceExpressionText( mHtmlSource, &context );
  }
  else
  {
    newText = mHtmlSource;
  }
  mWebPage->mainFrame()->setHtml( newText );
  emit appearanceChanged();
}

void QgsHtmlAnnotation::javascript()
{
  QWebFrame *frame = mWebPage->mainFrame();
  frame->addToJavaScriptWindowObject( u"layer"_s, mapLayer() );
}
