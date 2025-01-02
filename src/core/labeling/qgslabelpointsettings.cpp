/***************************************************************************
  qgslabelpointsettings.cpp
  ----------------------------
  Date                 : May 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelpointsettings.h"
#include "moc_qgslabelpointsettings.cpp"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"


void QgsLabelPointSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  // TODO -- ideally quadrant and ordered positions would also be evaluated here,
  // but they have been left in their original evaluation location for now to avoid
  // any unforeseen unwanted side effects...
  if ( properties.isActive( QgsPalLayerSettings::Property::MaximumDistance ) )
  {
    context.setOriginalValueVariable( mMaximumDistance );
    mMaximumDistance = properties.valueAsDouble( QgsPalLayerSettings::Property::MaximumDistance, context, mMaximumDistance );
  }
}
