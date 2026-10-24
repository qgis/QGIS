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

#include "qgsexpressioncontextutils.h"
#include "qgsmessagelog.h"
#include "qgsvaluerelationfieldformatter.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "moc_qgsqmlwidgetwrapper.cpp"

using namespace Qt::StringLiterals;

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
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );

  if ( form )
  {
    mFormFeature = form->feature();
    connect( form, &QgsAttributeForm::widgetValueChanged, this, [this]( const QString &attribute, const QVariant &newValue, bool attributeChanged ) {
      if ( attributeChanged )
      {
        if ( mRequiresFormScope )
        {
          mFormFeature.setAttribute( attribute, newValue );
          setQmlContext();
        }
      }
    } );
  }

  QQuickWidget *widget = new QQuickWidget( parent );

  return widget;
}

void QgsQmlWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QQuickWidget *>( editor );

  if ( !mWidget )
  {
    return;
  }

  if ( !mQmlFile.open() )
  {
    QgsMessageLog::logMessage( tr( "Failed to open temporary QML file" ) );
    return;
  }

  if ( mWidgetBridge )
  {
    mWidgetBridge->deleteLater();
    mWidgetBridge = nullptr;
  }
  setQmlContext();

  QUrl sourceUrl = QUrl::fromLocalFile( mQmlFile.fileName() );
  sourceUrl.setQuery( u"t=%1"_s.arg( QDateTime::currentSecsSinceEpoch() ) );
  mWidget->setSource( sourceUrl );

  mQmlFile.close();
}


void QgsQmlWidgetWrapper::reinitWidget()
{
  if ( !mWidget )
  {
    return;
  }

  mWidget->engine()->clearComponentCache();

  initWidget( mWidget );
}

void QgsQmlWidgetWrapper::setQmlCode( const QString &qmlCode )
{
  if ( mQmlCode == qmlCode )
  {
    return;
  }

  mQmlCode = qmlCode;

  bool ok = false;
  const thread_local QRegularExpression
    expRe( QStringLiteral( R"re(expression.evaluate\s*\(\s*"(.*)"\))re" ), QRegularExpression::PatternOption::MultilineOption | QRegularExpression::PatternOption::DotMatchesEverythingOption );
  QRegularExpressionMatchIterator matchIt = expRe.globalMatch( mQmlCode );
  while ( !ok && matchIt.hasNext() )
  {
    const QRegularExpressionMatch match = matchIt.next();
    const QgsExpression exp = match.captured( 1 );
    ok = QgsValueRelationFieldFormatter::expressionRequiresFormScope( exp );
  }
  mRequiresFormScope = ok;

  if ( !mQmlFile.open() )
  {
    QgsMessageLog::logMessage( tr( "Failed to open temporary QML file" ) );
    return;
  }

  mQmlFile.resize( 0 );
  mQmlFile.write( mQmlCode.toUtf8() );
  mQmlFile.close();
}

void QgsQmlWidgetWrapper::setQmlContext()
{
  if ( !mWidget )
  {
    return;
  }

  const QgsAttributeEditorContext attributeEditorContext = context();

  if ( !mWidgetBridge )
  {
    QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent() );
    mWidgetBridge = new QgsQmlAttributeFormBridge( form, this );
    mWidget->rootContext()->setContextProperty( "attributeFormBridge", mWidgetBridge );
  }
  mWidgetBridge->setContext( attributeEditorContext );

  QgsExpressionContext expressionContext = layer()->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::formScope( mFormFeature, attributeEditorContext.attributeFormModeString() );
  if ( attributeEditorContext.parentFormFeature().isValid() )
  {
    expressionContext << QgsExpressionContextUtils::parentFormScope( attributeEditorContext.parentFormFeature() );
  }
  expressionContext.setFeature( mFeature );

  QmlExpression *qmlExpression = new QmlExpression();
  qmlExpression->setExpressionContext( expressionContext );

  mWidget->rootContext()->setContextProperty( "expression", qmlExpression );
}

void QgsQmlWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( !mWidget )
  {
    return;
  }

  mFeature = feature;
  mFormFeature = feature;

  emit mWidgetBridge->currentFormFeatureChanged();

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


QgsQmlAttributeFormBridge::QgsQmlAttributeFormBridge( QgsAttributeForm *form, QObject *parent )
  : QObject( parent )
  , mForm( form )
{
  if ( mForm )
  {
    connect( mForm, &QgsAttributeForm::widgetValueChanged, this, [this]( const QString &, const QVariant &, bool attributeChanged ) {
      if ( attributeChanged )
      {
        emit currentFormFeatureChanged();
      }
    } );

    connect( mForm, &QgsAttributeForm::editableChanged, this, &QgsQmlAttributeFormBridge::editableChanged );
  }
}

bool QgsQmlAttributeFormBridge::editable() const
{
  return mForm ? mForm->editable() : false;
}

QgsAttributeEditorContext QgsQmlAttributeFormBridge::context() const
{
  return mContext;
}

void QgsQmlAttributeFormBridge::setContext( const QgsAttributeEditorContext &context )
{
  mContext = context;
  emit contextChanged();
}

QgsVectorLayer *QgsQmlAttributeFormBridge::layer() const
{
  return mForm ? mForm->layer() : nullptr;
}

QgsFeature QgsQmlAttributeFormBridge::currentFormFeature() const
{
  return mForm ? mForm->currentFormFeature() : QgsFeature();
}

void QgsQmlAttributeFormBridge::changeGeometry( const QgsGeometry &geometry )
{
  if ( mForm )
  {
    mForm->changeGeometry( geometry );
  }
}

void QgsQmlAttributeFormBridge::changeAttribute( const QString &field, const QVariant &value )
{
  if ( mForm )
  {
    mForm->changeAttribute( field, value );
  }
}

QVariant QgsQmlAttributeFormBridge::attribute( const QString &field )
{
  return mForm ? mForm->feature().attribute( field ) : QVariant();
}
///@endcond
