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
  const int horizontalDpi = qApp->desktop()->screen()->logicalDpiX();
  mWidget->setZoomFactor( horizontalDpi / 96.0 );

  QWebPage *page = mWidget->page();
  connect( page, &QWebPage::contentsChanged, this, &QgsHtmlWidgetWrapper::fixHeight, Qt::ConnectionType::UniqueConnection );
#endif

}


void QgsHtmlWidgetWrapper::reinitWidget( )
{
  if ( !mWidget )
    return;

  initWidget( mWidget );
}

void QgsHtmlWidgetWrapper::setHtmlCode( const QString &htmlCode )
{
  mHtmlCode = htmlCode;
}

void QgsHtmlWidgetWrapper::setHtmlContext( )
{
  if ( !mWidget )
    return;

  QgsAttributeEditorContext attributecontext = context();
  QgsExpressionContext expressionContext = layer()->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::formScope( mFeature, attributecontext.attributeFormModeString() );
  expressionContext.setFeature( mFeature );

  HtmlExpression *htmlExpression = new HtmlExpression();
  htmlExpression->setExpressionContext( expressionContext );
  mWidget->page()->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
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
  int docHeight { page->mainFrame()->contentsSize().height() };
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
///@endcond
