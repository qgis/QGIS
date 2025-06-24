/***************************************************************************
                            qgschartregistry.cpp
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

#include "qgschartregistry.h"
#include "moc_qgschartregistry.cpp"
#include "qgschart.h"
#include "qgsplot.h"

QgsChartRegistry::QgsChartRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsChartRegistry::~QgsChartRegistry()
{
  qDeleteAll( mMetadata );
}

bool QgsChartRegistry::populate()
{
  if ( !mMetadata.isEmpty() )
    return false;

  addChartType( new QgsChartMetadata( QLatin1String( "bar" ), QObject::tr( "Bar chart" ), QgsBarChart::create ) );
  addChartType( new QgsChartMetadata( QLatin1String( "line" ), QObject::tr( "Line chart" ), QgsLineChart::create ) );

  return true;
}

QgsChartAbstractMetadata *QgsChartRegistry::chartMetadata( const QString &type ) const
{
  return mMetadata.value( type );
}

bool QgsChartRegistry::addChartType( QgsChartAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->type() ) )
    return false;

  mMetadata[metadata->type()] = metadata;
  emit chartAdded( metadata->type(), metadata->visibleName() );
  return true;
}

bool QgsChartRegistry::removeChartType( const QString &type )
{
  if ( !mMetadata.contains( type ) )
    return false;

  emit chartAboutToBeRemoved( type );
  delete mMetadata.take( type );
  return true;
}

QgsPlot *QgsChartRegistry::createChart( const QString &type ) const
{
  if ( !mMetadata.contains( type ) )
    return nullptr;

  return mMetadata[type]->createChart();
}

QMap<QString, QString> QgsChartRegistry::chartTypes() const
{
  QMap<QString, QString> types;
  for ( auto it = mMetadata.constBegin(); it != mMetadata.constEnd(); ++it )
  {
    types.insert( it.key(), it.value()->visibleName() );
  }

  return types;
}
