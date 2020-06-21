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

#include <QToolButton>
#include <QClipboard>

#include "qgsapplication.h"
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
    tooltip.replace( QStringLiteral( "..." ), QString() );
    tooltip.replace( QString( QChar( 0x2026 ) ), QString() );
    searchText.replace( QStringLiteral( "..." ), QString() );
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

void QgsActiveLayerFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext &context )
{
  // Normally skip very short search strings, unless when specifically searching using this filter
  if ( string.length() < 3 && !context.usingPrefix )
    return;

  bool allowNumeric = false;
  double numericalValue = string.toDouble( &allowNumeric );

  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgisApp::instance()->activeLayer() );
  if ( !layer )
    return;

  mDispExpression = QgsExpression( layer->displayExpression() );
  mContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  mDispExpression.prepare( &mContext );

  // build up request expression
  QStringList expressionParts;
  const QgsFields fields = layer->fields();
  for ( const QgsField &field : fields )
  {
    if ( field.type() == QVariant::String )
    {
      expressionParts << QStringLiteral( "%1 ILIKE '%%2%'" ).arg( QgsExpression::quotedColumnRef( field.name() ),
                      string );
    }
    else if ( allowNumeric && field.isNumeric() )
    {
      expressionParts << QStringLiteral( "%1 = %2" ).arg( QgsExpression::quotedColumnRef( field.name() ), QString::number( numericalValue, 'g', 17 ) );
    }
  }

  QString expression = QStringLiteral( "(%1)" ).arg( expressionParts.join( QStringLiteral( " ) OR ( " ) ) );

  QgsFeatureRequest req;
  req.setFlags( QgsFeatureRequest::NoGeometry );
  req.setFilterExpression( expression );
  req.setLimit( 30 );
  mIterator = layer->getFeatures( req );

  mLayerId = layer->id();
  mLayerIcon = QgsMapLayerModel::iconForLayer( layer );
  mAttributeAliases.clear();
  for ( int idx = 0; idx < layer->fields().size(); ++idx )
  {
    mAttributeAliases.append( layer->attributeDisplayName( idx ) );
  }
}

void QgsActiveLayerFeaturesLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  int found = 0;
  QgsFeature f;

  while ( mIterator.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      return;

    QgsLocatorResult result;

    mContext.setFeature( f );

    // find matching field content
    int idx = 0;
    const QgsAttributes attributes = f.attributes();
    for ( const QVariant &var : attributes )
    {
      QString attrString = var.toString();
      if ( attrString.contains( string, Qt::CaseInsensitive ) )
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

    result.userData = QVariantList() << f.id() << mLayerId;
    result.icon = mLayerIcon;
    result.score = static_cast< double >( string.length() ) / result.displayString.size();
    emit resultFetched( result );

    found++;
    if ( found >= 30 )
      break;
  }
}

void QgsActiveLayerFeaturesLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QVariantList dataList = result.userData.toList();
  QgsFeatureId id = dataList.at( 0 ).toLongLong();
  QString layerId = dataList.at( 1 ).toString();
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgisApp::instance()->mapCanvas()->zoomToFeatureIds( layer, QgsFeatureIds() << id );
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

void QgsAllLayersFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext &context )
{
  // Normally skip very short search strings, unless when specifically searching using this filter
  if ( string.length() < 3 && !context.usingPrefix )
    return;

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
    req.setLimit( 6 );

    QgsFeatureRequest exactMatchRequest = req;
    exactMatchRequest.setFilterExpression( QStringLiteral( "%1 ILIKE '%2'" )
                                           .arg( layer->displayExpression(), enhancedSearch ) );
    exactMatchRequest.setLimit( 10 );

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
  }
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

    result.score = fuzzyScore( result.displayString, string );;

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
