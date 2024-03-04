/***************************************************************************
                        qgslayoutlocatorfilters.cpp
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

#include "qgslayoutlocatorfilter.h"
#include "qgsproject.h"
#include "qgsmasterlayoutinterface.h"
#include "qgslayoutmanager.h"
#include "qgisapp.h"


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
    result.setUserData( layout->name() );

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
  const QString layoutName = result.userData().toString();
  QgsMasterLayoutInterface *layout = QgsProject::instance()->layoutManager()->layoutByName( layoutName );
  if ( !layout )
    return;

  QgisApp::instance()->openLayoutDesignerDialog( layout );
}
