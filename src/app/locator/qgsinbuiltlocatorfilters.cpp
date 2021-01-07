/***************************************************************************
                        qgsinbuiltlocatorfilters.cpp
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

#include <QClipboard>
#include <QMap>
#include <QSpinBox>
#include <QString>
#include <QToolButton>
#include <QUrl>

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinateutils.h"
#include "qgsinbuiltlocatorfilters.h"
#include "qgsproject.h"
#include "qgslayertree.h"
#include "qgsfeedback.h"
#include "qgisapp.h"
#include "qgsmaplayermodel.h"
#include "qgslayoutmanager.h"
#include "qgsmapcanvas.h"
#include "qgsfeatureaction.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsexpressioncontextutils.h"
#include "qgssettings.h"
#include "qgsunittypes.h"
#include "qgslocatorwidget.h"


QgsLayerTreeLocatorFilter::QgsLayerTreeLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsLayerTreeLocatorFilter *QgsLayerTreeLocatorFilter::clone() const
{
  return new QgsLayerTreeLocatorFilter();
}

void QgsLayerTreeLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback * )
{
  QgsLayerTree *tree = QgsProject::instance()->layerTreeRoot();
  const QList<QgsLayerTreeLayer *> layers = tree->findLayers();
  for ( QgsLayerTreeLayer *layer : layers )
  {
    // if the layer is broken, don't include it in the results
    if ( ! layer->layer() )
      continue;

    QgsLocatorResult result;
    result.displayString = layer->layer()->name();
    result.userData = layer->layerId();
    result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );

    // return all the layers in case the string query is empty using an equal default score
    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

void QgsLayerTreeLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString layerId = result.userData.toString();
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( layerId );
  QgisApp::instance()->setActiveLayer( layer );
}

//
// QgsLayoutLocatorFilter
//

QgsLayoutLocatorFilter::QgsLayoutLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsLayoutLocatorFilter *QgsLayoutLocatorFilter::clone() const
{
  return new QgsLayoutLocatorFilter();
}

void QgsLayoutLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback * )
{
  const QList< QgsMasterLayoutInterface * > layouts = QgsProject::instance()->layoutManager()->layouts();
  for ( QgsMasterLayoutInterface *layout : layouts )
  {
    // if the layout is broken, don't include it in the results
    if ( ! layout )
      continue;

    QgsLocatorResult result;
    result.displayString = layout->name();
    result.userData = layout->name();

    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

void QgsLayoutLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString layoutName = result.userData.toString();
  QgsMasterLayoutInterface *layout = QgsProject::instance()->layoutManager()->layoutByName( layoutName );
  if ( !layout )
    return;

  QgisApp::instance()->openLayoutDesignerDialog( layout );
}

QgsActionLocatorFilter::QgsActionLocatorFilter( const QList<QWidget *> &parentObjectsForActions, QObject *parent )
  : QgsLocatorFilter( parent )
  , mActionParents( parentObjectsForActions )
{
  setUseWithoutPrefix( false );
}

QgsActionLocatorFilter *QgsActionLocatorFilter::clone() const
{
  return new QgsActionLocatorFilter( mActionParents );
}

void QgsActionLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback * )
{
  // collect results in main thread, since this method is inexpensive and
  // accessing the gui actions is not thread safe

  QList<QAction *> found;

  for ( QWidget *object : qgis::as_const( mActionParents ) )
  {
    searchActions( string, object, found );
  }
}

void QgsActionLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QAction *action = qobject_cast< QAction * >( qvariant_cast<QObject *>( result.userData ) );
  if ( action )
    action->trigger();
}

void QgsActionLocatorFilter::searchActions( const QString &string, QWidget *parent, QList<QAction *> &found )
{
  const QList< QWidget *> children = parent->findChildren<QWidget *>();
  for ( QWidget *widget : children )
  {
    searchActions( string, widget, found );
  }

  QRegularExpression extractFromTooltip( QStringLiteral( "<b>(.*)</b>" ) );
  QRegularExpression newLineToSpace( QStringLiteral( "[\\s\\n\\r]+" ) );

  const auto constActions = parent->actions();
  for ( QAction *action : constActions )
  {
    if ( action->menu() )
    {
      searchActions( string, action->menu(), found );
      continue;
    }

    if ( !action->isEnabled() || !action->isVisible() || action->text().isEmpty() )
      continue;
    if ( found.contains( action ) )
      continue;

    QString searchText = action->text();
    searchText.replace( '&', QString() );

    QString tooltip = action->toolTip();
    tooltip.replace( newLineToSpace, QStringLiteral( " " ) );
    QRegularExpressionMatch match = extractFromTooltip.match( tooltip );
    if ( match.hasMatch() )
    {
      tooltip = match.captured( 1 );
    }
    tooltip.replace( QLatin1String( "..." ), QString() );
    tooltip.replace( QString( QChar( 0x2026 ) ), QString() );
    searchText.replace( QLatin1String( "..." ), QString() );
    searchText.replace( QString( QChar( 0x2026 ) ), QString() );
    bool uniqueTooltip = searchText.trimmed().compare( tooltip.trimmed(), Qt::CaseInsensitive ) != 0;
    if ( action->isChecked() )
    {
      searchText += QStringLiteral( " [%1]" ).arg( tr( "Active" ) );
    }
    if ( uniqueTooltip )
    {
      searchText += QStringLiteral( " (%1)" ).arg( tooltip.trimmed() );
    }

    QgsLocatorResult result;
    result.displayString = searchText;
    result.userData = QVariant::fromValue( action );
    result.icon = action->icon();
    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
    {
      found << action;
      emit resultFetched( result );
    }
  }
}

//
// QgsActiveLayerFeaturesLocatorFilter
//

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
  mMaxTotalResults = settings.value( QStringLiteral( "locator_filters/active_layer_features/limit_global" ), 30, QgsSettings::App ).toInt();

  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( !layer )
    return QStringList();

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
      req.setFlags( QgsFeatureRequest::NoGeometry );
    QString enhancedSearch = searchString;
    enhancedSearch.replace( ' ', '%' );
    req.setFilterExpression( QStringLiteral( "%1 ILIKE '%%2%'" )
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
    if ( field.configurationFlags().testFlag( QgsField::ConfigurationFlag::NotSearchable ) )
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
    completionList.append( QStringLiteral( "@%1 " ).arg( field.name() ) );

    if ( field.type() == QVariant::String )
    {
      expressionParts << QStringLiteral( "%1 ILIKE '%%2%'" ).arg( QgsExpression::quotedColumnRef( field.name() ),
                      searchString );
    }
    else if ( allowNumeric && field.isNumeric() )
    {
      expressionParts << QStringLiteral( "%1 = %2" ).arg( QgsExpression::quotedColumnRef( field.name() ), QString::number( numericalValue, 'g', 17 ) );
    }
  }

  QString expression = QStringLiteral( "(%1)" ).arg( expressionParts.join( QLatin1String( " ) OR ( " ) ) );

  QgsFeatureRequest req;
  if ( !mDispExpression.needsGeometry() )
    req.setFlags( QgsFeatureRequest::NoGeometry );
  req.setFilterExpression( expression );
  if ( isRestricting )
    req.setSubsetOfAttributes( subsetOfAttributes );

  req.setLimit( mMaxTotalResults );
  mFieldIterator = layer->getFeatures( req );

  mLayerId = layer->id();
  mLayerIcon = QgsMapLayerModel::iconForLayer( layer );
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
  for ( const QString &field : qgis::as_const( mFieldsCompletion ) )
  {
    QgsLocatorResult result;
    result.displayString = QStringLiteral( "@%1" ).arg( field );
    result.description = tr( "Limit the search to the field '%1'" ).arg( field );
    result.userData = QVariantMap( {{QStringLiteral( "type" ), QVariant::fromValue( ResultType::FieldRestriction )},
      {QStringLiteral( "search_text" ), QStringLiteral( "%1 @%2 " ).arg( prefix(), field ) } } );
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

      result.displayString =  mDispExpression.evaluate( &mContext ).toString();
      result.userData = QVariantMap(
      {
        {QStringLiteral( "type" ), QVariant::fromValue( ResultType::Feature )},
        {QStringLiteral( "feature_id" ), f.id()},
        {QStringLiteral( "layer_id" ), mLayerId}
      } );
      result.icon = mLayerIcon;
      result.score = static_cast< double >( searchString.length() ) / result.displayString.size();
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
          result.displayString = QStringLiteral( "%1 (%2)" ).arg( attrString, mAttributeAliases[idx] );
        else
          result.displayString = attrString;
        break;
      }
      idx++;
    }
    if ( result.displayString.isEmpty() )
      continue; //not sure how this result slipped through...

    result.description = mDispExpression.evaluate( &mContext ).toString();
    result.userData = QVariantMap(
    {
      {QStringLiteral( "type" ), QVariant::fromValue( ResultType::Feature )},
      {QStringLiteral( "feature_id" ), f.id()},
      {QStringLiteral( "layer_id" ), mLayerId}
    } );
    result.icon = mLayerIcon;
    result.score = static_cast< double >( searchString.length() ) / result.displayString.size();
    emit resultFetched( result );

    featuresFound << f.id();
    if ( featuresFound.count() >= mMaxTotalResults )
      break;
  }
}

void QgsActiveLayerFeaturesLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QVariantMap data = result.userData.value<QVariantMap>();
  switch ( data.value( QStringLiteral( "type" ) ).value<ResultType>() )
  {
    case ResultType::Feature:
    {
      QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( data.value( QStringLiteral( "layer_id" ) ).toString() );
      if ( layer )
      {
        QgsFeatureId fid = data.value( QStringLiteral( "feature_id" ) ).value<QgsFeatureId>();
        QgisApp::instance()->mapCanvas()->zoomToFeatureIds( layer, QgsFeatureIds() << fid );
        QgisApp::instance()->mapCanvas()->flashFeatureIds( layer, QgsFeatureIds() << fid );
      }
      break;
    }
    case ResultType::FieldRestriction:
    {
      // this is a field restriction
      QgisApp::instance()->locatorWidget()->search( data.value( QStringLiteral( "search_text" ) ).toString() );
      break;
    }
  }
}

void QgsActiveLayerFeaturesLocatorFilter::openConfigWidget( QWidget *parent )
{
  QString key = "locator_filters/active_layer_features";
  QgsSettings settings;
  std::unique_ptr<QDialog> dlg( new QDialog( parent ) );
  dlg->restoreGeometry( settings.value( QStringLiteral( "Windows/%1/geometry" ).arg( key ) ).toByteArray() );
  dlg->setWindowTitle( "All layers features locator filter" );
  QFormLayout *formLayout = new QFormLayout;
  QSpinBox *globalLimitSpinBox = new QSpinBox( dlg.get() );
  globalLimitSpinBox->setValue( settings.value( QStringLiteral( "%1/limit_global" ).arg( key ), 30, QgsSettings::App ).toInt() );
  globalLimitSpinBox->setMinimum( 1 );
  globalLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results:" ), globalLimitSpinBox );
  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );
  connect( buttonbBox, &QDialogButtonBox::accepted, [&]()
  {
    settings.setValue( QStringLiteral( "%1/limit_global" ).arg( key ), globalLimitSpinBox->value(), QgsSettings::App );
    dlg->accept();
  } );
  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );
  dlg->exec();
}

//
// QgsAllLayersFeaturesLocatorFilter
//

QgsAllLayersFeaturesLocatorFilter::QgsAllLayersFeaturesLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsAllLayersFeaturesLocatorFilter *QgsAllLayersFeaturesLocatorFilter::clone() const
{
  return new QgsAllLayersFeaturesLocatorFilter();
}

QStringList QgsAllLayersFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext &context )
{
  // Normally skip very short search strings, unless when specifically searching using this filter
  if ( string.length() < 3 && !context.usingPrefix )
    return QStringList();

  QgsSettings settings;
  mMaxTotalResults = settings.value( "locator_filters/all_layers_features/limit_global", 15, QgsSettings::App ).toInt();
  mMaxResultsPerLayer = settings.value( "locator_filters/all_layers_features/limit_per_layer", 8, QgsSettings::App ).toInt();

  mPreparedLayers.clear();
  const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  for ( auto it = layers.constBegin(); it != layers.constEnd(); ++it )
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( it.value() );
    if ( !layer || !layer->dataProvider() || !layer->flags().testFlag( QgsMapLayer::Searchable ) )
      continue;

    QgsExpression expression( layer->displayExpression() );
    QgsExpressionContext context;
    context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
    expression.prepare( &context );

    QgsFeatureRequest req;
    req.setSubsetOfAttributes( qgis::setToList( expression.referencedAttributeIndexes( layer->fields() ) ) );
    if ( !expression.needsGeometry() )
      req.setFlags( QgsFeatureRequest::NoGeometry );
    QString enhancedSearch = string;
    enhancedSearch.replace( ' ', '%' );
    req.setFilterExpression( QStringLiteral( "%1 ILIKE '%%2%'" )
                             .arg( layer->displayExpression(), enhancedSearch ) );
    req.setLimit( mMaxResultsPerLayer );

    QgsFeatureRequest exactMatchRequest = req;
    exactMatchRequest.setFilterExpression( QStringLiteral( "%1 ILIKE '%2'" )
                                           .arg( layer->displayExpression(), enhancedSearch ) );
    exactMatchRequest.setLimit( mMaxResultsPerLayer );

    std::shared_ptr<PreparedLayer> preparedLayer( new PreparedLayer() );
    preparedLayer->expression = expression;
    preparedLayer->context = context;
    preparedLayer->layerId = layer->id();
    preparedLayer->layerName = layer->name();
    preparedLayer->featureSource.reset( new QgsVectorLayerFeatureSource( layer ) );
    preparedLayer->request = req;
    preparedLayer->exactMatchRequest = exactMatchRequest;
    preparedLayer->layerIcon = QgsMapLayerModel::iconForLayer( layer );

    mPreparedLayers.append( preparedLayer );
  }

  return QStringList();
}

void QgsAllLayersFeaturesLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  int foundInCurrentLayer;
  int foundInTotal = 0;
  QgsFeature f;

  // we cannot used const loop since iterator::nextFeature is not const
  for ( auto preparedLayer : qgis::as_const( mPreparedLayers ) )
  {
    foundInCurrentLayer = 0;

    QgsFeatureIds foundFeatureIds;

    QgsFeatureIterator exactMatchIt = preparedLayer->featureSource->getFeatures( preparedLayer->exactMatchRequest );
    while ( exactMatchIt.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return;

      QgsLocatorResult result;
      result.group = preparedLayer->layerName;

      preparedLayer->context.setFeature( f );

      result.displayString = preparedLayer->expression.evaluate( &( preparedLayer->context ) ).toString();

      result.userData = QVariantList() << f.id() << preparedLayer->layerId;
      foundFeatureIds << f.id();
      result.icon = preparedLayer->layerIcon;
      result.score = static_cast< double >( string.length() ) / result.displayString.size();

      result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );
      emit resultFetched( result );

      foundInCurrentLayer++;
      foundInTotal++;
      if ( foundInCurrentLayer >= mMaxResultsPerLayer )
        break;
    }
    if ( foundInCurrentLayer >= mMaxResultsPerLayer )
      continue;
    if ( foundInTotal >= mMaxTotalResults )
      break;

    QgsFeatureIterator it = preparedLayer->featureSource->getFeatures( preparedLayer->request );
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        return;

      if ( foundFeatureIds.contains( f.id() ) )
        continue;

      QgsLocatorResult result;
      result.group = preparedLayer->layerName;

      preparedLayer->context.setFeature( f );

      result.displayString = preparedLayer->expression.evaluate( &( preparedLayer->context ) ).toString();

      result.userData = QVariantList() << f.id() << preparedLayer->layerId;
      result.icon = preparedLayer->layerIcon;
      result.score = static_cast< double >( string.length() ) / result.displayString.size();

      result.actions << QgsLocatorResult::ResultAction( OpenForm, tr( "Open form…" ) );
      emit resultFetched( result );

      foundInCurrentLayer++;
      foundInTotal++;
      if ( foundInCurrentLayer >= mMaxResultsPerLayer )
        break;
    }
    if ( foundInTotal >= mMaxTotalResults )
      break;
  }
}

void QgsAllLayersFeaturesLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  triggerResultFromAction( result, NoEntry );
}

void QgsAllLayersFeaturesLocatorFilter::triggerResultFromAction( const QgsLocatorResult &result, const int actionId )
{
  QVariantList dataList = result.userData.toList();
  QgsFeatureId fid = dataList.at( 0 ).toLongLong();
  QString layerId = dataList.at( 1 ).toString();
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  if ( actionId == OpenForm )
  {
    QgsFeature f;
    QgsFeatureRequest request;
    request.setFilterFid( fid );
    bool fetched = layer->getFeatures( request ).nextFeature( f );
    if ( !fetched )
      return;
    QgsFeatureAction action( tr( "Attributes changed" ), f, layer, QString(), -1, QgisApp::instance() );
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

void QgsAllLayersFeaturesLocatorFilter::openConfigWidget( QWidget *parent )
{
  QString key = "locator_filters/all_layers_features";
  QgsSettings settings;
  std::unique_ptr<QDialog> dlg( new QDialog( parent ) );
  dlg->restoreGeometry( settings.value( QStringLiteral( "Windows/%1/geometry" ).arg( key ) ).toByteArray() );
  dlg->setWindowTitle( "All layers features locator filter" );
  QFormLayout *formLayout = new QFormLayout;
  QSpinBox *globalLimitSpinBox = new QSpinBox( dlg.get() );
  globalLimitSpinBox->setValue( settings.value( QStringLiteral( "%1/limit_global" ).arg( key ), 15, QgsSettings::App ).toInt() );
  globalLimitSpinBox->setMinimum( 1 );
  globalLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results:" ), globalLimitSpinBox );
  QSpinBox *perLayerLimitSpinBox = new QSpinBox( dlg.get() );
  perLayerLimitSpinBox->setValue( settings.value( QStringLiteral( "%1/limit_per_layer" ).arg( key ), 8, QgsSettings::App ).toInt() );
  perLayerLimitSpinBox->setMinimum( 1 );
  perLayerLimitSpinBox->setMaximum( 200 );
  formLayout->addRow( tr( "&Maximum number of results per layer:" ), perLayerLimitSpinBox );
  QDialogButtonBox *buttonbBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlg.get() );
  formLayout->addRow( buttonbBox );
  dlg->setLayout( formLayout );
  connect( buttonbBox, &QDialogButtonBox::accepted, [&]()
  {
    settings.setValue( QStringLiteral( "%1/limit_global" ).arg( key ), globalLimitSpinBox->value(), QgsSettings::App );
    settings.setValue( QStringLiteral( "%1/limit_per_layer" ).arg( key ), perLayerLimitSpinBox->value(), QgsSettings::App );
    dlg->accept();
  } );
  connect( buttonbBox, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject );
  dlg->exec();
}


//
// QgsExpressionCalculatorLocatorFilter
//
QgsExpressionCalculatorLocatorFilter::QgsExpressionCalculatorLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsExpressionCalculatorLocatorFilter *QgsExpressionCalculatorLocatorFilter::clone() const
{
  return new QgsExpressionCalculatorLocatorFilter();
}

void QgsExpressionCalculatorLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback * )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
          << QgsExpressionContextUtils::layerScope( QgisApp::instance()->activeLayer() );

  QString error;
  if ( QgsExpression::checkExpression( string, &context, error ) )
  {
    QgsExpression exp( string );
    QString resultString = exp.evaluate( &context ).toString();
    if ( !resultString.isEmpty() )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Copy “%1” to clipboard" ).arg( resultString );
      result.userData = resultString;
      result.score = 1;
      emit resultFetched( result );
    }
  }
}

void QgsExpressionCalculatorLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QApplication::clipboard()->setText( result.userData.toString() );
}

// SettingsLocatorFilter
//
QgsSettingsLocatorFilter::QgsSettingsLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsSettingsLocatorFilter *QgsSettingsLocatorFilter::clone() const
{
  return new QgsSettingsLocatorFilter();
}

void QgsSettingsLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback * )
{
  QMap<QString, QMap<QString, QString>> matchingSettingsPagesMap;

  QMap<QString, int > optionsPagesMap = QgisApp::instance()->optionsPagesMap();
  for ( auto optionsPagesIterator = optionsPagesMap.constBegin(); optionsPagesIterator != optionsPagesMap.constEnd(); ++optionsPagesIterator )
  {
    QString title = optionsPagesIterator.key();
    matchingSettingsPagesMap.insert( title + " (" + tr( "Options" ) + ")", settingsPage( QStringLiteral( "optionpage" ), QString::number( optionsPagesIterator.value() ) ) );
  }

  QMap<QString, QString> projectPropertyPagesMap = QgisApp::instance()->projectPropertiesPagesMap();
  for ( auto projectPropertyPagesIterator = projectPropertyPagesMap.constBegin(); projectPropertyPagesIterator != projectPropertyPagesMap.constEnd(); ++projectPropertyPagesIterator )
  {
    QString title = projectPropertyPagesIterator.key();
    matchingSettingsPagesMap.insert( title + " (" + tr( "Project Properties" ) + ")", settingsPage( QStringLiteral( "projectpropertypage" ), projectPropertyPagesIterator.value() ) );
  }

  QMap<QString, QString> settingPagesMap = QgisApp::instance()->settingPagesMap();
  for ( auto settingPagesIterator = settingPagesMap.constBegin(); settingPagesIterator != settingPagesMap.constEnd(); ++settingPagesIterator )
  {
    QString title = settingPagesIterator.key();
    matchingSettingsPagesMap.insert( title, settingsPage( QStringLiteral( "settingspage" ), settingPagesIterator.value() ) );
  }

  for ( auto matchingSettingsPagesIterator = matchingSettingsPagesMap.constBegin(); matchingSettingsPagesIterator != matchingSettingsPagesMap.constEnd(); ++matchingSettingsPagesIterator )
  {
    QString title = matchingSettingsPagesIterator.key();
    QMap<QString, QString> settingsPage = matchingSettingsPagesIterator.value();
    QgsLocatorResult result;
    result.filter = this;
    result.displayString = title;
    result.userData.setValue( settingsPage );

    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

QMap<QString, QString> QgsSettingsLocatorFilter::settingsPage( const QString &type,  const QString &page )
{
  QMap<QString, QString> returnPage;
  returnPage.insert( QStringLiteral( "type" ), type );
  returnPage.insert( QStringLiteral( "page" ), page );
  return returnPage;
}

void QgsSettingsLocatorFilter::triggerResult( const QgsLocatorResult &result )
{

  QMap<QString, QString> settingsPage = qvariant_cast<QMap<QString, QString>>( result.userData );
  QString type = settingsPage.value( QStringLiteral( "type" ) );
  QString page = settingsPage.value( QStringLiteral( "page" ) );

  if ( type == QLatin1String( "optionpage" ) )
  {
    const int pageNumber = page.toInt();
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), QString(), pageNumber );
  }
  else if ( type == QLatin1String( "projectpropertypage" ) )
  {
    QgisApp::instance()->showProjectProperties( page );
  }
  else if ( type == QLatin1String( "settingspage" ) )
  {
    QgisApp::instance()->showSettings( page );
  }
}

// QgBookmarkLocatorFilter
//

QgsBookmarkLocatorFilter::QgsBookmarkLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsBookmarkLocatorFilter *QgsBookmarkLocatorFilter::clone() const
{
  return new QgsBookmarkLocatorFilter();
}

void QgsBookmarkLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{
  QMap<QString, QModelIndex> bookmarkMap = QgisApp::instance()->getBookmarkIndexMap();

  QMapIterator<QString, QModelIndex> i( bookmarkMap );

  while ( i.hasNext() )
  {
    i.next();

    if ( feedback->isCanceled() )
      return;

    QString name = i.key();
    QModelIndex index = i.value();
    QgsLocatorResult result;
    result.filter = this;
    result.displayString = name;
    result.userData = index;
    result.icon = QgsApplication::getThemeIcon( QStringLiteral( "/mItemBookmark.svg" ) );

    if ( context.usingPrefix && string.isEmpty() )
    {
      emit resultFetched( result );
      continue;
    }

    result.score = fuzzyScore( result.displayString, string );

    if ( result.score > 0 )
      emit resultFetched( result );
  }
}

void QgsBookmarkLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QModelIndex index = qvariant_cast<QModelIndex>( result.userData );
  QgisApp::instance()->zoomToBookmarkIndex( index );
}

//
// QgsGotoLocatorFilter
//

QgsGotoLocatorFilter::QgsGotoLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

QgsGotoLocatorFilter *QgsGotoLocatorFilter::clone() const
{
  return new QgsGotoLocatorFilter();
}

void QgsGotoLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  if ( feedback->isCanceled() )
    return;

  const QgsCoordinateReferenceSystem currentCrs = QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs();
  const QgsCoordinateReferenceSystem wgs84Crs( QStringLiteral( "EPSG:4326" ) );

  bool okX = false;
  bool okY = false;
  double posX = 0.0;
  double posY = 0.0;
  bool posIsDms = false;
  QLocale locale;

  // Coordinates such as 106.8468,-6.3804
  QRegularExpression separatorRx( QStringLiteral( "^([0-9\\-\\%1\\%2]*)[\\s%3]*([0-9\\-\\%1\\%2]*)$" ).arg( locale.decimalPoint(),
                                  locale.groupSeparator(),
                                  locale.decimalPoint() != ',' && locale.groupSeparator() != ',' ? QStringLiteral( "\\," ) : QString() ) );
  QRegularExpressionMatch match = separatorRx.match( string.trimmed() );
  if ( match.hasMatch() )
  {
    posX = locale.toDouble( match.captured( 1 ), &okX );
    posY = locale.toDouble( match.captured( 2 ), &okY );
  }

  if ( !match.hasMatch() || !okX || !okY )
  {
    // Digit detection using user locale failed, use default C decimal separators
    separatorRx = QRegularExpression( QStringLiteral( "^([0-9\\-\\.]*)[\\s\\,]*([0-9\\-\\.]*)$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      posX = match.captured( 1 ).toDouble( &okX );
      posY = match.captured( 2 ).toDouble( &okY );
    }
  }

  if ( !match.hasMatch() )
  {
    // Check if the string is a pair of degree minute second
    separatorRx = QRegularExpression( QStringLiteral( "^((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)[,\\s]+((?:([-+nsew])\\s*)?\\d{1,3}(?:[^0-9.]+[0-5]?\\d)?[^0-9.]+[0-5]?\\d(?:\\.\\d+)?[^0-9.,]*[-+nsew]?)$" ) );
    match = separatorRx.match( string.trimmed() );
    if ( match.hasMatch() )
    {
      posIsDms = true;
      bool isEasting = false;
      posX = QgsCoordinateUtils::dmsToDecimal( match.captured( 1 ), &okX, &isEasting );
      posY = QgsCoordinateUtils::dmsToDecimal( match.captured( 3 ), &okY );
      if ( !isEasting )
        std::swap( posX, posY );
    }
  }

  if ( okX && okY )
  {
    QVariantMap data;
    QgsPointXY point( posX, posY );
    data.insert( QStringLiteral( "point" ), point );

    bool withinWgs84 = wgs84Crs.bounds().contains( point );
    if ( !posIsDms && currentCrs != wgs84Crs )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1 %2 (Map CRS, %3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), currentCrs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 0.9;
      emit resultFetched( result );
    }

    if ( withinWgs84 )
    {
      if ( currentCrs != wgs84Crs )
      {
        QgsCoordinateTransform transform( wgs84Crs, currentCrs, QgsProject::instance()->transformContext() );
        QgsPointXY transformedPoint;
        try
        {
          transformedPoint = transform.transform( point );
        }
        catch ( const QgsException &e )
        {
          Q_UNUSED( e )
          return;
        }
        data[QStringLiteral( "point" )] = transformedPoint;
      }

      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1° %2° (%3)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ), wgs84Crs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 1.0;
      emit resultFetched( result );
    }
    return;
  }

  QMap<int, double> scales;
  scales[0] = 739571909;
  scales[1] = 369785954;
  scales[2] = 184892977;
  scales[3] = 92446488;
  scales[4] = 46223244;
  scales[5] = 23111622;
  scales[6] = 11555811;
  scales[7] = 5777905;
  scales[8] = 2888952;
  scales[9] = 1444476;
  scales[10] = 722238;
  scales[11] = 361119;
  scales[12] = 180559;
  scales[13] = 90279;
  scales[14] = 45139;
  scales[15] = 22569;
  scales[16] = 11284;
  scales[17] = 5642;
  scales[18] = 2821;
  scales[19] = 1500;
  scales[20] = 1000;
  scales[21] = 282;

  QUrl url( string );
  if ( url.isValid() )
  {
    double scale = 0.0;
    int meters = 0;
    okX = false;
    okY = false;
    posX = 0.0;
    posY = 0.0;
    if ( url.hasFragment() )
    {
      // Check for OSM/Leaflet/OpenLayers pattern (e.g. http://www.openstreetmap.org/#map=6/46.423/4.746)
      QStringList fragments = url.fragment().split( '&' );
      for ( const QString &fragment : fragments )
      {
        if ( fragment.startsWith( QLatin1String( "map=" ) ) )
        {
          QStringList params = fragment.mid( 4 ).split( '/' );
          if ( params.size() >= 3 )
          {
            if ( scales.contains( params.at( 0 ).toInt() ) )
            {
              scale = scales.value( params.at( 0 ).toInt() );
            }
            posX = params.at( 2 ).toDouble( &okX );
            posY = params.at( 1 ).toDouble( &okY );
          }
          break;
        }
      }
    }

    if ( !okX && !okY )
    {
      QRegularExpression locationRx( QStringLiteral( "google.*\\/@([0-9\\-\\.\\,]*)(z|m|a)" ) );
      match = locationRx.match( string );
      if ( match.hasMatch() )
      {
        QStringList params = match.captured( 1 ).split( ',' );
        if ( params.size() == 3 )
        {
          posX = params.at( 1 ).toDouble( &okX );
          posY = params.at( 0 ).toDouble( &okY );

          if ( okX && okY )
          {
            if ( match.captured( 2 ) == QChar( 'z' ) && scales.contains( static_cast<int>( params.at( 2 ).toDouble() ) ) )
            {
              scale = scales.value( static_cast<int>( params.at( 2 ).toDouble() ) );
            }
            else if ( match.captured( 2 ) == QChar( 'm' ) )
            {
              // satellite view URL, scale to be derived from canvas height
              meters = params.at( 2 ).toInt();
            }
            else if ( match.captured( 2 ) == QChar( 'a' ) )
            {
              // street view URL, use most zoomed in scale value
              scale = scales.value( 21 );
            }
          }
        }
      }
    }

    if ( okX && okY )
    {
      QVariantMap data;
      QgsPointXY point( posX, posY );
      QgsPointXY dataPoint = point;
      bool withinWgs84 = wgs84Crs.bounds().contains( point );
      if ( withinWgs84 && currentCrs != wgs84Crs )
      {
        QgsCoordinateTransform transform( wgs84Crs, currentCrs, QgsProject::instance()->transformContext() );
        dataPoint = transform.transform( point );
      }
      data.insert( QStringLiteral( "point" ), dataPoint );

      if ( meters > 0 )
      {
        QSize outputSize = QgisApp::instance()->mapCanvas()->mapSettings().outputSize();
        QgsDistanceArea da;
        da.setSourceCrs( currentCrs, QgsProject::instance()->transformContext() );
        da.setEllipsoid( QgsProject::instance()->ellipsoid() );
        double height = da.measureLineProjected( dataPoint, meters );
        double width = outputSize.width() * ( height / outputSize.height() );

        QgsRectangle extent;
        extent.setYMinimum( dataPoint.y() -  height / 2.0 );
        extent.setYMaximum( dataPoint.y() +  height / 2.0 );
        extent.setXMinimum( dataPoint.x() -  width / 2.0 );
        extent.setXMaximum( dataPoint.x() +  width / 2.0 );

        QgsScaleCalculator calculator;
        calculator.setMapUnits( currentCrs.mapUnits() );
        calculator.setDpi( QgisApp::instance()->mapCanvas()->mapSettings().outputDpi() );
        scale = calculator.calculate( extent, outputSize.width() );
      }

      if ( scale > 0.0 )
      {
        data.insert( QStringLiteral( "scale" ), scale );
      }

      QgsLocatorResult result;
      result.filter = this;
      result.displayString = tr( "Go to %1° %2° %3(%4)" ).arg( locale.toString( point.x(), 'g', 10 ), locale.toString( point.y(), 'g', 10 ),
                             scale > 0.0 ? tr( "at scale 1:%1 " ).arg( scale ) : QString(),
                             wgs84Crs.userFriendlyIdentifier() );
      result.userData = data;
      result.score = 1.0;
      emit resultFetched( result );
    }
  }
}

void QgsGotoLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QgsMapCanvas *mapCanvas = QgisApp::instance()->mapCanvas();

  QVariantMap data = result.userData.toMap();
  QgsPointXY point = data[QStringLiteral( "point" )].value<QgsPointXY>();
  mapCanvas->setCenter( point );
  if ( data.contains( QStringLiteral( "scale" ) ) )
  {
    mapCanvas->zoomScale( data[QStringLiteral( "scale" )].toDouble() );
  }
  else
  {
    mapCanvas->refresh();
  }

  mapCanvas->flashGeometries( QList< QgsGeometry >() << QgsGeometry::fromPointXY( point ) );
}
