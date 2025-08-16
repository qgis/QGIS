/***************************************************************************
                            qgschartplotregistry.cpp
                            ------------------------
    begin                : June 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"

#include "qgschartplotregistry.h"
#include "moc_qgschartplotregistry.cpp"
#include "qgschartplot.h"
#include "qgsplot.h"

QgsChartPlotRegistry::QgsChartPlotRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsChartPlotRegistry::~QgsChartPlotRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsChartPlotRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  addChartType( new QgsChartPlotMetadata( QLatin1String( "bar" ), QObject::tr( "Bar chart" ), QgsBarChartPlot::create ) );
  addChartType( new QgsChartPlotMetadata( QLatin1String( "line" ), QObject::tr( "Line chart" ), QgsLineChartPlot::create ) );

  return true;
}

QgsChartPlotAbstractMetadata *QgsChartPlotRegistry::chartMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsChartPlotRegistry::addChartType( QgsChartPlotAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit chartAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsChartPlotRegistry::removeChartType( const QString &type )
{
  if ( !mMetadata.contains( type ) )
    return false;

  emit chartAboutToBeRemoved( type );
  delete mMetadata.take( type );
  return true;
}

QgsPlot *QgsChartPlotRegistry::createChart( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createChart();
}

QMap<QString, QString> QgsChartPlotRegistry::chartTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}
