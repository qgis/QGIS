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
#include "qgspropertycollection.h"
#include "qgsexpressioncontext.h"
#include "qgslabelingengine.h"
#include "qgspallabeling.h"


void QgsLabelLineSettings::updateDataDefinedProperties( const QgsPropertyCollection &properties, QgsExpressionContext &context )
{
  if ( properties.isActive( QgsPalLayerSettings::LinePlacementOptions ) )
  {
    context.setOriginalValueVariable( QgsLabelingUtils::encodeLinePlacementFlags( mPlacementFlags ) );
    const QString dataDefinedLineArrangement = properties.valueAsString( QgsPalLayerSettings::LinePlacementOptions, context );
    if ( !dataDefinedLineArrangement.isEmpty() )
    {
      mPlacementFlags = QgsLabelingUtils::decodeLinePlacementFlags( dataDefinedLineArrangement );
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::OverrunDistance ) )
  {
    context.setOriginalValueVariable( mOverrunDistance );
    mOverrunDistance = properties.valueAsDouble( QgsPalLayerSettings::OverrunDistance, context, mOverrunDistance );
  }

  if ( properties.isActive( QgsPalLayerSettings::LineAnchorPercent ) )
  {
    context.setOriginalValueVariable( mLineAnchorPercent );
    mLineAnchorPercent = properties.valueAsDouble( QgsPalLayerSettings::LineAnchorPercent, context, mLineAnchorPercent );
  }

  if ( properties.isActive( QgsPalLayerSettings::LineAnchorClipping ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::LineAnchorClipping, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( QLatin1String( "visible" ), Qt::CaseInsensitive ) == 0 )
        mAnchorClipping = AnchorClipping::UseVisiblePartsOfLine;
      else if ( value.compare( QLatin1String( "entire" ), Qt::CaseInsensitive ) == 0 )
        mAnchorClipping = AnchorClipping::UseEntireLine;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::LineAnchorType ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::LineAnchorType, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( QLatin1String( "hint" ), Qt::CaseInsensitive ) == 0 )
        mAnchorType = AnchorType::HintOnly;
      else if ( value.compare( QLatin1String( "strict" ), Qt::CaseInsensitive ) == 0 )
        mAnchorType = AnchorType::Strict;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::LineAnchorTextPoint ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::LineAnchorTextPoint, context, QString(), &ok ).trimmed();
    if ( ok )
    {
      if ( value.compare( QLatin1String( "follow" ), Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::FollowPlacement;
      else if ( value.compare( QLatin1String( "start" ), Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::StartOfText;
      else if ( value.compare( QLatin1String( "center" ), Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::CenterOfText;
      else if ( value.compare( QLatin1String( "end" ), Qt::CaseInsensitive ) == 0 )
        mAnchorTextPoint = AnchorTextPoint::EndOfText;
    }
  }
}
