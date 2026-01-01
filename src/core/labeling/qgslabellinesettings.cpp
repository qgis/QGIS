/***************************************************************************
  qgslabellinesettings.cpp
  ----------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabellinesettings.h"

#include "qgsexpressioncontext.h"
#include "qgslabelingengine.h"
#include "qgspallabeling.h"
#include "qgspropertycollection.h"

#include "moc_qgslabellinesettings.cpp"

void QgsLabelLineSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::LinePlacementOptions ) )
  {
    context.setOriginalValueVariable( QgsLabelingUtils::encodeLinePlacementFlags( mPlacementFlags ) );
    const QString dataDefinedLineArrangement = properties.valueAsString( QgsPalLayerSettings::Property::LinePlacementOptions, context );
    if ( !dataDefinedLineArrangement.isEmpty() )
    {
      mPlacementFlags = QgsLabelingUtils::decodeLinePlacementFlags( dataDefinedLineArrangement );
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::OverrunDistance ) )
  {
    context.setOriginalValueVariable( mOverrunDistance );
    mOverrunDistance = properties.valueAsDouble( QgsPalLayerSettings::Property::OverrunDistance, context, mOverrunDistance );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorPercent ) )
  {
    context.setOriginalValueVariable( mLineAnchorPercent );
    mLineAnchorPercent = properties.valueAsDouble( QgsPalLayerSettings::Property::LineAnchorPercent, context, mLineAnchorPercent );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorClipping ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorClipping, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( "visible"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorClipping = AnchorClipping::UseVisiblePartsOfLine;
      else if ( value.compare( "entire"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorClipping = AnchorClipping::UseEntireLine;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorType ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorType, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( "hint"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorType = AnchorType::HintOnly;
      else if ( value.compare( "strict"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorType = AnchorType::Strict;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorTextPoint ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorTextPoint, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( "follow"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::FollowPlacement;
      else if ( value.compare( "start"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::StartOfText;
      else if ( value.compare( "center"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::CenterOfText;
      else if ( value.compare( "end"_L1, Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::EndOfText;
    }
  }
}
