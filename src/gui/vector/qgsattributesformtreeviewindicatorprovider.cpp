#include "qgsapplication.h"
#include "qgsattributesformtreeviewindicatorprovider.h"
#include "qgsfieldconstraints.h"


QgsAttributesFormTreeViewIndicatorProvider::QgsAttributesFormTreeViewIndicatorProvider( QgsAttributesFormBaseView *view )
  : QObject( view )
  , mAttributesFormTreeView( view )
{
  QgsAttributesFormItem *item = mAttributesFormTreeView->sourceModel()->rootItem();
  if ( item->childCount() > 0 )
  {
    onAddedChildren( item, 0, item->childCount() - 1 );
  }

  connect( item, &QgsAttributesFormItem::addedChildren, this, &QgsAttributesFormTreeViewIndicatorProvider::onAddedChildren );
  //connect( item, &QgsAttributesFormItem::willRemoveChildren, this, &QgsAttributesFormTreeViewIndicatorProvider::onWillRemoveChildren );
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
    else // if ( QgsLayerTree::isLayer( childNode ) )
    {
      //connectSignals( layerNode->layer() );
      updateItemIndicator( childItem );
    }
  }
}

std::unique_ptr<QgsAttributesFormTreeViewIndicator> QgsAttributesFormTreeViewIndicatorProvider::newIndicator( QgsAttributesFormItem *item )
{
  auto indicator = std::make_unique<QgsAttributesFormTreeViewIndicator>( this );
  indicator->setIcon( QgsApplication::getThemeIcon( iconName( item ) ) );
  indicator->setToolTip( tooltipText( item ) );
  //connect( indicator.get(), &QgsLayerTreeViewIndicator::clicked, this, &QgsLayerTreeViewIndicatorProvider::onIndicatorClicked );
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
        // Update just in case ...
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
    const QList<QgsAttributesFormTreeViewIndicator *> itemIndicators = mAttributesFormTreeView->indicators( item );

    // there may be existing indicator we need to get rid of
    for ( QgsAttributesFormTreeViewIndicator *indicator : itemIndicators )
    {
      if ( mIndicators.contains( indicator ) )
      {
        mAttributesFormTreeView->removeIndicator( item, indicator );
        indicator->deleteLater();
        return;
      }
    }

    // no indicator was there before, nothing to do
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

QString QgsFieldConstraintIndicatorProvider::iconName( QgsAttributesFormItem * )
{
  return QStringLiteral( "/mIndicatorEmbedded.svg" );
}

QString QgsFieldConstraintIndicatorProvider::tooltipText( QgsAttributesFormItem *item )
{
  const QgsAttributesFormData::FieldConfig config = item->data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
  const QgsFieldConstraints constraints = config.mFieldConstraints;

  auto addOriginAndStrengthText = [=]( QgsFieldConstraints::Constraint constraint ) {
    QString text;
    if ( constraints.constraintOrigin( constraint ) == QgsFieldConstraints::ConstraintOriginProvider )
    {
      text += tr( "provider, " );
    }
    else
    {
      text += tr( "layer, " );
    }

    if ( constraints.constraintStrength( constraint ) == QgsFieldConstraints::ConstraintStrengthHard )
    {
      text += tr( "enforced)" );
    }
    else
    {
      text += tr( "unenforced)" );
    }
    return text;
  };

  QString tooltipText;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += tr( "Not Null (" );
    tooltipText += addOriginAndStrengthText( QgsFieldConstraints::ConstraintNotNull );
  }

  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += tr( "\nUnique (" );
    tooltipText += addOriginAndStrengthText( QgsFieldConstraints::ConstraintUnique );
  }

  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) != QgsFieldConstraints::ConstraintOriginNotSet )
  {
    tooltipText += tr( "\nExpression (" );
    tooltipText += addOriginAndStrengthText( QgsFieldConstraints::ConstraintExpression );
    tooltipText += !constraints.constraintDescription().isEmpty() ? QStringLiteral( "\n   " ) + constraints.constraintDescription() : QString();
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
    return true;
  }
  return false;
}

QString QgsFieldDefaultValueIndicatorProvider::iconName( QgsAttributesFormItem * )
{
  return QStringLiteral( "/mIndicatorNotes.svg" );
}

QString QgsFieldDefaultValueIndicatorProvider::tooltipText( QgsAttributesFormItem * )
{
  return QStringLiteral( "Default value" );
}
