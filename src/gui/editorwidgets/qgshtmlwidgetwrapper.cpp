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

#include <QWebFrame>

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
  const int horizontalDpi = qApp->desktop()->screen()->logicalDpiX();
  mWidget->setZoomFactor( horizontalDpi / 96.0 );

  auto page = mWidget->page();
  connect( page, &QWebPage::contentsChanged, this, [ = ]
  {
    auto docHeight { page->mainFrame()->contentsSize().height() };
    mWidget->setFixedHeight( docHeight );
  }, Qt::ConnectionType::UniqueConnection );

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
  connect( frame, &QWebFrame::javaScriptWindowObjectCleared, [ = ]
  {
    frame->addToJavaScriptWindowObject( QStringLiteral( "expression" ), htmlExpression );
  } );

  mWidget->setHtml( mHtmlCode );
}

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
