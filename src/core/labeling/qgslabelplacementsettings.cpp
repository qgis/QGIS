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

#include "qgsdatadefinedutils.h"
#include "qgsexpressioncontext.h"
#include "qgspallabeling.h"
#include "qgspropertycollection.h"

#include <QString>

using namespace Qt::StringLiterals;

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

    static constexpr auto OVERLAP_HANDLING_MAP = makeDataDefinedStringDecoder<Qgis::LabelOverlapHandling>( {
      { u"prevent", Qgis::LabelOverlapHandling::PreventOverlap },
      { u"allowifneeded", Qgis::LabelOverlapHandling::AllowOverlapIfRequired },
      { u"alwaysallow", Qgis::LabelOverlapHandling::AllowOverlapAtNoCost },
    } );

    auto evaluatedHandling = OVERLAP_HANDLING_MAP.lookup( handlingString );
    if ( evaluatedHandling )
    {
      mOverlapHandling = *evaluatedHandling;
    }
  }

  // this property is messy - to avoid breaking old projects we need to also allow it to be treated as a boolean
  if ( properties.isActive( QgsPalLayerSettings::Property::LabelAllParts ) )
  {
    const QString stringValue = properties.valueAsString( QgsPalLayerSettings::Property::LabelAllParts, context );
    bool handledAsString = false;
    if ( !stringValue.isEmpty() )
    {
      static constexpr auto MULTIPART_HANDLING_MAP = makeDataDefinedStringDecoder<Qgis::MultiPartLabelingBehavior>( {
        { u"largestpartonly", Qgis::MultiPartLabelingBehavior::LabelLargestPartOnly },
        { u"labeleverypart", Qgis::MultiPartLabelingBehavior::LabelEveryPartWithEntireLabel },
        { u"splitlabeltextlinesoverparts", Qgis::MultiPartLabelingBehavior::SplitLabelTextLinesOverParts },
      } );

      auto evaluatedHandling = MULTIPART_HANDLING_MAP.lookup( stringValue );
      if ( evaluatedHandling )
      {
        handledAsString = true;
        mMultiPartBehavior = *evaluatedHandling;
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

  if ( properties.isActive( QgsPalLayerSettings::Property::WhitespaceCollisionHandling ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::WhitespaceCollisionHandling, context, QString(), &ok );
    if ( ok )
    {
      static constexpr auto WHITESPACE_HANDLING_MAP = makeDataDefinedStringDecoder<Qgis::LabelWhitespaceCollisionHandling>(
        { { u"treatwhitespaceascollision", Qgis::LabelWhitespaceCollisionHandling::TreatWhitespaceAsCollision },
          { u"ignorewhitespacecollisions", Qgis::LabelWhitespaceCollisionHandling::IgnoreWhitespaceCollisions } }
      );

      auto evaluatedHandling = WHITESPACE_HANDLING_MAP.lookup( value );
      if ( evaluatedHandling )
      {
        mWhitespaceCollisionHandling = *evaluatedHandling;
      }
    }
  }
}
