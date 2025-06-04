/***************************************************************************
  qgslabelthinningsettings.cpp
  ----------------------------
  Date                 : December 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelthinningsettings.h"
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"


void QgsLabelThinningSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::LabelMarginDistance ) )
  {
    context.setOriginalValueVariable( mLabelMarginDistance );
    mLabelMarginDistance = properties.valueAsDouble( QgsPalLayerSettings::Property::LabelMarginDistance, context, mLabelMarginDistance );
  }
  if ( properties.isActive( QgsPalLayerSettings::Property::RemoveDuplicateLabels ) )
  {
    context.setOriginalValueVariable( mAllowDuplicateRemoval );
    mAllowDuplicateRemoval = properties.valueAsBool( QgsPalLayerSettings::Property::RemoveDuplicateLabels, context, mAllowDuplicateRemoval );
  }
  if ( properties.isActive( QgsPalLayerSettings::Property::RemoveDuplicateLabelDistance ) )
  {
    context.setOriginalValueVariable( mMinDistanceToDuplicate );
    mMinDistanceToDuplicate = properties.valueAsDouble( QgsPalLayerSettings::Property::RemoveDuplicateLabelDistance, context, mMinDistanceToDuplicate );
  }
}
