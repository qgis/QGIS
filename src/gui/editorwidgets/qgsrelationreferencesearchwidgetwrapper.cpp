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

#include "qgsfields.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrelationreferencewidget.h"
#include "qgsrelationmanager.h"
#include "qgssettings.h"
#include "qgsapplication.h"

#include <QStringListModel>

QgsRelationReferenceSearchWidgetWrapper::QgsRelationReferenceSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QgsMapCanvas *canvas, QWidget *parent )
  : QgsSearchWidgetWrapper( vl, fieldIdx, parent )
  , mCanvas( canvas )
{

}

bool QgsRelationReferenceSearchWidgetWrapper::applyDirectly()
{
  return true;
}

QString QgsRelationReferenceSearchWidgetWrapper::expression() const
{
  return mExpression;
}

QVariant QgsRelationReferenceSearchWidgetWrapper::value() const
{
  if ( !mWidget )
    return QVariant( );

  const QVariantList fkeys = mWidget->foreignKeys();

  if ( fkeys.isEmpty() )
  {
    return QVariant( );
  }
  else
  {
    const QList<QgsRelation::FieldPair> fieldPairs = mWidget->relation().fieldPairs();
    Q_ASSERT( fieldPairs.count() == fkeys.count() );
    for ( int i = 0; i < fieldPairs.count(); i++ )
    {
      if ( fieldPairs.at( i ).referencingField() == layer()->fields().at( fieldIndex() ).name() )
        return fkeys.at( i );
    }
    return QVariant(); // should not happen
  }
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
  const QString fieldName = createFieldIdentifier();

  //clear any unsupported flags
  flags &= supportedFlags();
  if ( flags & IsNull )
    return fieldName + " IS NULL";
  if ( flags & IsNotNull )
    return fieldName + " IS NOT NULL";

  const QVariant v = value();
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
      {
        if ( v.isNull() )
          return fieldName + " IS NULL";
        return fieldName + '=' + v.toString();
      }
      else if ( flags & NotEqualTo )
      {
        if ( v.isNull() )
          return fieldName + " IS NOT NULL";
        return fieldName + "<>" + v.toString();
      }
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

void QgsRelationReferenceSearchWidgetWrapper::onValueChanged( const QVariant &value )
{
  onValuesChanged( QVariantList() << value );
}

void QgsRelationReferenceSearchWidgetWrapper::onValuesChanged( const QVariantList &values )
{
  if ( values.isEmpty() )
  {
    clearExpression();
    emit valueCleared();
  }
  else
  {
    const QgsSettings settings;
    // TODO: adapt for composite keys
    const QVariant value = values.at( 0 );
    setExpression( value.isNull() ? QgsApplication::nullRepresentation() : value.toString() );
    emit valueChanged();
  }
  emit expressionChanged( mExpression );
}

void QgsRelationReferenceSearchWidgetWrapper::setExpression( const QString &expression )
{
  QString exp = expression;
  const QString nullValue = QgsApplication::nullRepresentation();
  const QString fieldName = layer()->fields().at( mFieldIdx ).name();

  QString str;
  if ( exp == nullValue )
  {
    str = QStringLiteral( "%1 IS NULL" ).arg( QgsExpression::quotedColumnRef( fieldName ) );
  }
  else
  {
    str = QStringLiteral( "%1 = '%3'" )
          .arg( QgsExpression::quotedColumnRef( fieldName ),
                exp.replace( '\'', QLatin1String( "''" ) )
              );
  }
  mExpression = str;
}

QWidget *QgsRelationReferenceSearchWidgetWrapper::createWidget( QWidget *parent )
{
  return new QgsRelationReferenceWidget( parent );
}

void QgsRelationReferenceSearchWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsRelationReferenceWidget *>( editor );
  if ( !mWidget )
    return;

  mWidget->setEditorContext( context(), mCanvas, nullptr );

  mWidget->setEmbedForm( false );
  mWidget->setReadOnlySelector( false );
  mWidget->setAllowMapIdentification( config( QStringLiteral( "MapIdentification" ), false ).toBool() );
  mWidget->setOrderByValue( config( QStringLiteral( "OrderByValue" ), false ).toBool() );
  mWidget->setAllowAddFeatures( false );
  mWidget->setOpenFormButtonVisible( false );

  if ( config( QStringLiteral( "FilterFields" ), QVariant() ).isValid() )
  {
    mWidget->setFilterFields( config( QStringLiteral( "FilterFields" ) ).toStringList() );
    mWidget->setChainFilters( config( QStringLiteral( "ChainFilters" ) ).toBool() );
    mWidget->setFilterExpression( config( QStringLiteral( "FilterExpression" ) ).toString() );
  }

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config( QStringLiteral( "Relation" ) ).toString() );
  // if no relation is given from the config, fetch one if there is only one available
  if ( !relation.isValid() && !layer()->referencingRelations( mFieldIdx ).isEmpty() && layer()->referencingRelations( mFieldIdx ).count() == 1 )
    relation = layer()->referencingRelations( mFieldIdx )[0];
  mWidget->setRelation( relation, config( QStringLiteral( "AllowNULL" ) ).toBool() );

  mWidget->showIndeterminateState();
  connect( mWidget, &QgsRelationReferenceWidget::foreignKeysChanged, this, &QgsRelationReferenceSearchWidgetWrapper::onValuesChanged );
}


