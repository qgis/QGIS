/***************************************************************************
                            qgsplotregistry.cpp
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
#include "qgsplotregistry.h"

#include "qgsbarchartplot.h"
#include "qgslinechartplot.h"
#include "qgspiechartplot.h"
#include "qgsplot.h"

#include "moc_qgsplotregistry.cpp"

QgsPlotRegistry::QgsPlotRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsPlotRegistry::~QgsPlotRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsPlotRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  addPlotType( new QgsPlotMetadata( "bar"_L1, QObject::tr( "Bar chart" ), QgsBarChartPlot::create, QgsBarChartPlot::createDataGatherer ) );
  addPlotType( new QgsPlotMetadata( "line"_L1, QObject::tr( "Line chart" ), QgsLineChartPlot::create, QgsLineChartPlot::createDataGatherer ) );
  addPlotType( new QgsPlotMetadata( "pie"_L1, QObject::tr( "Pie chart" ), QgsPieChartPlot::create, QgsPieChartPlot::createDataGatherer ) );

  return true;
}

QgsPlotAbstractMetadata *QgsPlotRegistry::plotMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsPlotRegistry::addPlotType( QgsPlotAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit plotAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsPlotRegistry::removePlotType( const QString &type )
{
  if ( !mMetadata.contains( type ) )
    return false;

  emit plotAboutToBeRemoved( type );
  delete mMetadata.take( type );
  return true;
}

QgsPlot *QgsPlotRegistry::createPlot( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createPlot();
}

QMap<QString, QString> QgsPlotRegistry::plotTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}
