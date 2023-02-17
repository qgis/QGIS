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
#include "qgsexpressioncontextutils.h"
#include "qgswebframe.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgsattributeform.h"
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

  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );

  if ( form )
  {
    mFormFeature = form->feature();
    connect( form, &QgsAttributeForm::widgetValueChanged, this, [ = ]( const QString & attribute, const QVariant & newValue, bool attributeChanged )
    {
      if ( attributeChanged )
      {
        const QRegularExpression expRe { QStringLiteral( R"re(expression.evaluate\s*\(\s*"(.*)"\))re" ), QRegularExpression::PatternOption::MultilineOption | QRegularExpression::PatternOption::DotMatchesEverythingOption };
        const QRegularExpressionMatch match { expRe.match( mHtmlCode ) };
        if ( match.hasMatch() && QgsValueRelationFieldFormatter::expressionRequiresFormScope( match.captured( 1 ) ) )
        {
          mFormFeature.setAttribute( attribute, newValue );
          setHtmlContext();
        }
      }
    } );
  }

  return new QgsWebView( parent );
}

void QgsHtmlWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsWebView *>( editor );

  if ( !mWidget )
    return;

  mWidget->setHtml( mHtmlCode.replace( "\n", " " ) );

#ifdef WITH_QTWEBKIT

  const int horizontalDpi = mWidget->logicalDpiX();

  mWidget->setZoomFactor( horizontalDpi / 96.0 );

  QWebPage *page = mWidget->page();
  connect( page, &QWebPage::contentsChanged, this, &QgsHtmlWidgetWrapper::fixHeight, Qt::ConnectionType::UniqueConnection );
  connect( page, &QWebPage::loadFinished, this, &QgsHtmlWidgetWrapper::fixHeight, Qt::ConnectionType::UniqueConnection );

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
  if ( QgsVectorLayer *vl = layer() )
  {
    const QgsExpressionContext expressionContext = vl->createExpressionContext();
    evaluator.setExpressionContext( expressionContext );
  }

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
  expressionContext << QgsExpressionContextUtils::formScope( mFormFeature, attributecontext.attributeFormModeString() );
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
  mFormFeature = feature;
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
  QgsExpression exp { expression };
  exp.prepare( &mExpressionContext );
  return exp.evaluate( &mExpressionContext ).toString();
}

void NeedsGeometryEvaluator::evaluate( const QString &expression )
{
  QgsExpression exp { expression };
  exp.prepare( &mExpressionContext );
  mNeedsGeometry |= exp.needsGeometry();
}

void NeedsGeometryEvaluator::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}


///@endcond
