/***************************************************************************
  qgsqmlwidgetwrapper.cpp

 ---------------------
 begin                : 25.6.2018
 copyright            : (C) 2018 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsqmlwidgetwrapper.h"
#include "qgsmessagelog.h"
#include "qgsexpressioncontextutils.h"

#include <QtQuickWidgets/QQuickWidget>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

QgsQmlWidgetWrapper::QgsQmlWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
  connect( this, &QgsWidgetWrapper::contextChanged, this, &QgsQmlWidgetWrapper::setQmlContext );
}

bool QgsQmlWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsQmlWidgetWrapper::createWidget( QWidget *parent )
{
  return new QQuickWidget( parent );
}

void QgsQmlWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QQuickWidget *>( editor );

  if ( !mWidget )
    return;


  if ( !mQmlFile.open() )
  {
    QgsMessageLog::logMessage( tr( "Failed to open temporary QML file" ) );
    return;
  }

  mWidget->setSource( QUrl::fromLocalFile( mQmlFile.fileName() ) );

  mQmlFile.close();
}


void QgsQmlWidgetWrapper::reinitWidget( )
{
  if ( !mWidget )
    return;

  mWidget->engine()->clearComponentCache();

  initWidget( mWidget );
}

void QgsQmlWidgetWrapper::setQmlCode( const QString &qmlCode )
{
  if ( !mQmlFile.open() )
  {
    QgsMessageLog::logMessage( tr( "Failed to open temporary QML file" ) );
    return;
  }

  mQmlFile.resize( 0 );
  mQmlFile.write( qmlCode.toUtf8() );

  mQmlFile.close();
}

void QgsQmlWidgetWrapper::setQmlContext( )
{
  if ( !mWidget )
    return;

  const QgsAttributeEditorContext attributecontext = context();
  QgsExpressionContext expressionContext = layer()->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::formScope( mFeature, attributecontext.attributeFormModeString() );
  expressionContext.setFeature( mFeature );

  QmlExpression *qmlExpression = new QmlExpression();
  qmlExpression->setExpressionContext( expressionContext );

  mWidget->rootContext()->setContextProperty( "expression", qmlExpression );
}

void QgsQmlWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( !mWidget )
    return;

  mFeature = feature;

  setQmlContext();
}

///@cond PRIVATE
void QmlExpression::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
}

QVariant QmlExpression::evaluate( const QString &expression ) const
{
  QgsExpression exp = QgsExpression( expression );
  exp.prepare( &mExpressionContext );
  return exp.evaluate( &mExpressionContext );
}
///@endcond
