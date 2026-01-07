/***************************************************************************
    qgsattributesformtreeviewindicatorprovider.cpp
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributesformtreeviewindicatorprovider.h"

#include "qgsapplication.h"
#include "qgsfieldconstraints.h"

#include "moc_qgsattributesformtreeviewindicatorprovider.cpp"

QgsAttributesFormTreeViewIndicatorProvider::QgsAttributesFormTreeViewIndicatorProvider( QgsAttributesFormBaseView *view )
  : QObject( view )
  , mAttributesFormTreeView( view )
{
}

void QgsAttributesFormTreeViewIndicatorProvider::onAddedChildren( QgsAttributesFormItem *item, int indexFrom, int indexTo )
{
  // recursively populate indicators
  for ( int i = indexFrom; i <= indexTo; ++i )
  {
    QgsAttributesFormItem *childItem = item->child( i );

    if ( QgsAttributesFormItem::isGroup( childItem ) )
    {
      if ( childItem->childCount() > 0 )
      {
        onAddedChildren( childItem, 0, childItem->childCount() - 1 );
      }
    }
    else
    {
      updateItemIndicator( childItem );
    }
  }
}

std::unique_ptr<QgsAttributesFormTreeViewIndicator> QgsAttributesFormTreeViewIndicatorProvider::newIndicator( QgsAttributesFormItem *item )
{
  auto indicator = std::make_unique<QgsAttributesFormTreeViewIndicator>( this );
  indicator->setIcon( QgsApplication::getThemeIcon( iconName( item ) ) );
  indicator->setToolTip( tooltipText( item ) );
  mIndicators.insert( indicator.get() );

  return indicator;
}

void QgsAttributesFormTreeViewIndicatorProvider::updateItemIndicator( QgsAttributesFormItem *item )
{
  if ( acceptsItem( item ) )
  {
    const QList<QgsAttributesFormTreeViewIndicator *> itemIndicators = mAttributesFormTreeView->indicators( item );

    // maybe the indicator exists already
    for ( QgsAttributesFormTreeViewIndicator *indicator : itemIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        indicator->setToolTip( tooltipText( item ) );
        indicator->setIcon( QgsApplication::getThemeIcon( iconName( item ) ) );
        return;
      }
    }

    // it does not exist: need to create a new one
    mAttributesFormTreeView->addIndicator( item, newIndicator( item ).release() );
  }
  else
  {
    removeItemIndicator( item );
  }
}

void QgsAttributesFormTreeViewIndicatorProvider::removeItemIndicator( QgsAttributesFormItem *item )
{
  const QList<QgsAttributesFormTreeViewIndicator *> itemIndicators = mAttributesFormTreeView->indicators( item );

  // Get rid of the existing indicator
  for ( QgsAttributesFormTreeViewIndicator *indicator : itemIndicators )
  {
    if ( mIndicators.contains( indicator ) )
    {
      mIndicators.remove( indicator );
      mAttributesFormTreeView->removeIndicator( item, indicator );
      indicator->deleteLater();
      return;
    }
  }
}

bool QgsAttributesFormTreeViewIndicatorProvider::isEnabled() const
{
  return mEnabled;
}

void QgsAttributesFormTreeViewIndicatorProvider::setEnabled( bool enabled )
{
  QgsAttributesFormItem *item = mAttributesFormTreeView->sourceModel()->rootItem();

  if ( enabled )
  {
    if ( mEnabled )
    {
      return; // Already done
    }

    // Draw indicators for all existing items
    if ( item->childCount() > 0 )
    {
      onAddedChildren( item, 0, item->childCount() - 1 );
    }

    // Connect
    connect( item, &QgsAttributesFormItem::addedChildren, this, &QgsAttributesFormTreeViewIndicatorProvider::onAddedChildren );
    mEnabled = true;
  }
  else
  {
    if ( !mEnabled )
    {
      return; // Already done
    }

    // Disconnect
    disconnect( item, &QgsAttributesFormItem::addedChildren, this, &QgsAttributesFormTreeViewIndicatorProvider::onAddedChildren );

    // Get rid of all item indicators in the view and in the provider
    mAttributesFormTreeView->removeAllIndicators();
    mIndicators.clear();
    mEnabled = false;
  }
}


QgsFieldConstraintIndicatorProvider::QgsFieldConstraintIndicatorProvider( QgsAttributesFormBaseView *view )
  : QgsAttributesFormTreeViewIndicatorProvider( view )
{
}

bool QgsFieldConstraintIndicatorProvider::acceptsItem( QgsAttributesFormItem *item )
{
  if ( item->type() == QgsAttributesFormData::Field )
  {
    const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
    const QgsFieldConstraints constraints = config.mFieldConstraints;

    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) != QgsFieldConstraints::ConstraintOriginNotSet )
      return true;

    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) != QgsFieldConstraints::ConstraintOriginNotSet )
      return true;

    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) != QgsFieldConstraints::ConstraintOriginNotSet )
      return true;
  }
  return false;
}

QString QgsFieldConstraintIndicatorProvider::iconName( QgsAttributesFormItem *item )
{
  const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
  const QgsFieldConstraints constraints = config.mFieldConstraints;

  bool hardConstraint = false;

  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    if ( constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintStrengthHard )
    {
      hardConstraint = true;
    }
  }

  if ( !hardConstraint )
  {
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) != QgsFieldConstraints::ConstraintOriginNotSet )
    {
      if ( constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintStrengthHard )
      {
        hardConstraint = true;
      }
    }
  }

  if ( !hardConstraint )
  {
    if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) != QgsFieldConstraints::ConstraintOriginNotSet )
    {
      if ( constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintStrengthHard )
      {
        hardConstraint = true;
      }
    }
  }

  return hardConstraint ? u"/field_indicators/mIndicatorConstraintHard.svg"_s : u"/field_indicators/mIndicatorConstraint.svg"_s;
}

QString QgsFieldConstraintIndicatorProvider::tooltipText( QgsAttributesFormItem *item )
{
  const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
  const QgsFieldConstraints constraints = config.mFieldConstraints;

  auto addOriginAndStrengthText = [=]( QString name, QgsFieldConstraints::Constraint constraint ) {
    return u"%1 (%2, %3)"_s.arg( name, constraints.constraintOrigin( constraint ) == QgsFieldConstraints::ConstraintOriginProvider ? tr( "provider" ) : tr( "layer" ), constraints.constraintStrength( constraint ) == QgsFieldConstraints::ConstraintStrengthHard ? tr( "enforced" ) : tr( "unenforced" ) );
  };

  QString tooltipText;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += addOriginAndStrengthText( tr( "Not Null" ), QgsFieldConstraints::ConstraintNotNull );
  }

  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += "\n" + addOriginAndStrengthText( tr( "Unique" ), QgsFieldConstraints::ConstraintUnique );
  }

  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += "\n" + addOriginAndStrengthText( tr( "Expression" ), QgsFieldConstraints::ConstraintExpression );
    tooltipText += !constraints.constraintDescription().isEmpty() ? u"\n   "_s + constraints.constraintDescription() : QString();
  }

  return tooltipText;
}


QgsFieldDefaultValueIndicatorProvider::QgsFieldDefaultValueIndicatorProvider( QgsAttributesFormBaseView *view )
  : QgsAttributesFormTreeViewIndicatorProvider( view )
{
}

bool QgsFieldDefaultValueIndicatorProvider::acceptsItem( QgsAttributesFormItem *item )
{
  if ( item->type() == QgsAttributesFormData::Field )
  {
    const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
    return !config.mDefaultValueExpression.isEmpty();
  }
  return false;
}

QString QgsFieldDefaultValueIndicatorProvider::iconName( QgsAttributesFormItem *item )
{
  const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();

  QString iconName = u"/field_indicators/mIndicatorDefaultValue.svg"_s;

  if ( !config.mDefaultValueExpression.isEmpty() )
  {
    if ( config.mApplyDefaultValueOnUpdate )
    {
      iconName = u"/field_indicators/mIndicatorDefaultValueApplyOnUpdate.svg"_s;
    }
  }
  return iconName;
}

QString QgsFieldDefaultValueIndicatorProvider::tooltipText( QgsAttributesFormItem *item )
{
  const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
  QString text;
  if ( !config.mDefaultValueExpression.isEmpty() )
  {
    text += config.mDefaultValueExpression;
    text += u"\n(%1)"_s.arg( config.mApplyDefaultValueOnUpdate ? tr( "Apply on update" ) : tr( "Do not apply on update" ) );
  }
  return text;
}
