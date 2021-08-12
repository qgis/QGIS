/***************************************************************************
  qgshtmlwidgetwrapper.h

 ---------------------
 begin                : 23.03.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshtmlwidgetwrapper.h"
#include "qgsmessagelog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsapplication.h"
#include "qgswebframe.h"
#include <QScreen>

QgsHtmlWidgetWrapper::QgsHtmlWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
  connect( this, &QgsWidgetWrapper::contextChanged, this, &QgsHtmlWidgetWrapper::setHtmlContext );
}

bool QgsHtmlWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsHtmlWidgetWrapper::createWidget( QWidget *parent )
{
  return new QgsWebView( parent );
}

void QgsHtmlWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsWebView *>( editor );

  if ( !mWidget )
    return;

  mWidget->setHtml( mHtmlCode );
#ifdef WITH_QTWEBKIT

  const int horizontalDpi = mWidget->logicalDpiX();

  mWidget->setZoomFactor( horizontalDpi / 96.0 );

  QWebPage *page = mWidget->page();
  connect( page, &QWebPage::contentsChanged, this, &QgsHtmlWidgetWrapper::fixHeight, Qt::ConnectionType::UniqueConnection );
  connect( page, &QWebPage::loadFinished, this, [ = ]( bool ) { fixHeight(); }, Qt::ConnectionType::UniqueConnection );

#endif

  checkGeometryNeeds();
}

void QgsHtmlWidgetWrapper::reinitWidget( )
{
  if ( !mWidget )
    return;

  initWidget( mWidget );
}

void QgsHtmlWidgetWrapper::checkGeometryNeeds()
{
  if ( !mWidget )
    return;

  // initialize a temporary QgsWebView to render HTML code and check if one evaluated expression
  // needs geometry
  QgsWebView webView;
  NeedsGeometryEvaluator evaluator;

  const QgsAttributeEditorContext attributecontext = context();
  const QgsExpressionContext expressionContext = layer()->createExpressionContext();
  evaluator.setExpressionContext( expressionContext );

  auto frame = webView.page()->mainFrame();
  connect( frame, &QWebFrame::javaScriptWindowObjectCleared, frame, [ frame, &evaluator ]
  {
    frame->addToJavaScriptWindowObject( QStringLiteral( "expression" ), &evaluator );
  } );

  webView.setHtml( mHtmlCode );

  mNeedsGeometry = evaluator.needsGeometry();
}

void QgsHtmlWidgetWrapper::setHtmlCode( const QString &htmlCode )
{
  mHtmlCode = htmlCode;
  checkGeometryNeeds();
}

void QgsHtmlWidgetWrapper::setHtmlContext( )
{
  if ( !mWidget )
    return;

  const QgsAttributeEditorContext attributecontext = context();
  QgsExpressionContext expressionContext = layer()->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::formScope( mFeature, attributecontext.attributeFormModeString() );
  if ( attributecontext.parentFormFeature().isValid() )
  {
    expressionContext << QgsExpressionContextUtils::parentFormScope( attributecontext.parentFormFeature() );
  }
  expressionContext.setFeature( mFeature );

  HtmlExpression *htmlExpression = new HtmlExpression();
  htmlExpression->setExpressionContext( expressionContext );
  auto frame = mWidget->page()->mainFrame();
  connect( frame, &QWebFrame::javaScriptWindowObjectCleared, frame, [ = ]
  {
    frame->addToJavaScriptWindowObject( QStringLiteral( "expression" ), htmlExpression );
  } );

  mWidget->setHtml( mHtmlCode );
}

#ifdef WITH_QTWEBKIT
void QgsHtmlWidgetWrapper::fixHeight()
{
  QWebPage *page = mWidget->page();
  const int docHeight { page->mainFrame()->contentsSize().height() };
  mWidget->setFixedHeight( docHeight );
}
#endif

void QgsHtmlWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( !mWidget )
    return;

  mFeature = feature;
  setHtmlContext();
}

bool QgsHtmlWidgetWrapper::needsGeometry() const
{
  return mNeedsGeometry;
}


///@cond PRIVATE
void HtmlExpression::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}

QString HtmlExpression::evaluate( const QString &expression ) const
{
  QgsExpression exp = QgsExpression( expression );
  exp.prepare( &mExpressionContext );
  return exp.evaluate( &mExpressionContext ).toString();
}

void NeedsGeometryEvaluator::evaluate( const QString &expression )
{
  QgsExpression exp = QgsExpression( expression );
  exp.prepare( &mExpressionContext );
  mNeedsGeometry |= exp.needsGeometry();
}

void NeedsGeometryEvaluator::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}


///@endcond
