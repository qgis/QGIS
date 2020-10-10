/***************************************************************************
  qgsquickfeatureslistmodel.cpp
 ---------------------------
  Date                 : Sep 2020
  Copyright            : (C) 2020 by Tomas Mizera
  Email                : tomas.mizera2 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickfeatureslistmodel.h"
#include "qgsexpressioncontextutils.h"
#include "qgslogger.h"

QgsQuickFeaturesListModel::QgsQuickFeaturesListModel( QObject *parent )
  : QAbstractListModel( parent ),
    mCurrentLayer( nullptr )
{
}

int QgsQuickFeaturesListModel::rowCount( const QModelIndex &parent ) const
{
  // For list models only the root node (an invalid parent) should return the list's size. For all
  // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
  if ( parent.isValid() )
    return 0;

  return mFeatures.count();
}

QVariant QgsQuickFeaturesListModel::featureTitle( const QgsQuickFeatureLayerPair &featurePair ) const
{
  QString title;

  if ( !mFeatureTitleField.isEmpty() )
  {
    title = featurePair.feature().attribute( mFeatureTitleField ).toString();
    if ( !title.isEmpty() )
      return title;
  }

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( featurePair.layer() ) );
  context.setFeature( featurePair.feature() );
  QgsExpression expr( featurePair.layer()->displayExpression() );
  title = expr.evaluate( &context ).toString();

  if ( title.isEmpty() )
    return featurePair.feature().id();

  return title;
}

QVariant QgsQuickFeaturesListModel::data( const QModelIndex &index, int role ) const
{
  int row = index.row();
  if ( row < 0 || row >= mFeatures.count() )
    return QVariant();

  if ( !index.isValid() )
    return QVariant();

  const QgsQuickFeatureLayerPair pair = mFeatures.at( index.row() );

  switch ( role )
  {
    case FeatureTitle: return featureTitle( pair );
    case FeatureId: return QVariant( pair.feature().id() );
    case Feature: return QVariant::fromValue<QgsFeature>( pair.feature() );
    case Description: return QVariant( QString( "Feature ID %1" ).arg( pair.feature().id() ) );
    case KeyColumn: return mKeyField.isEmpty() ? QVariant() : pair.feature().attribute( mKeyField );
    case FoundPair: return foundPair( pair );
    case Qt::DisplayRole: return featureTitle( pair );
  }

  return QVariant();
}

QString QgsQuickFeaturesListModel::foundPair( const QgsQuickFeatureLayerPair &pair ) const
{
  if ( mFilterExpression.isEmpty() )
    return QString();

  QgsFields fields = pair.feature().fields();

  for ( const QgsField &field : fields )
  {
    QString attrValue = pair.feature().attribute( field.name() ).toString();

    if ( attrValue.toLower().indexOf( mFilterExpression.toLower() ) != -1 )
      return field.name() + ": " + attrValue;
  }
  return QString();
}

QString QgsQuickFeaturesListModel::buildFilterExpression()
{
  if ( mFilterExpression.isEmpty() || !mCurrentLayer )
    return QString();

  const QgsFields fields = mCurrentLayer->fields();
  QStringList expressionParts;

  bool filterExpressionIsNumeric;
  int filterInt = mFilterExpression.toInt( &filterExpressionIsNumeric );
  Q_UNUSED( filterInt ); // we only need to know if expression is numeric, int value is not used

  for ( const QgsField &field : fields )
  {
    if ( field.isNumeric() && filterExpressionIsNumeric )
      expressionParts << QStringLiteral( "%1 ~ '%2.*'" ).arg( QgsExpression::quotedColumnRef( field.name() ), mFilterExpression );
    else if ( field.type() == QVariant::String )
      expressionParts << QStringLiteral( "%1 ILIKE '%%2%'" ).arg( QgsExpression::quotedColumnRef( field.name() ), mFilterExpression );
  }

  QString expression = QStringLiteral( "(%1)" ).arg( expressionParts.join( QLatin1String( " ) OR ( " ) ) );

  return expression;
}

void QgsQuickFeaturesListModel::loadFeaturesFromLayer( QgsVectorLayer *layer )
{
  if ( layer && layer->isValid() )
    mCurrentLayer = layer;

  if ( mCurrentLayer )
  {
    beginResetModel();

    mFeatures.clear();
    QgsFeatureRequest req;
    if ( !mFilterExpression.isEmpty() )
      req.setFilterExpression( buildFilterExpression() );
    req.setLimit( FEATURES_LIMIT );

    QgsFeatureIterator it = mCurrentLayer->getFeatures( req );
    QgsFeature f;

    while ( it.nextFeature( f ) )
    {
      mFeatures << QgsQuickFeatureLayerPair( f, mCurrentLayer );
    }
    emit featuresCountChanged( featuresCount() );
    endResetModel();
  }
}

void QgsQuickFeaturesListModel::setupValueRelation( const QVariantMap &config )
{
  beginResetModel();
  emptyData();

  QgsVectorLayer *layer = QgsValueRelationFieldFormatter::resolveLayer( config, QgsProject::instance() );

  if ( layer )
  {
    // save key and value field
    QgsFields fields = layer->fields();

    setKeyField( fields.field( config.value( QStringLiteral( "Key" ) ).toString() ).name() );
    setFeatureTitleField( fields.field( config.value( QStringLiteral( "Value" ) ).toString() ).name() );

    loadFeaturesFromLayer( layer );
  }

  endResetModel();
}

void QgsQuickFeaturesListModel::populateFromLayer( QgsVectorLayer *layer )
{
  beginResetModel();
  emptyData();

  loadFeaturesFromLayer( layer );
  endResetModel();
}

void QgsQuickFeaturesListModel::reloadFeatures()
{
  loadFeaturesFromLayer();
}

void QgsQuickFeaturesListModel::emptyData()
{
  mFeatures.clear();
  mCurrentLayer = nullptr;
  mKeyField.clear();
  mFeatureTitleField.clear();
  mFilterExpression.clear();
}

QHash<int, QByteArray> QgsQuickFeaturesListModel::roleNames() const
{
  QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
  roleNames[FeatureTitle] = QStringLiteral( "FeatureTitle" ).toLatin1();
  roleNames[FeatureId] = QStringLiteral( "FeatureId" ).toLatin1();
  roleNames[Feature] = QStringLiteral( "Feature" ).toLatin1();
  roleNames[Description] = QStringLiteral( "Description" ).toLatin1();
  roleNames[FoundPair] = QStringLiteral( "FoundPair" ).toLatin1();
  roleNames[KeyColumn] = QStringLiteral( "KeyColumn" ).toLatin1();
  return roleNames;
}

int QgsQuickFeaturesListModel::featuresCount() const
{
  if ( mCurrentLayer )
    return mCurrentLayer->featureCount();
  return 0;
}

QString QgsQuickFeaturesListModel::filterExpression() const
{
  return mFilterExpression;
}

void QgsQuickFeaturesListModel::setFilterExpression( const QString &filterExpression )
{
  mFilterExpression = filterExpression;
  emit filterExpressionChanged( mFilterExpression );

  loadFeaturesFromLayer();
}

void QgsQuickFeaturesListModel::setFeatureTitleField( const QString &attribute )
{
  mFeatureTitleField = attribute;
}

void QgsQuickFeaturesListModel::setKeyField( const QString &attribute )
{
  mKeyField = attribute;
}

int QgsQuickFeaturesListModel::featuresLimit() const
{
  return FEATURES_LIMIT;
}

int QgsQuickFeaturesListModel::rowFromAttribute( const int role, const QVariant &value ) const
{
  for ( int i = 0; i < mFeatures.count(); ++i )
  {
    QVariant d = data( index( i, 0 ), role );
    if ( d == value )
    {
      return i;
    }
  }
  return -1;
}

int QgsQuickFeaturesListModel::keyFromAttribute( const int role, const QVariant &value ) const
{
  for ( int i = 0; i < mFeatures.count(); ++i )
  {
    QVariant d = data( index( i, 0 ), role );
    if ( d == value )
    {
      QVariant key = data( index( i, 0 ), KeyColumn );
      return key.toInt();
    }
  }
  return -1;
}

QgsQuickFeatureLayerPair QgsQuickFeaturesListModel::featureLayerPair( const int &featureId )
{
  for ( const QgsQuickFeatureLayerPair &i : mFeatures )
  {
    if ( i.feature().id() == featureId )
      return i;
  }
  return QgsQuickFeatureLayerPair();
}
