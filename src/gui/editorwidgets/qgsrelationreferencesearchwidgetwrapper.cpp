/***************************************************************************
    qgsrelationreferencesearchwidgetwrapper.cpp
     ------------------------------------------
    Date                 : 2016-05-25
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationreferencesearchwidgetwrapper.h"

#include "qgsfield.h"
#include "qgsmaplayerregistry.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

#include <QSettings>
#include <QStringListModel>

QgsRelationReferenceSearchWidgetWrapper::QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QgsMapCanvas* canvas, QWidget* parent )
    : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
    , mWidget( nullptr )
    , mLayer( nullptr )
    , mCanvas( canvas )
{

}

bool QgsRelationReferenceSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsRelationReferenceSearchWidgetWrapper::expression()
{
  return mExpression;
}

QVariant QgsRelationReferenceSearchWidgetWrapper::value() const
{
  if ( mWidget )
  {
    return mWidget->foreignKey();
  }
  return QVariant();
}

QgsSearchWidgetWrapper::FilterFlags QgsRelationReferenceSearchWidgetWrapper::supportedFlags() const
{
  return EqualTo | NotEqualTo | IsNull | IsNotNull;
}

QgsSearchWidgetWrapper::FilterFlags QgsRelationReferenceSearchWidgetWrapper::defaultFlags() const
{
  return EqualTo;
}

QString QgsRelationReferenceSearchWidgetWrapper::createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const
{
  QString fieldName = QgsExpression::quotedColumnRef( layer()->fields().at( mFieldIdx ).name() );

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  QVariant v = value();
  if ( !v.isValid() )
    return QString();

  switch ( v.type() )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    {
      if ( flags & EqualTo )
        return fieldName + '=' + v.toString();
      else if ( flags & NotEqualTo )
        return fieldName + "<>" + v.toString();
      break;
    }

    default:
    {
      if ( flags & EqualTo )
        return fieldName + "='" + v.toString() + '\'';
      else if ( flags & NotEqualTo )
        return fieldName + "<>'" + v.toString() + '\'';
      break;
    }
  }

  return QString();
}

void QgsRelationReferenceSearchWidgetWrapper::clearWidget()
{
  if ( mWidget )
  {
    mWidget->showIndeterminateState();
  }
}

void QgsRelationReferenceSearchWidgetWrapper::setEnabled( bool enabled )
{
  if ( mWidget )
  {
    mWidget->setEnabled( enabled );
  }
}

bool QgsRelationReferenceSearchWidgetWrapper::valid() const
{
  return true;
}

void QgsRelationReferenceSearchWidgetWrapper::onValueChanged( QVariant value )
{
  if ( !value.isValid() )
  {
    clearExpression();
    emit valueCleared();
  }
  else
  {
    QSettings settings;
    setExpression( value.isNull() ? settings.value( "qgis/nullValue", "NULL" ).toString() : value.toString() );
    emit valueChanged();
  }
  emit expressionChanged( mExpression );
}

void QgsRelationReferenceSearchWidgetWrapper::setExpression( QString exp )
{
  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();
  QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str;
  if ( exp == nullValue )
  {
    str = QString( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QString( "%1 = '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ),
                exp.replace( '\'', "''" )
              );
  }
  mExpression = str;
}

QWidget* QgsRelationReferenceSearchWidgetWrapper::createWidget( QWidget* parent )
{
  return new QgsRelationReferenceWidget( parent );
}

void QgsRelationReferenceSearchWidgetWrapper::initWidget( QWidget* editor )
{
  mWidget = qobject_cast<QgsRelationReferenceWidget*>( editor );
  if ( !mWidget )
    return;

  mWidget->setEditorContext( context(), mCanvas, nullptr );

  mWidget->setEmbedForm( false );
  mWidget->setReadOnlySelector( false );
  mWidget->setAllowMapIdentification( config( "MapIdentification", false ).toBool() );
  mWidget->setOrderByValue( config( "OrderByValue", false ).toBool() );
  mWidget->setAllowAddFeatures( false );
  mWidget->setOpenFormButtonVisible( false );

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config( "Relation" ).toString() );
  mWidget->setRelation( relation, false );

  mWidget->showIndeterminateState();
  connect( mWidget, SIGNAL( foreignKeyChanged( QVariant ) ), this, SLOT( onValueChanged( QVariant ) ) );
}


