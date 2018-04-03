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


#include "qgsinbuiltlocatorfilters.h"
#include "qgsproject.h"
#include "qgslayertree.h"
#include "qgsfeedback.h"
#include "qgisapp.h"
#include "qgsstringutils.h"
#include "qgsmaplayermodel.h"
#include "qgslayoutmanager.h"
#include "qgsmapcanvas.h"
#include <QToolButton>

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
    if ( layer->layer() && ( stringMatches( layer->layer()->name(), string ) || ( context.usingPrefix && string.isEmpty() ) ) )
    {
      QgsLocatorResult result;
      result.displayString = layer->layer()->name();
      result.userData = layer->layerId();
      result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );
      result.score = static_cast< double >( string.length() ) / layer->layer()->name().length();
      emit resultFetched( result );
    }
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
    if ( layout && ( stringMatches( layout->name(), string ) || ( context.usingPrefix && string.isEmpty() ) ) )
    {
      QgsLocatorResult result;
      result.displayString = layout->name();
      result.userData = layout->name();
      //result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );
      result.score = static_cast< double >( string.length() ) / layout->name().length();
      emit resultFetched( result );
    }
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
    searchActions( string,  object, found );
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
  Q_FOREACH ( QAction *action, parent->actions() )
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
    if ( stringMatches( searchText, string ) )
    {
      QgsLocatorResult result;
      result.displayString = searchText;
      result.userData = QVariant::fromValue( action );
      result.icon = action->icon();
      result.score = static_cast< double >( string.length() ) / searchText.length();
      emit resultFetched( result );
      found << action;
    }
  }
}

QgsActiveLayerFeaturesLocatorFilter::QgsActiveLayerFeaturesLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{
  setUseWithoutPrefix( false );
}

QgsActiveLayerFeaturesLocatorFilter *QgsActiveLayerFeaturesLocatorFilter::clone() const
{
  return new QgsActiveLayerFeaturesLocatorFilter();
}

void QgsActiveLayerFeaturesLocatorFilter::prepare( const QString &string, const QgsLocatorContext & )
{
  if ( string.length() < 3 )
    return;

  bool allowNumeric = false;
  double numericValue = string.toDouble( &allowNumeric );

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
      expressionParts << QStringLiteral( "%1 = %2" ).arg( QgsExpression::quotedColumnRef( field.name() ) ).arg( numericValue );
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
    Q_FOREACH ( const QVariant &var, f.attributes() )
    {
      QString attrString = var.toString();
      if ( attrString.contains( string, Qt::CaseInsensitive ) )
      {
        result.displayString = attrString;
        break;
      }
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
  QgsVectorLayer *layer = qobject_cast< QgsVectorLayer *>( QgsProject::instance()->mapLayer( layerId ) );
  if ( !layer )
    return;

  QgisApp::instance()->mapCanvas()->zoomToFeatureIds( layer, QgsFeatureIds() << id );
}
