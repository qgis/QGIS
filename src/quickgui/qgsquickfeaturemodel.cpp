/***************************************************************************
 qgsquickfeaturemodel.cpp
  --------------------------------------
  Date                 : 10.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsmessagelog.h"
#include "qgsvectorlayer.h"

#include "qgsquickfeaturemodel.h"

QgsQuickFeatureModel::QgsQuickFeatureModel( QObject *parent )
  : QAbstractListModel( parent )
{
  connect( this, &QgsQuickFeatureModel::modelReset, this, &QgsQuickFeatureModel::featureChanged );
}

QgsQuickFeature QgsQuickFeatureModel::feature() const
{
  return mFeature;
}

void QgsQuickFeatureModel::setFeature( const QgsQuickFeature &feature )
{
  setFeatureOnly( feature.feature() );
  setLayer( feature.layer() );
}

void QgsQuickFeatureModel::setFeatureOnly( const QgsFeature &feature )
{
  if ( mFeature.feature() == feature )
    return;

  beginResetModel();
  mFeature.setFeature( feature );
  endResetModel();
  emit featureChanged();
}

void QgsQuickFeatureModel::setLayer( QgsVectorLayer *layer )
{
  if ( layer == mFeature.layer() )
    return;

  mFeature.setLayer( layer );
  if ( mFeature.layer() )
  {
    mFeature.setFeature( QgsFeature( mFeature.layer()->fields() ) );

    mRememberedAttributes.resize( layer->fields().size() );
    mRememberedAttributes.fill( false );
  }

  emit featureChanged();
}

QHash<int, QByteArray> QgsQuickFeatureModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[AttributeName]  = QByteArrayLiteral( "AttributeName" );
  roles[AttributeValue] = QByteArrayLiteral( "AttributeValue" );
  roles[Field] = QByteArrayLiteral( "Field" );
  roles[RememberAttribute] = QByteArrayLiteral( "RememberAttribute" );

  return roles;
}


int QgsQuickFeatureModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeature.feature().attributes().count();
}

QVariant QgsQuickFeatureModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case AttributeName:
      return mFeature.layer()->attributeDisplayName( index.row() );
      break;

    case AttributeValue:
      return mFeature.feature().attribute( index.row() );
      break;

    case Field:
      return mFeature.layer()->fields().at( index.row() );
      break;

    case RememberAttribute:
      return mRememberedAttributes.at( index.row() );
      break;
  }

  return QVariant();
}

bool QgsQuickFeatureModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( data( index, role ) == value )
    return true;

  switch ( role )
  {
    case AttributeValue:
    {
      QVariant val( value );
      QgsField fld = mFeature.feature().fields().at( index.row() );

      if ( !fld.convertCompatible( val ) )
      {
        QgsMessageLog::logMessage( tr( "Value \"%1\" %4 could not be converted to a compatible value for field %2(%3)." ).arg( value.toString(), fld.name(), fld.typeName(), value.isNull() ? "NULL" : "NOT NULL" ) );
        return false;
      }
      bool success = mFeature.feature().setAttribute( index.row(), val );
      if ( success )
        emit dataChanged( index, index, QVector<int>() << role );
      return success;
      break;
    }

    case RememberAttribute:
    {
      mRememberedAttributes[ index.row() ] = value.toBool();
      emit dataChanged( index, index, QVector<int>() << role );
      break;
    }
  }

  return false;
}

bool QgsQuickFeatureModel::save()
{
  if ( !mFeature.layer() )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  QgsFeature feat = mFeature.feature();
  if ( !mFeature.layer()->updateFeature( feat ) )
    QgsMessageLog::logMessage( tr( "Cannot update feature" ), QStringLiteral( "QgsQuick" ), Qgis::Warning );
  rv = commit();

  if ( rv )
  {
    QgsFeature feat;
    if ( mFeature.layer()->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.feature().id() ) ).nextFeature( feat ) )
      setFeatureOnly( feat );
    else
      QgsMessageLog::logMessage( tr( "Feature %1 could not be fetched after commit" ).arg( mFeature.feature().id() ),
                                 QStringLiteral( "QgsQuick" ),
                                 Qgis::Warning );
  }
  return rv;
}

bool QgsQuickFeatureModel::deleteFeature()
{
  if ( !mFeature.layer() )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  if ( !mFeature.layer()->deleteFeature( mFeature.feature().id() ) )
    QgsMessageLog::logMessage( tr( "Cannot delete feature" ), QStringLiteral( "QgsQuick" ), Qgis::Warning );
  rv = commit();

  return rv;
}

void QgsQuickFeatureModel::reset()
{
  if ( !mFeature.layer() )
    return;

  mFeature.layer()->rollBack();
}

bool QgsQuickFeatureModel::suppressFeatureForm() const
{
  if ( !mFeature.layer() )
    return false;

  return mFeature.layer()->editFormConfig().suppress();
}

void QgsQuickFeatureModel::resetAttributes()
{
  if ( !mFeature.layer() )
    return;

  QgsExpressionContext expressionContext = mFeature.layer()->createExpressionContext();
  expressionContext.setFeature( mFeature.feature() );

  QgsFields fields = mFeature.layer()->fields();

  beginResetModel();
  for ( int i = 0; i < fields.count(); ++i )
  {
    if ( !mRememberedAttributes.at( i ) )
    {
      if ( !fields.at( i ).defaultValueDefinition().expression().isEmpty() )
      {
        QgsExpression exp( fields.at( i ).defaultValueDefinition().expression() );
        exp.prepare( &expressionContext );
        if ( exp.hasParserError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has parser error: %3" ).arg( mFeature.layer()->name(),
                                     fields.at( i ).name(), exp.parserErrorString() ), QStringLiteral( "QgsQuick" ),
                                     Qgis::Warning );

        QVariant value = exp.evaluate( &expressionContext );

        if ( exp.hasEvalError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has evaluation error: %3" ).arg( mFeature.layer()->name(),
                                     fields.at( i ).name(), exp.evalErrorString() ),
                                     QStringLiteral( "QgsQuick" ),
                                     Qgis::Warning );

        mFeature.feature().setAttribute( i, value );
      }
      else
      {
        mFeature.feature().setAttribute( i, QVariant() );
      }
    }
  }
  endResetModel();
}

void QgsQuickFeatureModel::create()
{
  if ( !mFeature.layer() )
    return;

  startEditing();
  QgsFeature feat = mFeature.feature();
  if ( !mFeature.layer()->addFeature( feat ) )
  {
    QgsMessageLog::logMessage( tr( "Feature could not be added" ), QStringLiteral( "QgsQuick" ), Qgis::Critical );
  }
  commit();
}

bool QgsQuickFeatureModel::commit()
{
  if ( !mFeature.layer()->commitChanges() )
  {
    QgsMessageLog::logMessage( tr( "Could not save changes. Rolling back." ), QStringLiteral( "QgsQuick" ), Qgis::Critical );
    mFeature.layer()->rollBack();
    return false;
  }
  else
  {
    return true;
  }
}

bool QgsQuickFeatureModel::startEditing()
{
  // Already an edit session active
  if ( mFeature.layer()->editBuffer() )
    return true;

  if ( !mFeature.layer()->startEditing() )
  {
    QgsMessageLog::logMessage( tr( "Cannot start editing" ), QStringLiteral( "QgsQuick" ), Qgis::Warning );
    return false;
  }
  else
  {
    return true;
  }
}

QVector<bool> QgsQuickFeatureModel::rememberedAttributes() const
{
  return mRememberedAttributes;
}
