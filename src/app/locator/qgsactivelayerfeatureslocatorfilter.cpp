/***************************************************************************
                        qgsactivelayerfeatureslocatorfilters.cpp
                        ----------------------------
   begin                : May 2017
   copyright            : (C) 2017 by Nyall Dawson
   email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactivelayerfeatureslocatorfilter.h"

#include "qgisapp.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeatureaction.h"
#include "qgsiconutils.h"
#include "qgslocatorwidget.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"

#include <QSpinBox>

#include "moc_qgsactivelayerfeatureslocatorfilter.cpp"

QgsActiveLayerFeaturesLocatorFilter::QgsActiveLayerFeaturesLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsActiveLayerFeaturesLocatorFilter *QgsActiveLayerFeaturesLocatorFilter::clone() const
{
  return new QgsActiveLayerFeaturesLocatorFilter();
}

QString QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( QString &searchString, bool *isRestricting )
{
  QString _fieldRestriction;
  searchString = searchString.trimmed();
  if ( isRestricting )
    *isRestricting = searchString.startsWith( '@' );
  if ( searchString.startsWith( '@' ) )
  {
    _fieldRestriction = searchString.left( std::min( searchString.indexOf( ' ' ), searchString.length() ) ).remove( 0, 1 );
    searchString = searchString.mid( _fieldRestriction.length() + 2 );
  }
  return _fieldRestriction;
}

QStringList QgsActiveLayerFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext &context )
{
  mFieldsCompletion.clear();

  // Normally skip very short search strings, unless when specifically searching using this filter or try to match fields
  if ( string.length() < 3 && !context.usingPrefix && !string.startsWith( '@' ) )
    return QStringList();

  QgsSettings settings;
  mMaxTotalResults = settings.value( u"locator_filters/active_layer_features/limit_global"_s, 30, QgsSettings::App ).toInt();

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( !layer )
    return QStringList();

  if ( !layer->flags().testFlag( QgsMapLayer::Searchable ) )
    return QStringList();

  mLayerIsSpatial = layer->isSpatial();
  mDispExpression = QgsExpression( layer->displayExpression() );
  mContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  mDispExpression.prepare( &mContext );

  // determine if search is restricted to a specific field
  QString searchString = string;
  bool isRestricting = false;
  QString _fieldRestriction = fieldRestriction( searchString, &isRestricting );
  bool allowNumeric = false;
  double numericalValue = searchString.toDouble( &allowNumeric );

  // search in display expression if no field restriction
  if ( !isRestricting )
  {
    QgsFeatureRequest req;
    req.setSubsetOfAttributes( qgis::setToList( mDispExpression.referencedAttributeIndexes( layer->fields() ) ) );
    if ( !mDispExpression.needsGeometry() )
      req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    QString enhancedSearch = searchString;
    enhancedSearch.replace( ' ', '%' );
    req.setFilterExpression( u"%1 ILIKE '%%2%'"_s
                               .arg( layer->displayExpression(), enhancedSearch ) );
    req.setLimit( mMaxTotalResults );
    mDisplayTitleIterator = layer->getFeatures( req );
  }
  else
  {
    mDisplayTitleIterator = QgsFeatureIterator();
  }

  // build up request expression
  QStringList expressionParts;
  QStringList completionList;
  const QgsFields fields = layer->fields();
  QgsAttributeList subsetOfAttributes = qgis::setToList( mDispExpression.referencedAttributeIndexes( layer->fields() ) );
  for ( const QgsField &field : fields )
  {
    if ( field.configurationFlags().testFlag( Qgis::FieldConfigurationFlag::NotSearchable ) )
      continue;

    if ( isRestricting && !field.name().startsWith( _fieldRestriction ) )
      continue;

    if ( isRestricting )
    {
      int index = layer->fields().indexFromName( field.name() );
      if ( !subsetOfAttributes.contains( index ) )
        subsetOfAttributes << index;
    }

    // if we are trying to find a field (and not searching anything yet)
    // keep the list of matching fields to display them as results
    if ( isRestricting && searchString.isEmpty() && _fieldRestriction != field.name() )
    {
      mFieldsCompletion << field.name();
    }

    // the completion list (returned by the current method) is used by the locator line edit directly
    completionList.append( u"@%1 "_s.arg( field.name() ) );

    if ( field.type() == QMetaType::Type::QString )
    {
      expressionParts << u"%1 ILIKE '%%2%'"_s.arg( QgsExpression::quotedColumnRef( field.name() ), searchString );
    }
    else if ( allowNumeric && field.isNumeric() )
    {
      expressionParts << u"%1 = %2"_s.arg( QgsExpression::quotedColumnRef( field.name() ), QString::number( numericalValue, 'g', 17 ) );
    }
  }

  QString expression = u"(%1)"_s.arg( expressionParts.join( " ) OR ( "_L1 ) );

  QgsFeatureRequest req;
  if ( !mDispExpression.needsGeometry() )
    req.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  req.setFilterExpression( expression );
  if ( isRestricting )
    req.setSubsetOfAttributes( subsetOfAttributes );

  req.setLimit( mMaxTotalResults );
  mFieldIterator = layer->getFeatures( req );

  mLayerId = layer->id();
  mLayerIcon = QgsIconUtils::iconForLayer( layer );
  mAttributeAliases.clear();
  for ( int idx = 0; idx < layer->fields().size(); ++idx )
  {
    mAttributeAliases.append( layer->attributeDisplayName( idx ) );
  }

  return completionList;
}

void QgsActiveLayerFeaturesLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  QgsFeatureIds featuresFound;
  QgsFeature f;
  QString searchString = string;
  fieldRestriction( searchString );

  // propose available fields for restriction
  for ( const QString &field : std::as_const( mFieldsCompletion ) )
  {
    QgsLocatorResult result;
    result.displayString = u"@%1"_s.arg( field );
    result.description = tr( "Limit the search to the field '%1'" ).arg( field );
    result.setUserData( QVariantMap( { { u"type"_s, QVariant::fromValue( ResultType::FieldRestriction ) }, { u"search_text"_s, u"%1 @%2 "_s.arg( prefix(), field ) } } ) );
    result.score = 1;
    emit resultFetched( result );
  }

  // search in display title
  if ( mDisplayTitleIterator.isValid() )
  {
    while ( mDisplayTitleIterator.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return;

      mContext.setFeature( f );

      QgsLocatorResult result;

      result.displayString = mDispExpression.evaluate( &mContext ).toString();
      result.setUserData( QVariantMap(
        { { u"type"_s, QVariant::fromValue( ResultType::Feature ) },
          { u"feature_id"_s, f.id() },
          { u"layer_id"_s, mLayerId },
          { u"layer_is_spatial"_s, mLayerIsSpatial }
        }
      ) );
      result.icon = mLayerIcon;
      result.score = static_cast<double>( searchString.length() ) / result.displayString.size();
      if ( mLayerIsSpatial )
        result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );

      emit resultFetched( result );

      featuresFound << f.id();

      if ( featuresFound.count() >= mMaxTotalResults )
        break;
    }
  }

  // search in fields
  while ( mFieldIterator.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return;

    // do not display twice the same feature
    if ( featuresFound.contains( f.id() ) )
      continue;

    QgsLocatorResult result;

    mContext.setFeature( f );

    // find matching field content
    int idx = 0;
    const QgsAttributes attributes = f.attributes();
    for ( const QVariant &var : attributes )
    {
      QString attrString = var.toString();
      if ( attrString.contains( searchString, Qt::CaseInsensitive ) )
      {
        if ( idx < mAttributeAliases.count() )
          result.displayString = u"%1 (%2)"_s.arg( attrString, mAttributeAliases[idx] );
        else
          result.displayString = attrString;
        break;
      }
      idx++;
    }
    if ( result.displayString.isEmpty() )
      continue; //not sure how this result slipped through...

    result.description = mDispExpression.evaluate( &mContext ).toString();
    result.setUserData( QVariantMap(
      { { u"type"_s, QVariant::fromValue( ResultType::Feature ) },
        { u"feature_id"_s, f.id() },
        { u"layer_id"_s, mLayerId },
        { u"layer_is_spatial"_s, mLayerIsSpatial }
      }
    ) );
    result.icon = mLayerIcon;
    result.score = static_cast<double>( searchString.length() ) / result.displayString.size();
    if ( mLayerIsSpatial )
      result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );

    emit resultFetched( result );

    featuresFound << f.id();
    if ( featuresFound.count() >= mMaxTotalResults )
      break;
  }
}


void QgsActiveLayerFeaturesLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  triggerResultFromAction( result, NoEntry );
}

void QgsActiveLayerFeaturesLocatorFilter::triggerResultFromAction( const QgsLocatorResult &result, const int actionId )
{
  QVariantMap data = result.userData().value<QVariantMap>();
  switch ( data.value( u"type"_s ).value<ResultType>() )
  {
    case ResultType::Feature:
    {
      QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( data.value( u"layer_id"_s ).toString() );
      if ( layer )
      {
        QgsFeatureId fid = data.value( u"feature_id"_s ).value<QgsFeatureId>();
        if ( actionId == OpenForm || !data.value( u"layer_is_spatial"_s, true ).toBool() )
        {
          QgsFeature f;
          QgsFeatureRequest request;
          request.setFilterFid( fid );
          bool fetched = layer->getFeatures( request ).nextFeature( f );
          if ( !fetched )
            return;
          QgsFeatureAction action( tr( "Attributes changed" ), f, layer, QUuid(), -1, QgisApp::instance() );
          if ( layer->isEditable() )
          {
            action.editFeature( false );
          }
          else
          {
            action.viewFeatureForm();
          }
        }
        else
        {
          QgisApp::instance()->mapCanvas()->zoomToFeatureIds( layer, QgsFeatureIds() << fid );
          QgisApp::instance()->mapCanvas()->flashFeatureIds( layer, QgsFeatureIds() << fid );
        }
      }
      break;
    }
    case ResultType::FieldRestriction:
    {
      // this is a field restriction
      QgisApp::instance()->locatorWidget()->search( data.value( u"search_text"_s ).toString() );
      break;
    }
  }
}

void QgsActiveLayerFeaturesLocatorFilter::openConfigWidget( QWidget *parent )
{
  QString key = "locator_filters/active_layer_features";
  QgsSettings settings;
  auto dlg = std::make_unique<QDialog>( parent );
  dlg->restoreGeometry( settings.value( u"Windows/%1/geometry"_s.arg( key ) ).toByteArray() );
  dlg->setWindowTitle( "All layers features locator filter" );
  QFormLayout *formLayout = new QFormLayout;
  QSpinBox *globalLimitSpinBox = new QSpinBox( dlg.get() );
  globalLimitSpinBox->setValue( settings.value( u"%1/limit_global"_s.arg( key ), 30, QgsSettings::App ).toInt() );
  globalLimitSpinBox->setMinimum( 1 );
  globalLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results:" ), globalLimitSpinBox );
  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );
  connect( buttonbBox, &QDialogButtonBox::accepted, dlg.get(), [&]() {
    settings.setValue( u"%1/limit_global"_s.arg( key ), globalLimitSpinBox->value(), QgsSettings::App );
    dlg->accept();
  } );
  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );
  dlg->exec();
}
