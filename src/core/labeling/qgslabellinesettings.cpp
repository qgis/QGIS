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

#include "qgsdatadefinedutils.h"
#include "qgsexpressioncontext.h"
#include "qgslabelingengine.h"
#include "qgspallabeling.h"
#include "qgspropertycollection.h"

#include <QString>

#include "moc_qgslabellinesettings.cpp"

using namespace Qt::StringLiterals;

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
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorClipping, context, QString(), &ok );
    if ( ok )
    {
      static constexpr auto CLIPPING_MAP = makeDataDefinedStringDecoder<AnchorClipping>( {
        { u"visible", AnchorClipping::UseVisiblePartsOfLine },
        { u"entire", AnchorClipping::UseEntireLine },
      } );

      auto evaluatedClipping = CLIPPING_MAP.lookup( value );
      if ( evaluatedClipping )
      {
        mAnchorClipping = *evaluatedClipping;
      }
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorType ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorType, context, QString(), &ok );
    if ( ok )
    {
      static constexpr auto ANCHOR_MAP = makeDataDefinedStringDecoder<AnchorType>( {
        { u"hint", AnchorType::HintOnly },
        { u"strict", AnchorType::Strict },
      } );

      auto evaluatedAnchor = ANCHOR_MAP.lookup( value );
      if ( evaluatedAnchor )
      {
        mAnchorType = *evaluatedAnchor;
      }
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::LineAnchorTextPoint ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::LineAnchorTextPoint, context, QString(), &ok );
    if ( ok )
    {
      static constexpr auto ANCHOR_TEXT_MAP = makeDataDefinedStringDecoder<Qgis::TextAnchorPoint>( {
        { u"follow", Qgis::TextAnchorPoint::FollowPlacement },
        { u"start", Qgis::TextAnchorPoint::StartOfText },
        { u"center", Qgis::TextAnchorPoint::CenterOfText },
        { u"end", Qgis::TextAnchorPoint::EndOfText },
      } );
      auto evaluatedAnchor = ANCHOR_TEXT_MAP.lookup( value );
      if ( evaluatedAnchor )
      {
        mAnchorTextPoint = *evaluatedAnchor;
      }
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::CurvedLabelMode ) )
  {
    bool ok = false;
    const QString value = properties.valueAsString( QgsPalLayerSettings::Property::CurvedLabelMode, context, QString(), &ok );
    if ( ok )
    {
      static constexpr auto CURVED_LABEL_MODE_MAP = makeDataDefinedStringDecoder<Qgis::CurvedLabelMode>( {
        { u"default", Qgis::CurvedLabelMode::Default },
        { u"charactersatvertices", Qgis::CurvedLabelMode::PlaceCharactersAtVertices },
        { u"stretchcharacterspacingtofit", Qgis::CurvedLabelMode::StretchCharacterSpacingToFitLine },
        { u"stretchwordspacingtofit", Qgis::CurvedLabelMode::StretchWordSpacingToFitLine },
      } );

      auto evaluatedCurvedMode = CURVED_LABEL_MODE_MAP.lookup( value );
      if ( evaluatedCurvedMode )
      {
        mCurvedLabelMode = *evaluatedCurvedMode;
      }
    }
  }
}
