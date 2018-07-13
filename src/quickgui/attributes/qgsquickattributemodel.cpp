/***************************************************************************
 qgsquickattributemodel.cpp
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

#include "qgsquickattributemodel.h"

QgsQuickAttributeModel::QgsQuickAttributeModel( QObject *parent )
  : QAbstractListModel( parent )
{
  connect( this, &QgsQuickAttributeModel::modelReset, this, &QgsQuickAttributeModel::featureLayerPairChanged );
  connect( this, &QgsQuickAttributeModel::featureChanged, this, &QgsQuickAttributeModel::featureLayerPairChanged );
  connect( this, &QgsQuickAttributeModel::layerChanged, this, &QgsQuickAttributeModel::featureLayerPairChanged );
}

QgsQuickFeatureLayerPair QgsQuickAttributeModel::featureLayerPair() const
{
  return mFeatureLayerPair;
}

void QgsQuickAttributeModel::setFeatureLayerPair( const QgsQuickFeatureLayerPair &pair )
{
  setVectorLayer( pair.layer() );
  setFeature( pair.feature() );
}


void QgsQuickAttributeModel::setVectorLayer( QgsVectorLayer *layer )
{
  if ( mFeatureLayerPair.layer() == layer )
    return;

  beginResetModel();
  mFeatureLayerPair = QgsQuickFeatureLayerPair( mFeatureLayerPair.feature(), layer );


  if ( mFeatureLayerPair.layer() )
  {
    mRememberedAttributes.resize( mFeatureLayerPair.layer()->fields().size() );
    mRememberedAttributes.fill( false );
  }
  else
  {
    mRememberedAttributes.clear();
  }

  endResetModel();
  emit layerChanged();
}

void QgsQuickAttributeModel::setFeature( const QgsFeature &feature )
{
  if ( mFeatureLayerPair.feature() == feature )
    return;

  beginResetModel();
  mFeatureLayerPair = QgsQuickFeatureLayerPair( feature, mFeatureLayerPair.layer() );
  endResetModel();

  emit featureChanged();
}

QHash<int, QByteArray> QgsQuickAttributeModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[AttributeName]  = QByteArrayLiteral( "AttributeName" );
  roles[AttributeValue] = QByteArrayLiteral( "AttributeValue" );
  roles[Field] = QByteArrayLiteral( "Field" );
  roles[RememberAttribute] = QByteArrayLiteral( "RememberAttribute" );

  return roles;
}


int QgsQuickAttributeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeatureLayerPair.feature().attributes().count();
}

QVariant QgsQuickAttributeModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case AttributeName:
      return mFeatureLayerPair.layer()->attributeDisplayName( index.row() );
      break;

    case AttributeValue:
      return mFeatureLayerPair.feature().attribute( index.row() );
      break;

    case Field:
      return mFeatureLayerPair.layer()->fields().at( index.row() );
      break;

    case RememberAttribute:
      return mRememberedAttributes.at( index.row() );
      break;
  }

  return QVariant();
}

bool QgsQuickAttributeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( data( index, role ) == value )
    return true;

  switch ( role )
  {
    case AttributeValue:
    {
      QVariant val( value );
      QgsField fld = mFeatureLayerPair.feature().fields().at( index.row() );

      if ( !fld.convertCompatible( val ) )
      {
        QgsMessageLog::logMessage( tr( "Value \"%1\" %4 could not be converted to a compatible value for field %2(%3)." ).arg( value.toString(), fld.name(), fld.typeName(), value.isNull() ? "NULL" : "NOT NULL" ) );
        return false;
      }
      bool success = mFeatureLayerPair.featureRef().setAttribute( index.row(), val );
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

bool QgsQuickAttributeModel::save()
{
  if ( !mFeatureLayerPair.layer() )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  QgsFeature feat = mFeatureLayerPair.feature();
  if ( !mFeatureLayerPair.layer()->updateFeature( feat ) )
    QgsMessageLog::logMessage( tr( "Cannot update feature" ),
                               QStringLiteral( "QgsQuick" ),
                               Qgis::Warning );

  // This calls lower-level I/O functions which shouldn't be used
  // in a Q_INVOKABLE because they can make the UI unresponsive.
  rv = commit();

  if ( rv )
  {
    QgsFeature feat;
    if ( mFeatureLayerPair.layer()->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureLayerPair.feature().id() ) ).nextFeature( feat ) )
      setFeature( feat );
    else
      QgsMessageLog::logMessage( tr( "Feature %1 could not be fetched after commit" ).arg( mFeatureLayerPair.feature().id() ),
                                 QStringLiteral( "QgsQuick" ),
                                 Qgis::Warning );
  }
  return rv;
}

bool QgsQuickAttributeModel::deleteFeature()
{
  if ( !mFeatureLayerPair.layer() )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  if ( !mFeatureLayerPair.layer()->deleteFeature( mFeatureLayerPair.feature().id() ) )
    QgsMessageLog::logMessage( tr( "Cannot delete feature" ),
                               QStringLiteral( "QgsQuick" ),
                               Qgis::Warning );

  rv = commit();

  return rv;
}

void QgsQuickAttributeModel::reset()
{
  if ( !mFeatureLayerPair.layer() )
    return;

  mFeatureLayerPair.layer()->rollBack();
}

bool QgsQuickAttributeModel::suppressFeatureForm() const
{
  if ( !mFeatureLayerPair.layer() )
    return false;

  return mFeatureLayerPair.layer()->editFormConfig().suppress();
}

void QgsQuickAttributeModel::resetAttributes()
{
  if ( !mFeatureLayerPair.layer() )
    return;

  QgsExpressionContext expressionContext = mFeatureLayerPair.layer()->createExpressionContext();
  expressionContext.setFeature( mFeatureLayerPair.feature() );

  QgsFields fields = mFeatureLayerPair.layer()->fields();

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
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has parser error: %3" ).arg(
                                       mFeatureLayerPair.layer()->name(),
                                       fields.at( i ).name(),
                                       exp.parserErrorString() ),
                                     QStringLiteral( "QgsQuick" ),
                                     Qgis::Warning );

        QVariant value = exp.evaluate( &expressionContext );

        if ( exp.hasEvalError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has evaluation error: %3" ).arg(
                                       mFeatureLayerPair.layer()->name(),
                                       fields.at( i ).name(),
                                       exp.evalErrorString() ),
                                     QStringLiteral( "QgsQuick" ),
                                     Qgis::Warning );

        mFeatureLayerPair.feature().setAttribute( i, value );
      }
      else
      {
        mFeatureLayerPair.feature().setAttribute( i, QVariant() );
      }
    }
  }
  endResetModel();
}

void QgsQuickAttributeModel::create()
{
  if ( !mFeatureLayerPair.layer() )
    return;

  startEditing();
  QgsFeature feat = mFeatureLayerPair.feature();
  if ( !mFeatureLayerPair.layer()->addFeature( feat ) )
  {
    QgsMessageLog::logMessage( tr( "Feature could not be added" ),
                               QStringLiteral( "QgsQuick" ),
                               Qgis::Critical );
  }
  commit();
}

bool QgsQuickAttributeModel::commit()
{
  if ( !mFeatureLayerPair.layer()->commitChanges() )
  {
    QgsMessageLog::logMessage( tr( "Could not save changes. Rolling back." ),
                               QStringLiteral( "QgsQuick" ),
                               Qgis::Critical );
    mFeatureLayerPair.layer()->rollBack();
    return false;
  }
  else
  {
    return true;
  }
}

bool QgsQuickAttributeModel::startEditing()
{
  // Already an edit session active
  if ( mFeatureLayerPair.layer()->editBuffer() )
    return true;

  if ( !mFeatureLayerPair.layer()->startEditing() )
  {
    QgsMessageLog::logMessage( tr( "Cannot start editing" ),
                               QStringLiteral( "QgsQuick" ),
                               Qgis::Warning );
    return false;
  }
  else
  {
    return true;
  }
}

QVector<bool> QgsQuickAttributeModel::rememberedAttributes() const
{
  return mRememberedAttributes;
}
