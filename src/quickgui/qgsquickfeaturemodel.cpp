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

void QgsQuickFeatureModel::setFeature( const QgsFeature &feature )
{
  if ( mFeature == feature )
    return;

  beginResetModel();
  mFeature = feature;
  endResetModel();
  emit featureChanged();
}

void QgsQuickFeatureModel::setLayer( QgsVectorLayer *layer )
{
  if ( layer == mLayer )
    return;

  mLayer = layer;
  if ( mLayer )
  {
    mFeature = QgsFeature( mLayer->fields() );

    mRememberedAttributes.resize( layer->fields().size() );
    mRememberedAttributes.fill( false );
  }

  emit layerChanged();
}

QgsVectorLayer *QgsQuickFeatureModel::layer() const
{
  return mLayer;
}

QgsFeature QgsQuickFeatureModel::feature() const
{
  return mFeature;
}

QHash<int, QByteArray> QgsQuickFeatureModel::roleNames() const
{
  QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
  roles[AttributeName]  = "AttributeName";
  roles[AttributeValue] = "AttributeValue";
  roles[Field] = "Field";
  roles[RememberAttribute] = "RememberAttribute";

  return roles;
}


int QgsQuickFeatureModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  else
    return mFeature.attributes().count();
}

QVariant QgsQuickFeatureModel::data( const QModelIndex &index, int role ) const
{
  if ( mLayer )
    qWarning() << "Get data " << mLayer->name();

  switch ( role )
  {
    case AttributeName:
      return mLayer->attributeDisplayName( index.row() );
      break;

    case AttributeValue:
      return mFeature.attribute( index.row() );
      break;

    case Field:
      return mLayer->fields().at( index.row() );
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
      QgsField fld = mFeature.fields().at( index.row() );

      if ( !fld.convertCompatible( val ) )
      {
        QgsMessageLog::logMessage( tr( "Value \"%1\" %4 could not be converted to a compatible value for field %2(%3)." ).arg( value.toString(), fld.name(), fld.typeName(), value.isNull() ? "NULL" : "NOT NULL" ) );
        return false;
      }
      bool success = mFeature.setAttribute( index.row(), val );
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
  if ( !mLayer )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  QgsFeature feat = mFeature;
  if ( !mLayer->updateFeature( feat ) )
    QgsMessageLog::logMessage( tr( "Cannot update feature" ), "QgsQuick", Qgis::Warning );
  rv = commit();

  if ( rv )
  {
    QgsFeature feat;
    if ( mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.id() ) ).nextFeature( feat ) )
      setFeature( feat );
    else
      QgsMessageLog::logMessage( tr( "Feature %1 could not be fetched after commit" ).arg( mFeature.id() ), "QgsQuick", Qgis::Warning );
  }
  return rv;
}

bool QgsQuickFeatureModel::deleteFeature()
{
  if ( !mLayer )
    return false;

  bool rv = true;

  if ( !startEditing() )
  {
    rv = false;
  }

  if ( !mLayer->deleteFeature( mFeature.id() ) )
    QgsMessageLog::logMessage( tr( "Cannot delete feature" ), "QgsQuick", Qgis::Warning );
  rv = commit();

  return rv;
}

void QgsQuickFeatureModel::reset()
{
  if ( !mLayer )
    return;

  mLayer->rollBack();
}

bool QgsQuickFeatureModel::suppressFeatureForm() const
{
  if ( !mLayer )
    return false;

  return mLayer->editFormConfig().suppress();
}

void QgsQuickFeatureModel::resetAttributes()
{
  if ( !mLayer )
    return;

  QgsExpressionContext expressionContext = mLayer->createExpressionContext();
  expressionContext.setFeature( mFeature );

  QgsFields fields = mLayer->fields();

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
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has parser error: %3" ).arg( mLayer->name(), fields.at( i ).name(), exp.parserErrorString() ), QStringLiteral( "QgsQuick" ), Qgis::Warning );

        QVariant value = exp.evaluate( &expressionContext );

        if ( exp.hasEvalError() )
          QgsMessageLog::logMessage( tr( "Default value expression for %1:%2 has evaluation error: %3" ).arg( mLayer->name(), fields.at( i ).name(), exp.evalErrorString() ), QStringLiteral( "QgsQuick" ), Qgis::Warning );

        mFeature.setAttribute( i, value );
      }
      else
      {
        mFeature.setAttribute( i, QVariant() );
      }
    }
  }
  endResetModel();
}

void QgsQuickFeatureModel::create()
{
  if ( !mLayer )
    return;

  startEditing();
  if ( !mLayer->addFeature( mFeature ) )
  {
    QgsMessageLog::logMessage( tr( "Feature could not be added" ), "QgsQuick", Qgis::Critical );
  }
  commit();
}

bool QgsQuickFeatureModel::commit()
{
  if ( !mLayer->commitChanges() )
  {
    QgsMessageLog::logMessage( tr( "Could not save changes. Rolling back." ), "QgsQuick", Qgis::Critical );
    mLayer->rollBack();
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
  if ( mLayer->editBuffer() )
    return true;

  if ( !mLayer->startEditing() )
  {
    QgsMessageLog::logMessage( tr( "Cannot start editing" ), "QgsQuick", Qgis::Warning );
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
