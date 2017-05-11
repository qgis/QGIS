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
#include "qgscomposition.h"
#include "qgslayoutmanager.h"
#include <QToolButton>

QgsLayerTreeLocatorFilter::QgsLayerTreeLocatorFilter( QObject *parent )
  : QgsLocatorFilter( parent )
{}

void QgsLayerTreeLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  QgsLayerTree *tree = QgsProject::instance()->layerTreeRoot();
  QList<QgsLayerTreeLayer *> layers = tree->findLayers();
  Q_FOREACH ( QgsLayerTreeLayer *layer, layers )
  {
    if ( feedback->isCanceled() )
      return;

    if ( layer->layer() && stringMatches( layer->layer()->name(), string ) )
    {
      QgsLocatorResult result;
      result.filter = this;
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

void QgsLayoutLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &, QgsFeedback *feedback )
{
  Q_FOREACH ( QgsComposition *composition, QgsProject::instance()->layoutManager()->compositions() )
  {
    if ( feedback->isCanceled() )
      return;

    if ( composition && stringMatches( composition->name(), string ) )
    {
      QgsLocatorResult result;
      result.filter = this;
      result.displayString = composition->name();
      result.userData = composition->name();
      //result.icon = QgsMapLayerModel::iconForLayer( layer->layer() );
      result.score = static_cast< double >( string.length() ) / composition->name().length();
      emit resultFetched( result );
    }
  }
}

void QgsLayoutLocatorFilter::triggerResult( const QgsLocatorResult &result )
{
  QString layoutName = result.userData.toString();
  QgsComposition *composition = QgsProject::instance()->layoutManager()->compositionByName( layoutName );
  if ( !composition )
    return;

  QgisApp::instance()->openComposer( composition );
}




QgsActionLocatorFilter::QgsActionLocatorFilter( const QList<QWidget *> &parentObjectsForActions, QObject *parent )
  : QgsLocatorFilter( parent )
  , mActionParents( parentObjectsForActions )
{
  setUseWithoutPrefix( false );
}

void QgsActionLocatorFilter::fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback )
{
  QList<QAction *> found;

  Q_FOREACH ( QWidget *object, mActionParents )
  {
    if ( feedback->isCanceled() )
      return;

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
  QList< QWidget *> children = parent->findChildren<QWidget *>();
  Q_FOREACH ( QWidget *widget, children )
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
      result.filter = this;
      result.displayString = searchText;
      result.userData = QVariant::fromValue( action );
      result.icon = action->icon();
      result.score = static_cast< double >( string.length() ) / searchText.length();
      emit resultFetched( result );
      found << action;
    }
  }
}
