/***************************************************************************
  qgslayertreeviewlowaccuracyindicator.cpp
  --------------------------------------
  Date                 : May 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewlowaccuracyindicator.h"
#include "qgsdatums.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsexception.h"

QgsLayerTreeViewLowAccuracyIndicatorProvider::QgsLayerTreeViewLowAccuracyIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewLowAccuracyIndicatorProvider::connectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::connectSignals( layer );
  connect( layer, &QgsMapLayer::crsChanged, this, &QgsLayerTreeViewLowAccuracyIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewLowAccuracyIndicatorProvider::disconnectSignals( QgsMapLayer *layer )
{
  QgsLayerTreeViewIndicatorProvider::disconnectSignals( layer );
  disconnect( layer, &QgsMapLayer::crsChanged, this, &QgsLayerTreeViewLowAccuracyIndicatorProvider::onLayerChanged );
}

void QgsLayerTreeViewLowAccuracyIndicatorProvider::onIndicatorClicked( const QModelIndex & )
{

}

QString QgsLayerTreeViewLowAccuracyIndicatorProvider::iconName( QgsMapLayer * )
{
  return QStringLiteral( "/mIndicatorLowAccuracy.svg" );
}

QString QgsLayerTreeViewLowAccuracyIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  if ( !layer )
    return QString();

  const QgsCoordinateReferenceSystem crs = layer->crs();
  if ( !crs.isValid() )
    return QString();

  // based on datum ensemble?
  try
  {
    const QgsDatumEnsemble ensemble = crs.datumEnsemble();
    if ( ensemble.isValid() )
    {
      QString id;
      if ( !ensemble.code().isEmpty() )
        id = QStringLiteral( "<i>%1</i> (%2:%3)" ).arg( ensemble.name(), ensemble.authority(), ensemble.code() );
      else
        id = QStringLiteral( "<i>%</i>”" ).arg( ensemble.name() );

      if ( ensemble.accuracy() > 0 )
      {
        return tr( "Based on %1, which has a limited accuracy of <b>at best %2 meters</b>." ).arg( id ).arg( ensemble.accuracy() );
      }
      else
      {
        return tr( "Based on %1, which has a limited accuracy." ).arg( id );
      }
    }
  }
  catch ( QgsNotSupportedException & )
  {

  }

  // dynamic crs with no epoch?
  if ( crs.isDynamic() && std::isnan( crs.coordinateEpoch() ) )
  {
    return tr( "%1 is a dynamic CRS, but no coordinate epoch is set. Coordinates are ambiguous and of limited accuracy." ).arg( crs.userFriendlyIdentifier() );
  }

  return QString();
}

bool QgsLayerTreeViewLowAccuracyIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  const QgsSettings settings;
  if ( !settings.value( QStringLiteral( "/projections/crsAccuracyIndicator" ), false, QgsSettings::App ).toBool() )
    return false;

  if ( !layer->isValid() )
    return false;

  const QgsCoordinateReferenceSystem crs = layer->crs();
  if ( !crs.isValid() )
    return false;

  // dynamic crs with no epoch?
  if ( crs.isDynamic() && std::isnan( crs.coordinateEpoch() ) )
  {
    return true;
  }

  // based on datum ensemble?
  try
  {
    const QgsDatumEnsemble ensemble = crs.datumEnsemble();
    if ( ensemble.isValid() )
    {
      if ( ensemble.accuracy() > 0 )
      {
        if ( ensemble.accuracy() >= settings.value( QStringLiteral( "/projections/crsAccuracyWarningThreshold" ), 0.0, QgsSettings::App ).toDouble() )
          return true;
      }
      else
      {
        // unknown accuracy, always show warning
        return true;
      }
    }
  }
  catch ( QgsNotSupportedException & )
  {

  }

  return false;
}
