/***************************************************************************
  qgslabelplacementsettings.cpp
  --------------------------
  Date                 : May 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelplacementsettings.h"

#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"
#include "qgspropertycollection.h"

void QgsLabelPlacementSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::AllowDegradedPlacement ) )
  {
    context.setOriginalValueVariable( mAllowDegradedPlacement );
    mAllowDegradedPlacement = properties.valueAsBool( QgsPalLayerSettings::Property::AllowDegradedPlacement, context, mAllowDegradedPlacement );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::OverlapHandling ) )
  {
    const QString handlingString = properties.valueAsString( QgsPalLayerSettings::Property::OverlapHandling, context );
    const QString cleanedString = handlingString.trimmed();
    if ( cleanedString.compare( "prevent"_L1, Qt::CaseInsensitive ) == 0 )
      mOverlapHandling = Qgis::LabelOverlapHandling::PreventOverlap;
    else if ( cleanedString.compare( "allowifneeded"_L1, Qt::CaseInsensitive ) == 0 )
      mOverlapHandling = Qgis::LabelOverlapHandling::AllowOverlapIfRequired;
    else if ( cleanedString.compare( "alwaysallow"_L1, Qt::CaseInsensitive ) == 0 )
      mOverlapHandling = Qgis::LabelOverlapHandling::AllowOverlapAtNoCost;
  }
}
