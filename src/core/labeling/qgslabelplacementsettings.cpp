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

  // this property is messy - to avoid breaking old projects we need to also allow it to be treated as a boolean
  if ( properties.isActive( QgsPalLayerSettings::Property::LabelAllParts ) )
  {
    const QString stringValue = properties.valueAsString( QgsPalLayerSettings::Property::LabelAllParts, context );
    bool handledAsString = false;
    if ( !stringValue.isEmpty() )
    {
      const QString cleanedString = stringValue.trimmed();
      if ( cleanedString.compare( "LargestPartOnly"_L1, Qt::CaseInsensitive ) == 0 )
      {
        handledAsString = true;
        mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::LabelLargestPartOnly;
      }
      else if ( cleanedString.compare( "LabelEveryPart"_L1, Qt::CaseInsensitive ) == 0 )
      {
        handledAsString = true;
        mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::LabelEveryPartWithEntireLabel;
      }
      else if ( cleanedString.compare( "SplitLabelTextLinesOverParts"_L1, Qt::CaseInsensitive ) == 0 )
      {
        handledAsString = true;
        mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::SplitLabelTextLinesOverParts;
      }
    }

    // fallback to old boolean compatibility
    if ( !handledAsString )
    {
      context.setOriginalValueVariable( mMultiPartBehavior == Qgis::MultiPartLabelingBehavior::LabelEveryPartWithEntireLabel );
      if ( properties.valueAsBool( QgsPalLayerSettings::Property::LabelAllParts, context, mMultiPartBehavior == Qgis::MultiPartLabelingBehavior::LabelEveryPartWithEntireLabel ) )
      {
        mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::LabelEveryPartWithEntireLabel;
      }
      else
      {
        mMultiPartBehavior = Qgis::MultiPartLabelingBehavior::LabelLargestPartOnly;
      }
    }
  }
}
