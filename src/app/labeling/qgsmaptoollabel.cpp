/***************************************************************************
                          qgsmaptoollabel.cpp
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoollabel.h"
#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsdiagramrenderer.h"
#include "qgssettingsregistrycore.h"
#include "qgsauxiliarystorage.h"
#include "qgstextrenderer.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"
#include "qgsstatusbar.h"
#include "qgslabelingresults.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsreferencedgeometry.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgssettingsentryimpl.h"
#include "qgsfontutils.h"

#include <QMouseEvent>

QgsMapToolLabel::QgsMapToolLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolAdvancedDigitizing( canvas, cadDock )
{
}

QgsMapToolLabel::~QgsMapToolLabel()
{
  delete mLabelRubberBand;
  delete mHoverRubberBand;
  delete mCalloutOtherPointsRubberBand;
  delete mFeatureRubberBand;
  delete mFixPointRubberBand;
  delete mOffsetFromLineStartRubberBand;
}

void QgsMapToolLabel::deactivate()
{
  clearHoveredLabel();
  QgsMapToolAdvancedDigitizing::deactivate();
}

bool QgsMapToolLabel::labelAtPosition( QMouseEvent *e, QgsLabelPosition &p )
{
  QgsPointXY pt = toMapCoordinates( e->pos() );
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
    return false;

  QList<QgsLabelPosition> labelPosList = labelingResults->labelsAtPosition( pt );
  labelPosList.erase( std::remove_if( labelPosList.begin(), labelPosList.end(), [this]( const QgsLabelPosition & position )
  {
    if ( position.layerID.isEmpty() )
      return true;

    if ( QgsMapLayer *layer = QgsMapTool::layer( position.layerID ) )
    {
      // strip out any labels from non vector layers (e.g. those from vector tile layers). Only vector layer labels
      // are supported by the map tools.
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
          return false;

        case Qgis::LayerType::Raster:
        case Qgis::LayerType::Plugin:
        case Qgis::LayerType::Mesh:
        case Qgis::LayerType::VectorTile:
        case Qgis::LayerType::Annotation:
        case Qgis::LayerType::PointCloud:
        case Qgis::LayerType::Group:
        case Qgis::LayerType::TiledScene:
          return true;
      }
    }

    return true;
  } ), labelPosList.end() );

  if ( labelPosList.empty() )
    return false;

  // prioritize labels in the current selected layer, in case of overlaps
  QList<QgsLabelPosition> activeLayerLabels;
  if ( const QgsVectorLayer *currentLayer = qobject_cast< QgsVectorLayer * >( mCanvas->currentLayer() ) )
  {
    for ( const QgsLabelPosition &pos : std::as_const( labelPosList ) )
    {
      if ( pos.layerID == currentLayer->id() )
      {
        activeLayerLabels.append( pos );
      }
    }
  }
  if ( !activeLayerLabels.empty() )
    labelPosList = activeLayerLabels;

  // prioritize unplaced labels
  QList<QgsLabelPosition> unplacedLabels;
  for ( const QgsLabelPosition &pos : std::as_const( labelPosList ) )
  {
    if ( pos.isUnplaced )
    {
      unplacedLabels.append( pos );
    }
  }
  if ( !unplacedLabels.empty() )
    labelPosList = unplacedLabels;

  if ( labelPosList.count() > 1 )
  {
    // multiple candidates found, so choose the smallest (i.e. most difficult to select otherwise)
    double minSize = std::numeric_limits< double >::max();
    for ( const QgsLabelPosition &pos : std::as_const( labelPosList ) )
    {
      const double labelSize = pos.width * pos.height;
      if ( labelSize < minSize )
      {
        minSize = labelSize;
        p = pos;
      }
    }
  }
  else
  {
    // only one candidate
    p = labelPosList.at( 0 );
  }

  return true;
}

bool QgsMapToolLabel::calloutAtPosition( QMouseEvent *e, QgsCalloutPosition &p, bool &isOrigin )
{
  QgsPointXY pt = toMapCoordinates( e->pos() );
  const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false );
  if ( !labelingResults )
    return false;

  const double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  QList<QgsCalloutPosition> calloutPosList = labelingResults->calloutsWithinRectangle( QgsRectangle::fromCenterAndSize( pt, tol * 2, tol * 2 ) );
  calloutPosList.erase( std::remove_if( calloutPosList.begin(), calloutPosList.end(), [ this ]( const QgsCalloutPosition & position )
  {
    if ( position.layerID.isEmpty() )
      return true;

    if ( QgsMapLayer *layer = QgsMapTool::layer( position.layerID ) )
    {
      // strip out any callouts from non vector layers (e.g. those from vector tile layers). Only vector layer callouts
      // are supported by the map tools.
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
          return false;

        case Qgis::LayerType::Raster:
        case Qgis::LayerType::Plugin:
        case Qgis::LayerType::Mesh:
        case Qgis::LayerType::VectorTile:
        case Qgis::LayerType::Annotation:
        case Qgis::LayerType::PointCloud:
        case Qgis::LayerType::Group:
        case Qgis::LayerType::TiledScene:
          return true;
      }
    }

    return true;
  } ), calloutPosList.end() );
  if ( calloutPosList.empty() )
    return false;

  // prioritize callouts in the current selected layer, in case of overlaps
  QList<QgsCalloutPosition> activeLayerCallouts;
  if ( const QgsVectorLayer *currentLayer = qobject_cast< QgsVectorLayer * >( mCanvas->currentLayer() ) )
  {
    for ( const QgsCalloutPosition &pos : std::as_const( calloutPosList ) )
    {
      if ( pos.layerID == currentLayer->id() )
      {
        activeLayerCallouts.append( pos );
      }
    }
  }
  if ( !activeLayerCallouts.empty() )
    calloutPosList = activeLayerCallouts;

  p = calloutPosList.at( 0 );

  isOrigin = QgsPointXY( p.origin() ).sqrDist( pt ) < QgsPointXY( p.destination() ).sqrDist( pt );

  return true;
}

void QgsMapToolLabel::createRubberBands()
{
  delete mLabelRubberBand;
  delete mFeatureRubberBand;
  delete mOffsetFromLineStartRubberBand;

  //label rubber band
  mLabelRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Line );
  mLabelRubberBand->addPoint( mCurrentLabel.pos.cornerPoints.at( 0 ) );
  mLabelRubberBand->addPoint( mCurrentLabel.pos.cornerPoints.at( 1 ) );
  mLabelRubberBand->addPoint( mCurrentLabel.pos.cornerPoints.at( 2 ) );
  mLabelRubberBand->addPoint( mCurrentLabel.pos.cornerPoints.at( 3 ) );
  mLabelRubberBand->addPoint( mCurrentLabel.pos.cornerPoints.at( 0 ) );
  mLabelRubberBand->setColor( QColor( 0, 255, 0, 65 ) );
  mLabelRubberBand->setWidth( 3 );
  mLabelRubberBand->show();

  //feature rubber band
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( vlayer )
  {
    QgsFeature f;
    if ( currentFeature( f, true ) )
    {
      QgsGeometry geom = f.geometry();
      if ( !geom.isNull() )
      {
        const QColor lineColor = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();

        if ( geom.type() == Qgis::GeometryType::Polygon )
        {
          // for polygons, we don't want to fill the whole polygon itself with the rubber band
          // as that obscures too much of the map and prevents users from getting a good view of
          // the underlying map
          // instead, just use the boundary of the polygon for the rubber band
          geom = QgsGeometry( geom.constGet()->boundary() );
        }

        if ( geom.type() == Qgis::GeometryType::Line || geom.type() == Qgis::GeometryType::Polygon )
        {
          mOffsetFromLineStartRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Point );
          mOffsetFromLineStartRubberBand->setColor( lineColor );
          mOffsetFromLineStartRubberBand->hide();
        }

        mFeatureRubberBand = new QgsRubberBand( mCanvas, geom.type() );
        mFeatureRubberBand->setColor( lineColor );
        mFeatureRubberBand->setToGeometry( geom, vlayer );
        mFeatureRubberBand->show();
      }
    }

    //fixpoint rubber band
    QgsPointXY fixPoint;
    if ( currentLabelRotationPoint( fixPoint, false ) )
    {
      if ( mCanvas )
      {
        const QgsMapSettings &s = mCanvas->mapSettings();
        fixPoint = s.mapToLayerCoordinates( vlayer, fixPoint );
      }

      QgsGeometry pointGeom = QgsGeometry::fromPointXY( fixPoint );
      mFixPointRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Line );
      mFixPointRubberBand->setColor( QColor( 0, 0, 255, 65 ) );
      mFixPointRubberBand->setToGeometry( pointGeom, vlayer );
      mFixPointRubberBand->show();
    }
  }
}

void QgsMapToolLabel::deleteRubberBands()
{
  delete mLabelRubberBand;
  mLabelRubberBand = nullptr;
  delete mFeatureRubberBand;
  mFeatureRubberBand = nullptr;
  delete mFixPointRubberBand;
  mFixPointRubberBand = nullptr;
  delete mOffsetFromLineStartRubberBand;
  mOffsetFromLineStartRubberBand = nullptr;
  cadDockWidget()->clear();
  cadDockWidget()->clearPoints();
}

QString QgsMapToolLabel::currentLabelText( int trunc )
{
  if ( !mCurrentLabel.valid )
  {
    return QString();
  }
  QgsPalLayerSettings &labelSettings = mCurrentLabel.settings;

  if ( labelSettings.isExpression )
  {
    QString labelText = mCurrentLabel.pos.labelText;

    if ( trunc > 0 && labelText.length() > trunc )
    {
      labelText.truncate( trunc );
      labelText += QChar( 0x2026 );
    }
    return labelText;
  }
  else
  {
    QgsVectorLayer *vlayer = mCurrentLabel.layer;
    if ( !vlayer )
    {
      return QString();
    }

    QString labelField = labelSettings.fieldName;
    if ( !labelField.isEmpty() )
    {
      int labelFieldId = vlayer->fields().lookupField( labelField );
      QgsFeature f;
      if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabel.pos.featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
      {
        QString labelText = f.attribute( labelFieldId ).toString();
        if ( trunc > 0 && labelText.length() > trunc )
        {
          labelText.truncate( trunc );
          labelText += QChar( 0x2026 );
        }
        return labelText;
      }
    }
  }
  return QString();
}

QgsMapToolLabel::LabelAlignment QgsMapToolLabel::currentAlignment()
{
  LabelAlignment labelAlignment = LabelAlignment::BottomLeft;

  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return labelAlignment;
  }

  QgsFeature f;
  if ( !currentFeature( f ) )
  {
    return labelAlignment;
  }

  // data defined quadrant offset
  if ( mCurrentLabel.settings.placement == Qgis::LabelPlacement::AroundPoint ||
       mCurrentLabel.settings.placement == Qgis::LabelPlacement::OverPoint )
  {
    Qgis::LabelQuadrantPosition quadrantOffset = Qgis::LabelQuadrantPosition::AboveRight;

    // quadrant offset defined via buttons
    if ( mCurrentLabel.settings.placement == Qgis::LabelPlacement::OverPoint )
      quadrantOffset = mCurrentLabel.settings.pointSettings().quadrant();

    // quadrant offset DD defined
    if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::OffsetQuad ) )
    {
      QVariant exprVal = evaluateDataDefinedProperty( QgsPalLayerSettings::Property::OffsetQuad, mCurrentLabel.settings, f, static_cast< int >( quadrantOffset ) );
      if ( !QgsVariantUtils::isNull( exprVal ) )
      {
        bool ok;
        int quadInt = exprVal.toInt( &ok );
        if ( ok && 0 <= quadInt && quadInt <= 8 )
        {
          quadrantOffset = static_cast< Qgis::LabelQuadrantPosition >( quadInt );
        }
      }
    }

    switch ( quadrantOffset )
    {
      case Qgis::LabelQuadrantPosition::AboveLeft:
        labelAlignment = LabelAlignment::BottomRight;
        break;
      case Qgis::LabelQuadrantPosition::Above:
        labelAlignment = LabelAlignment::BottomCenter;
        break;
      case Qgis::LabelQuadrantPosition::AboveRight:
        labelAlignment = LabelAlignment::BottomLeft;
        break;
      case Qgis::LabelQuadrantPosition::Left:
        labelAlignment = LabelAlignment::HalfRight;
        break;
      case Qgis::LabelQuadrantPosition::Over:
        labelAlignment = LabelAlignment::HalfCenter;
        break;
      case Qgis::LabelQuadrantPosition::Right:
        labelAlignment = LabelAlignment::HalfLeft;
        break;
      case Qgis::LabelQuadrantPosition::BelowLeft:
        labelAlignment = LabelAlignment::TopRight;
        break;
      case Qgis::LabelQuadrantPosition::Below:
        labelAlignment = LabelAlignment::TopCenter;
        break;
      case Qgis::LabelQuadrantPosition::BelowRight:
        labelAlignment = LabelAlignment::TopLeft;
        break;
    }
  }

  // quadrant defined by DD alignment
  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::Hali ) ||
       mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::Vali ) )
  {
    QString hali = QStringLiteral( "Left" );
    QString vali = QStringLiteral( "Bottom" );
    hali = evaluateDataDefinedProperty( QgsPalLayerSettings::Property::Hali, mCurrentLabel.settings, f, hali ).toString();
    vali = evaluateDataDefinedProperty( QgsPalLayerSettings::Property::Vali, mCurrentLabel.settings, f, vali ).toString();

    if ( hali.compare( QLatin1String( "Left" ), Qt::CaseInsensitive ) == 0 )
    {
      if ( vali.compare( QLatin1String( "Top" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::TopLeft;
      else if ( vali.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::HalfLeft;
      else if ( vali.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BottomLeft;
      else if ( vali.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BaseLeft;
      else if ( vali.compare( QLatin1String( "Cap" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::CapLeft;
    }
    else if ( hali.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
    {
      if ( vali.compare( QLatin1String( "Top" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::TopCenter;
      else if ( vali.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::HalfCenter;
      else if ( vali.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BottomCenter;
      else if ( vali.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BaseCenter;
      else if ( vali.compare( QLatin1String( "Cap" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::CapCenter;
    }
    else if ( hali.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
    {
      if ( vali.compare( QLatin1String( "Top" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::TopRight;
      else if ( vali.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::HalfRight;
      else if ( vali.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BottomRight;
      else if ( vali.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::BaseRight;
      else if ( vali.compare( QLatin1String( "Cap" ), Qt::CaseInsensitive ) == 0 )
        labelAlignment = LabelAlignment::CapRight;
    }
  }

  return labelAlignment;
}

bool QgsMapToolLabel::currentFeature( QgsFeature &f, bool fetchGeom )
{
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return false;
  }
  return vlayer->getFeatures( QgsFeatureRequest()
                              .setFilterFid( mCurrentLabel.pos.featureId )
                              .setFlags( fetchGeom ? Qgis::FeatureRequestFlag::NoFlags : Qgis::FeatureRequestFlag::NoGeometry )
                            ).nextFeature( f );
}

QFont QgsMapToolLabel::currentLabelFont()
{
  QFont font;

  QgsPalLayerSettings &labelSettings = mCurrentLabel.settings;
  QgsVectorLayer *vlayer = mCurrentLabel.layer;

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  if ( mCurrentLabel.valid && vlayer )
  {
    font = labelSettings.format().font();

    QgsFeature f;
    if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabel.pos.featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
    {
      //size
      int sizeIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Size, mCurrentLabel.settings, vlayer );
      if ( sizeIndx != -1 )
      {
        font.setPixelSize( QgsTextRenderer::sizeToPixel( f.attribute( sizeIndx ).toDouble(),
                           context, labelSettings.format().sizeUnit(),
                           labelSettings.format().sizeMapUnitScale() ) );
      }

      //family
      int fmIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Family, labelSettings, vlayer );
      if ( fmIndx != -1 )
      {
        QgsFontUtils::setFontFamily( font, f.attribute( fmIndx ).toString() );
      }

      //underline
      int ulIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Underline, labelSettings, vlayer );
      if ( ulIndx != -1 )
      {
        font.setUnderline( f.attribute( ulIndx ).toBool() );
      }

      //strikeout
      int soIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Strikeout, labelSettings, vlayer );
      if ( soIndx != -1 )
      {
        font.setStrikeOut( f.attribute( soIndx ).toBool() );
      }

      //bold
      int boIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Bold, labelSettings, vlayer );
      if ( boIndx != -1 )
      {
        font.setBold( f.attribute( boIndx ).toBool() );
      }

      //italic
      int itIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Property::Italic, labelSettings, vlayer );
      if ( itIndx != -1 )
      {
        font.setItalic( f.attribute( itIndx ).toBool() );
      }

      // TODO: Add other font data defined values (word spacing, etc.)
    }
  }

  return font;
}

bool QgsMapToolLabel::currentLabelPreserveRotation()
{
  if ( mCurrentLabel.valid )
  {
    return mCurrentLabel.settings.preserveRotation;
  }

  return true; // default, so there is no accidental data loss
}

bool QgsMapToolLabel::currentLabelRotationPoint( QgsPointXY &pos, bool ignoreUpsideDown )
{
  QVector<QgsPointXY> cornerPoints = mCurrentLabel.pos.cornerPoints;
  if ( cornerPoints.size() < 4 )
  {
    return false;
  }

  if ( mCurrentLabel.pos.upsideDown && !ignoreUpsideDown )
  {
    pos = cornerPoints.at( 2 );
  }
  else
  {
    pos = cornerPoints.at( 0 );
  }

  //alignment always center/center and rotation 0 for diagrams
  if ( mCurrentLabel.pos.isDiagram )
  {
    pos.setX( pos.x() + mCurrentLabel.pos.labelRect.width() / 2.0 );
    pos.setY( pos.y() + mCurrentLabel.pos.labelRect.height() / 2.0 );
    return true;
  }

  //adapt pos depending on data defined alignment
  LabelAlignment alignment = currentAlignment();

  QFontMetricsF labelFontMetrics( mCurrentLabel.pos.labelFont );

  // NOTE: this assumes the label corner points comprise a rectangle and that the
  //       CRS supports equidistant measurements to accurately determine hypotenuse
  QgsPointXY cp_0 = cornerPoints.at( 0 );
  QgsPointXY cp_1 = cornerPoints.at( 1 );
  QgsPointXY cp_3 = cornerPoints.at( 3 );
  //  QgsDebugMsgLevel( QStringLiteral( "cp_0: x=%1, y=%2" ).arg( cp_0.x() ).arg( cp_0.y() ), 2 );
  //  QgsDebugMsgLevel( QStringLiteral( "cp_1: x=%1, y=%2" ).arg( cp_1.x() ).arg( cp_1.y() ), 2 );
  //  QgsDebugMsgLevel( QStringLiteral( "cp_3: x=%1, y=%2" ).arg( cp_3.x() ).arg( cp_3.y() ), 2 );
  double labelSizeX = std::sqrt( cp_0.sqrDist( cp_1 ) );
  double labelSizeY = std::sqrt( cp_0.sqrDist( cp_3 ) );

  // X diff
  double xdiff = 0.0;
  switch ( alignment )
  {
    case LabelAlignment::BottomRight:
    case LabelAlignment::HalfRight:
    case LabelAlignment::TopRight:
    case LabelAlignment::BaseRight:
    case LabelAlignment::CapRight:
      xdiff = labelSizeX;
      break;
    case LabelAlignment::BottomCenter:
    case LabelAlignment::HalfCenter:
    case LabelAlignment::TopCenter:
    case LabelAlignment::BaseCenter:
    case LabelAlignment::CapCenter:
      xdiff = labelSizeX / 2.0;
      break;
    case LabelAlignment::BottomLeft:
    case LabelAlignment::HalfLeft:
    case LabelAlignment::TopLeft:
    case LabelAlignment::BaseLeft:
    case LabelAlignment::CapLeft:
      // Do nothing
      break;
  }

  // Y diff
  double ydiff = 0;
  double descentRatio = 1.0 / labelFontMetrics.ascent() / labelFontMetrics.height();
  double capHeightRatio = ( labelFontMetrics.boundingRect( 'H' ).height() + 1 + labelFontMetrics.descent() ) / labelFontMetrics.height();
  switch ( alignment )
  {
    case LabelAlignment::BottomRight:
    case LabelAlignment::BottomCenter:
    case LabelAlignment::BottomLeft:
      // Do nothing
      break;
    case LabelAlignment::HalfRight:
    case LabelAlignment::HalfCenter:
    case LabelAlignment::HalfLeft:
      ydiff = labelSizeY * 0.5 * ( capHeightRatio - descentRatio );
      break;
    case LabelAlignment::TopRight:
    case LabelAlignment::TopCenter:
    case LabelAlignment::TopLeft:
      ydiff = labelSizeY;
      break;
    case LabelAlignment::BaseRight:
    case LabelAlignment::BaseCenter:
    case LabelAlignment::BaseLeft:
      ydiff = labelSizeY * descentRatio;
      break;
    case LabelAlignment::CapRight:
    case LabelAlignment::CapCenter:
    case LabelAlignment::CapLeft:
      ydiff = labelSizeY * capHeightRatio;
      break;
  }

  double angle = mCurrentLabel.pos.rotation * M_PI / 180;
  double xd = xdiff * std::cos( angle ) - ydiff * std::sin( angle );
  double yd = xdiff * std::sin( angle ) + ydiff * std::cos( angle );
  if ( mCurrentLabel.pos.upsideDown && !ignoreUpsideDown )
  {
    pos.setX( pos.x() - xd );
    pos.setY( pos.y() - yd );
  }
  else
  {
    pos.setX( pos.x() + xd );
    pos.setY( pos.y() + yd );
  }
  return true;
}

#if 0
bool QgsMapToolLabel::hasDataDefinedColumn( QgsPalLayerSettings::Property::DataDefinedProperties p, QgsVectorLayer *vlayer ) const
{
  const auto constSubProviders = vlayer->labeling()->subProviders();
  for ( const QString &providerId : constSubProviders )
  {
    if ( QgsPalLayerSettings *settings = vlayer->labeling()->settings( vlayer, providerId ) )
    {
      QString fieldname = dataDefinedColumnName( p, *settings );
      if ( !fieldname.isEmpty() )
        return true;
    }
  }
  return false;
}
#endif

QString QgsMapToolLabel::dataDefinedColumnName( QgsPalLayerSettings::Property p, const QgsPalLayerSettings &labelSettings, const QgsVectorLayer *layer, PropertyStatus &status ) const
{
  status = PropertyStatus::DoesNotExist;
  if ( !labelSettings.dataDefinedProperties().isActive( p ) )
    return QString();

  const QgsProperty property = labelSettings.dataDefinedProperties().property( p );

  switch ( property.propertyType() )
  {
    case Qgis::PropertyType::Invalid:
      break;

    case Qgis::PropertyType::Static:
      status = PropertyStatus::Valid;
      break;

    case Qgis::PropertyType::Field:
      status = PropertyStatus::Valid;
      return property.field();

    case Qgis::PropertyType::Expression:
    {
      status = PropertyStatus::Valid;

      // an expression based property may still be a effectively a single field reference in the map canvas context.
      // e.g. if it is a expression like '"some_field"', or 'case when @some_project_var = 'a' then "field_a" else "field_b" end'

      QgsExpressionContext context = mCanvas->createExpressionContext();
      context.appendScope( layer->createExpressionContextScope() );

      QgsExpression expression( property.expressionString() );
      if ( expression.prepare( &context ) )
      {
        // maybe the expression is effectively a single node in this context...
        const QgsExpressionNode *node = expression.rootNode()->effectiveNode();
        if ( node->nodeType() == QgsExpressionNode::ntColumnRef )
        {
          const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( node );
          return columnRef->name();
        }

        // ok, it's not. But let's be super smart and helpful for users!
        // maybe it's a COALESCE("some field", 'some' || 'fallback' || 'expression') type expression, where the user wants to override
        // some labels with a value stored in a field but all others use some expression
        if ( node->nodeType() == QgsExpressionNode::ntFunction )
        {
          const QgsExpressionNodeFunction *functionNode = qgis::down_cast<const QgsExpressionNodeFunction *>( node );
          if ( const QgsExpressionFunction *function = QgsExpression::QgsExpression::Functions()[functionNode->fnIndex()] )
          {
            if ( function->name() == QLatin1String( "coalesce" ) )
            {
              if ( const QgsExpressionNode *firstArg = functionNode->args()->list().value( 0 ) )
              {
                const QgsExpressionNode *firstArgNode = firstArg->effectiveNode();
                if ( firstArgNode->nodeType() == QgsExpressionNode::ntColumnRef )
                {
                  const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( firstArgNode );
                  return columnRef->name();
                }
              }
            }
          }
        }

      }
      else
      {
        status = PropertyStatus::CurrentExpressionInvalid;
      }
      break;
    }
  }

  return QString();
}

int QgsMapToolLabel::dataDefinedColumnIndex( QgsPalLayerSettings::Property p, const QgsPalLayerSettings &labelSettings, const QgsVectorLayer *vlayer ) const
{
  PropertyStatus status = PropertyStatus::DoesNotExist;
  QString fieldname = dataDefinedColumnName( p, labelSettings, vlayer, status );
  if ( !fieldname.isEmpty() )
    return vlayer->fields().lookupField( fieldname );
  return -1;
}

QVariant QgsMapToolLabel::evaluateDataDefinedProperty( QgsPalLayerSettings::Property property, const QgsPalLayerSettings &labelSettings, const QgsFeature &feature, const QVariant &defaultValue ) const
{
  QgsExpressionContext context = mCanvas->mapSettings().expressionContext();
  context.setFeature( feature );
  context.setFields( feature.fields() );
  return labelSettings.dataDefinedProperties().value( property, context, defaultValue );
}

bool QgsMapToolLabel::currentLabelDataDefinedPosition( double &x, bool &xSuccess, double &y, bool &ySuccess, int &xCol, int &yCol, int &pointCol ) const
{
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  QgsFeatureId featureId = mCurrentLabel.pos.featureId;

  xSuccess = false;
  ySuccess = false;

  if ( !vlayer )
  {
    return false;
  }

  if ( mCurrentLabel.pos.isDiagram )
  {
    if ( !diagramMoveable( vlayer, xCol, yCol ) )
    {
      return false;
    }
  }
  else if ( !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol, pointCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  if ( !mCurrentLabel.pos.isUnplaced )
  {
    QgsAttributes attributes = f.attributes();

    if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionPoint ) )
    {
      if ( pointCol >= 0
           && !QgsVariantUtils::isNull( attributes.at( pointCol ) ) )
      {
        QVariant pointAsVariant = attributes.at( pointCol );
        if ( pointAsVariant.userType() == QMetaType::type( "QgsGeometry" ) )
        {
          const  QgsGeometry geometry = pointAsVariant.value<QgsGeometry>();
          if ( const QgsPoint *point  = ( geometry.constGet() ? qgsgeometry_cast<const QgsPoint *>( geometry.constGet()->simplifiedTypeRef() ) : nullptr ) )
          {
            x = point->x();
            y = point->y();

            xSuccess = true;
            ySuccess = true;
          }
        }
      }
    }
    else
    {
      if ( !QgsVariantUtils::isNull( attributes.at( xCol ) ) )
        x = attributes.at( xCol ).toDouble( &xSuccess );
      if ( !QgsVariantUtils::isNull( attributes.at( yCol ) ) )
        y = attributes.at( yCol ).toDouble( &ySuccess );
    }
  }

  return true;
}

bool QgsMapToolLabel::currentLabelDataDefinedLineAnchorPercent( double &lineAnchorPercent, bool &lineAnchorPercentSuccess, int &lineAnchorPercentCol,
    QString &lineAnchorClipping, bool &lineAnchorClippingSuccess, int &lineAnchorClippingCol,
    QString &lineAnchorType, bool &lineAnchorTypeSuccess, int &lineAnchorTypeCol,
    QString &lineAnchorTextPoint, bool &lineAnchorTextPointSuccess, int &lineAnchorTextPointCol ) const
{

  lineAnchorPercentSuccess = false;
  lineAnchorClippingSuccess = true;
  lineAnchorTypeSuccess = true;
  lineAnchorTextPointSuccess = true;
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  QgsFeatureId featureId = mCurrentLabel.pos.featureId;

  if ( ! vlayer )
  {
    return false;
  }

  if ( !labelAnchorPercentMovable( vlayer, mCurrentLabel.settings, lineAnchorPercentCol, lineAnchorClippingCol, lineAnchorTypeCol, lineAnchorTextPointCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  QgsAttributes attributes = f.attributes();

  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorPercent ) )
  {
    if ( !QgsVariantUtils::isNull( attributes.at( lineAnchorPercentCol ) ) )
    {
      lineAnchorPercent = attributes.at( lineAnchorPercentCol ).toDouble( &lineAnchorPercentSuccess );
    }
  }

  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorClipping ) )
  {
    if ( !QgsVariantUtils::isNull( attributes.at( lineAnchorClippingCol ) ) )
    {
      lineAnchorClipping = attributes.at( lineAnchorClippingCol ).toString();
    }
    else
    {
      lineAnchorClippingSuccess = false;
    }
  }

  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorType ) )
  {
    if ( !QgsVariantUtils::isNull( attributes.at( lineAnchorTypeCol ) ) )
    {
      lineAnchorType = attributes.at( lineAnchorTypeCol ).toString();
    }
    else
    {
      lineAnchorTypeSuccess = false;
    }
  }

  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorTextPoint ) )
  {
    if ( !QgsVariantUtils::isNull( attributes.at( lineAnchorTextPointCol ) ) )
    {
      lineAnchorTextPoint = attributes.at( lineAnchorTextPointCol ).toString();
    }
    else
    {
      lineAnchorTextPointSuccess = false;
    }
  }

  return true;
}

QgsMapToolLabel::PropertyStatus QgsMapToolLabel::labelRotatableStatus( QgsVectorLayer *layer, const QgsPalLayerSettings &settings, int &rotationCol ) const
{
  PropertyStatus status = PropertyStatus::DoesNotExist;
  QString rColName = dataDefinedColumnName( QgsPalLayerSettings::Property::LabelRotation, settings, layer, status );
  if ( status == PropertyStatus::CurrentExpressionInvalid )
    return status;

  rotationCol = layer->fields().lookupField( rColName );
  return rotationCol != -1 ? PropertyStatus::Valid : PropertyStatus::DoesNotExist;
}

bool QgsMapToolLabel::currentLabelDataDefinedRotation( double &rotation, bool &rotationSuccess, int &rCol, bool ignoreXY ) const
{
  QgsVectorLayer *vlayer = mCurrentLabel.layer;
  QgsFeatureId featureId = mCurrentLabel.pos.featureId;

  rotationSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  if ( labelRotatableStatus( vlayer, mCurrentLabel.settings, rCol ) != PropertyStatus::Valid )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  //test, if data defined x- and y- values are not null. Otherwise, the position is determined by PAL and the rotation cannot be fixed
  if ( !ignoreXY )
  {
    int xCol, yCol, pointCol;
    double x, y;
    bool xSuccess, ySuccess;
    if ( !currentLabelDataDefinedPosition( x, xSuccess, y, ySuccess, xCol, yCol, pointCol ) || !xSuccess || !ySuccess )
    {
      return false;
    }
  }

  rotation = f.attribute( rCol ).toDouble( &rotationSuccess );
  return true;
}

bool QgsMapToolLabel::changeCurrentLabelDataDefinedPosition( const QVariant &x, const QVariant &y )
{
  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionPoint ) )
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    QString pointColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionPoint, mCurrentLabel.settings, mCurrentLabel.layer, status );
    int pointCol = mCurrentLabel.layer->fields().lookupField( pointColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, pointCol, QVariant::fromValue( QgsReferencedGeometry( QgsGeometry::fromPointXY( QgsPoint( x.toDouble(), y.toDouble() ) ), mCurrentLabel.layer->crs() ) ) ) )
      return false;
  }
  else
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    QString xColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, mCurrentLabel.settings, mCurrentLabel.layer, status );
    QString yColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, mCurrentLabel.settings, mCurrentLabel.layer, status );
    int xCol = mCurrentLabel.layer->fields().lookupField( xColName );
    int yCol = mCurrentLabel.layer->fields().lookupField( yColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, xCol, x )
         || !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, yCol, y ) )
      return false;
  }

  return true;
}

bool QgsMapToolLabel::changeCurrentLabelDataDefinedLineAnchorPercent( const QVariant &lineAnchorPercent )
{
  if ( mCurrentLabel.settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::LineAnchorPercent ) )
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    const QString lineAnchorPercentColName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorPercent, mCurrentLabel.settings, mCurrentLabel.layer, status );
    const int lineAnchorPercentCol = mCurrentLabel.layer->fields().lookupField( lineAnchorPercentColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, lineAnchorPercentCol, lineAnchorPercent ) )
      return false;

    // Also set the other anchor properties
    const QString lineAnchorClippingColName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorClipping, mCurrentLabel.settings, mCurrentLabel.layer, status );
    const int lineAnchorClippingCol = mCurrentLabel.layer->fields().lookupField( lineAnchorClippingColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, lineAnchorClippingCol, QStringLiteral( "entire" ) ) )
      return false;

    const QString lineAnchorTypeColName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorType, mCurrentLabel.settings, mCurrentLabel.layer, status );
    const int lineAnchorTypeCol = mCurrentLabel.layer->fields().lookupField( lineAnchorTypeColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, lineAnchorTypeCol, QStringLiteral( "strict" ) ) )
      return false;

    const QString lineAnchorTextPointColName = dataDefinedColumnName( QgsPalLayerSettings::Property::LineAnchorTextPoint, mCurrentLabel.settings, mCurrentLabel.layer, status );
    const int lineAnchorTextPointCol = mCurrentLabel.layer->fields().lookupField( lineAnchorTextPointColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, lineAnchorTextPointCol, QStringLiteral( "start" ) ) )
      return false;

  }
  else
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    const QString lineAnchorPercentColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, mCurrentLabel.settings, mCurrentLabel.layer, status );
    const int lineAnchorPercentCol = mCurrentLabel.layer->fields().lookupField( lineAnchorPercentColName );

    if ( !mCurrentLabel.layer->changeAttributeValue( mCurrentLabel.pos.featureId, lineAnchorPercentCol, lineAnchorPercent ) )
      return false;
  }

  return true;
}

bool QgsMapToolLabel::dataDefinedShowHide( QgsVectorLayer *vlayer, QgsFeatureId featureId, int &show, bool &showSuccess, int &showCol ) const
{
  showSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  if ( mCurrentLabel.pos.isDiagram )
  {
    if ( ! diagramCanShowHide( vlayer, showCol ) )
    {
      return false;
    }
  }
  else if ( ! labelCanShowHide( vlayer, showCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( Qgis::FeatureRequestFlag::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  show = f.attribute( showCol ).toInt( &showSuccess );
  return true;
}

bool QgsMapToolLabel::diagramMoveable( QgsVectorLayer *vlayer, int &xCol, int &yCol ) const
{
  if ( vlayer && vlayer->diagramsEnabled() )
  {
    const QgsDiagramLayerSettings *dls = vlayer->diagramLayerSettings();
    if ( dls )
    {
      xCol = -1;
      if ( QgsProperty ddX = dls->dataDefinedProperties().property( QgsDiagramLayerSettings::Property::PositionX ) )
      {
        if ( ddX.propertyType() == Qgis::PropertyType::Field && ddX.isActive() )
        {
          xCol = vlayer->fields().lookupField( ddX.field() );
        }
      }
      yCol = -1;
      if ( QgsProperty ddY = dls->dataDefinedProperties().property( QgsDiagramLayerSettings::Property::PositionY ) )
      {
        if ( ddY.propertyType() == Qgis::PropertyType::Field && ddY.isActive() )
        {
          yCol = vlayer->fields().lookupField( ddY.field() );
        }
      }
      return xCol >= 0 && yCol >= 0;
    }
  }
  return false;
}

bool QgsMapToolLabel::labelCanShowHide( QgsVectorLayer *vlayer, int &showCol ) const
{
  if ( !vlayer || !vlayer->isEditable() || !vlayer->labelsEnabled() )
  {
    return false;
  }

  const auto constSubProviders = vlayer->labeling()->subProviders();
  PropertyStatus status = PropertyStatus::DoesNotExist;
  for ( const QString &providerId : constSubProviders )
  {
    QString fieldname = dataDefinedColumnName( QgsPalLayerSettings::Property::Show,
                        vlayer->labeling()->settings( providerId ), vlayer, status );
    showCol = vlayer->fields().lookupField( fieldname );
    if ( showCol != -1 )
      return true;
  }

  return false;
}

bool QgsMapToolLabel::isPinned()
{
  bool rc = false;

  if ( ! mCurrentLabel.pos.isDiagram )
  {
    if ( mCurrentLabel.pos.isPinned )
    {
      rc = true;
    }
    else
    {
      double lineAnchor;
      bool lineAnchorSuccess;
      int lineAnchorCol;
      QString lineAnchorClipping;
      bool lineAnchorClippingSuccess;
      int lineAnchorClippingCol;
      QString lineAnchorType;
      bool lineAnchorTypeSuccess;
      int lineAnchorTypeCol;
      QString lineAnchorTextPoint;
      bool lineAnchorTextSuccess;
      int lineAnchorTextCol;
      if ( currentLabelDataDefinedLineAnchorPercent( lineAnchor, lineAnchorSuccess, lineAnchorCol, lineAnchorClipping, lineAnchorClippingSuccess, lineAnchorClippingCol,
           lineAnchorType, lineAnchorTypeSuccess, lineAnchorTypeCol, lineAnchorTextPoint, lineAnchorTextSuccess, lineAnchorTextCol ) )
      {
        rc = lineAnchorSuccess;
      }
    }
  }
  else
  {
    // for diagrams, the isPinned attribute is not set. So we check directly if
    // there's data defined.
    int xCol, yCol, pointCol;
    double x, y;
    bool xSuccess, ySuccess;

    if ( currentLabelDataDefinedPosition( x, xSuccess, y, ySuccess, xCol, yCol, pointCol ) )
      rc = true;
  }

  return rc;
}

bool QgsMapToolLabel::labelMoveable( QgsVectorLayer *vlayer, const QgsPalLayerSettings &settings, int &xCol, int &yCol, int &pointCol ) const
{
  xCol = -1;
  yCol = -1;
  pointCol = -1;

  if ( settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionPoint ) )
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    QString pointColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionPoint, settings, vlayer, status );
    pointCol = vlayer->fields().lookupField( pointColName );
    if ( pointCol >= 0 )
      return true;
  }

  if ( settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionX )
       && settings.dataDefinedProperties().isActive( QgsPalLayerSettings::Property::PositionY ) )
  {
    PropertyStatus status = PropertyStatus::DoesNotExist;
    QString xColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionX, settings, vlayer, status );
    QString yColName = dataDefinedColumnName( QgsPalLayerSettings::Property::PositionY, settings, vlayer, status );
    xCol = vlayer->fields().lookupField( xColName );
    yCol = vlayer->fields().lookupField( yColName );
    if ( xCol >= 0 || yCol >= 0 )
      return true;
  }

  return false;
}

bool QgsMapToolLabel::labelAnchorPercentMovable( QgsVectorLayer *vlayer, const QgsPalLayerSettings &settings, int &lineAnchorPercentCol, int &lineAnchorClippingCol,  int &lineAnchorTypeCol,  int &lineAnchorTextPointCol ) const
{

  auto checkProperty = [ & ]( const QgsPalLayerSettings::Property & property, int &col ) -> bool
  {
    if ( settings.dataDefinedProperties().isActive( property ) )
    {
      PropertyStatus status = PropertyStatus::DoesNotExist;
      QString colName = dataDefinedColumnName( property, settings, vlayer, status );
      col = vlayer->fields().lookupField( colName );
      return col >= 0;
    }
    else
    {
      return false;
    }
  };

  return checkProperty( QgsPalLayerSettings::Property::LineAnchorPercent, lineAnchorPercentCol )
         && checkProperty( QgsPalLayerSettings::Property::LineAnchorClipping, lineAnchorClippingCol )
         && checkProperty( QgsPalLayerSettings::Property::LineAnchorType, lineAnchorTypeCol )
         && checkProperty( QgsPalLayerSettings::Property::LineAnchorTextPoint, lineAnchorTextPointCol );
}

bool QgsMapToolLabel::diagramCanShowHide( QgsVectorLayer *vlayer, int &showCol ) const
{
  showCol = -1;

  if ( vlayer && vlayer->isEditable() && vlayer->diagramsEnabled() )
  {
    if ( const QgsDiagramLayerSettings *dls = vlayer->diagramLayerSettings() )
    {
      if ( QgsProperty ddShow = dls->dataDefinedProperties().property( QgsDiagramLayerSettings::Property::Show ) )
      {
        if ( ddShow.propertyType() == Qgis::PropertyType::Field && ddShow.isActive() )
        {
          showCol = vlayer->fields().lookupField( ddShow.field() );
        }
      }
    }
  }

  return showCol >= 0;
}

//

QgsMapToolLabel::LabelDetails::LabelDetails( const QgsLabelPosition &p, QgsMapCanvas *canvas )
  : pos( p )
{
  layer = qobject_cast< QgsVectorLayer * >( canvas->layer( pos.layerID ) );
  if ( layer && layer->labelsEnabled() && !p.isDiagram )
  {
    settings = layer->labeling()->settings( pos.providerID );
    valid = true;
  }
  else if ( layer && layer->diagramsEnabled() && p.isDiagram )
  {
    valid = true;
  }

  if ( !valid )
  {
    layer = nullptr;
    settings = QgsPalLayerSettings();
  }
}

bool QgsMapToolLabel::createAuxiliaryFields( QgsPalIndexes &indexes )
{
  return createAuxiliaryFields( mCurrentLabel, indexes );
}

bool QgsMapToolLabel::createAuxiliaryFields( LabelDetails &details, QgsPalIndexes &indexes ) const
{
  bool newAuxiliaryLayer = false;
  QgsVectorLayer *vlayer = details.layer;
  QString providerId = details.pos.providerID;

  if ( !vlayer || !vlayer->labelsEnabled() )
    return false;

  if ( !vlayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( vlayer );
    dlg.exec();
    newAuxiliaryLayer = true;
  }

  if ( !vlayer->auxiliaryLayer() )
    return false;

  QgsTemporaryCursorOverride cursor( Qt::WaitCursor );

  bool changed = false;

  for ( const QgsPalLayerSettings::Property &p : std::as_const( mPalProperties ) )
  {
    int index = -1;

    // always use the default activated property
    QgsProperty prop = details.settings.dataDefinedProperties().property( p );
    if ( prop.propertyType() == Qgis::PropertyType::Field && prop.isActive() )
    {
      index = vlayer->fields().lookupField( prop.field() );
    }
    else
    {
      index = QgsAuxiliaryLayer::createProperty( p, vlayer, false );
      changed = true;
    }

    indexes[p] = index;
  }

  // Anchor properties are for linestrings and polygons only:
  if ( vlayer->geometryType() == Qgis::GeometryType::Line ||
       vlayer->geometryType() == Qgis::GeometryType::Polygon )
  {
    for ( const QgsPalLayerSettings::Property &p : std::as_const( mPalAnchorProperties ) )
    {
      int index = -1;

      // always use the default activated property
      QgsProperty prop = details.settings.dataDefinedProperties().property( p );
      if ( prop.propertyType() == Qgis::PropertyType::Field && prop.isActive() )
      {
        index = vlayer->fields().lookupField( prop.field() );
      }
      else
      {
        index = QgsAuxiliaryLayer::createProperty( p, vlayer, false );
        changed = true;
      }

      indexes[p] = index;
    }
  }

  if ( changed )
    emit vlayer->styleChanged();

  details.settings = vlayer->labeling()->settings( providerId );

  return newAuxiliaryLayer;
}

bool QgsMapToolLabel::createAuxiliaryFields( QgsDiagramIndexes &indexes )
{
  return createAuxiliaryFields( mCurrentLabel, indexes );
}

bool QgsMapToolLabel::createAuxiliaryFields( LabelDetails &details, QgsDiagramIndexes &indexes )
{
  bool newAuxiliaryLayer = false;
  QgsVectorLayer *vlayer = details.layer;

  if ( !vlayer )
    return newAuxiliaryLayer;

  if ( !vlayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( vlayer );
    dlg.exec();
    newAuxiliaryLayer = true;
  }

  if ( !vlayer->auxiliaryLayer() )
    return false;

  QgsTemporaryCursorOverride cursor( Qt::WaitCursor );
  bool changed = false;
  for ( const QgsDiagramLayerSettings::Property &p : std::as_const( mDiagramProperties ) )
  {
    int index = -1;

    // always use the default activated property
    QgsProperty prop = vlayer->diagramLayerSettings()->dataDefinedProperties().property( p );
    if ( prop.propertyType() == Qgis::PropertyType::Field && prop.isActive() )
    {
      index = vlayer->fields().lookupField( prop.field() );
    }
    else
    {
      index = QgsAuxiliaryLayer::createProperty( p, vlayer, false );
      changed = true;
    }

    indexes[p] = index;
  }
  if ( changed )
    emit vlayer->styleChanged();

  return newAuxiliaryLayer;
}

bool QgsMapToolLabel::createAuxiliaryFields( QgsCalloutIndexes &calloutIndexes )
{
  return createAuxiliaryFields( mCurrentCallout, calloutIndexes );
}

bool QgsMapToolLabel::createAuxiliaryFields( QgsCalloutPosition &details, QgsCalloutIndexes &calloutIndexes )
{
  bool newAuxiliaryLayer = false;
  QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( QgsMapTool::layer( details.layerID ) );

  if ( !vlayer )
    return newAuxiliaryLayer;

  if ( !vlayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( vlayer );
    dlg.exec();
    newAuxiliaryLayer = true;
  }

  if ( !vlayer->auxiliaryLayer() )
    return false;

  QgsTemporaryCursorOverride cursor( Qt::WaitCursor );
  bool changed = false;
  for ( const QgsCallout::Property &p : std::as_const( mCalloutProperties ) )
  {
    int index = -1;

    // always use the default activated property
    QgsProperty prop = vlayer->labeling() && vlayer->labeling()->settings( details.providerID ).callout() ? vlayer->labeling()->settings( details.providerID ).callout()->dataDefinedProperties().property( p ) :
                       QgsProperty();
    if ( prop.propertyType() == Qgis::PropertyType::Field && prop.isActive() )
    {
      index = vlayer->fields().lookupField( prop.field() );
    }
    else
    {
      index = QgsAuxiliaryLayer::createProperty( p, vlayer, false );
      changed = true;
    }
    calloutIndexes[p] = index;
  }
  if ( changed )
    emit vlayer->styleChanged();

  return newAuxiliaryLayer;
}

void QgsMapToolLabel::updateHoveredLabel( QgsMapMouseEvent *e )
{
  if ( !mHoverRubberBand )
  {
    mHoverRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Line );
    mHoverRubberBand->setWidth( 2 );
    mHoverRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
    mHoverRubberBand->setColor( QColor( 200, 0, 120, 255 ) );
    mHoverRubberBand->setIcon( QgsRubberBand::ICON_BOX );

    double scaleFactor = mCanvas->fontMetrics().xHeight();
    mHoverRubberBand->setIconSize( scaleFactor );
  }

  QgsCalloutPosition calloutPosition;
  bool isOrigin = false;
  if ( !mCalloutProperties.isEmpty() && calloutAtPosition( e, calloutPosition, isOrigin ) )
  {
    if ( !mCalloutOtherPointsRubberBand )
    {
      mCalloutOtherPointsRubberBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Point );
      mCalloutOtherPointsRubberBand->setWidth( 2 );
      mCalloutOtherPointsRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
      mCalloutOtherPointsRubberBand->setColor( QColor( 200, 0, 120, 255 ) );
      mCalloutOtherPointsRubberBand->setIcon( QgsRubberBand::ICON_X );
      double scaleFactor = mCanvas->fontMetrics().xHeight();
      mCalloutOtherPointsRubberBand->setIconSize( scaleFactor );
    }

    // callouts are a smaller target, so they take precedence over labels
    mCurrentHoverLabel = LabelDetails();

    mHoverRubberBand->show();
    mHoverRubberBand->reset( Qgis::GeometryType::Point );
    mCalloutOtherPointsRubberBand->show();
    mCalloutOtherPointsRubberBand->reset( Qgis::GeometryType::Point );

    if ( isOrigin )
    {
      mHoverRubberBand->addPoint( calloutPosition.origin() );
      mCalloutOtherPointsRubberBand->addPoint( calloutPosition.destination() );
    }
    else
    {
      mHoverRubberBand->addPoint( calloutPosition.destination() );
      mCalloutOtherPointsRubberBand->addPoint( calloutPosition.origin() );
    }
    return;
  }

  if ( mCalloutOtherPointsRubberBand )
    mCalloutOtherPointsRubberBand->hide();

  QgsLabelPosition labelPos;
  if ( !labelAtPosition( e, labelPos ) )
  {
    mHoverRubberBand->hide();
    mCurrentHoverLabel = LabelDetails();
    return;
  }

  LabelDetails newHoverLabel( labelPos, canvas() );

  if ( mCurrentHoverLabel.valid &&
       newHoverLabel.layer == mCurrentHoverLabel.layer &&
       newHoverLabel.pos.featureId == mCurrentHoverLabel.pos.featureId &&
       newHoverLabel.pos.providerID == mCurrentHoverLabel.pos.providerID
     )
    return;

  if ( !canModifyLabel( newHoverLabel ) )
  {
    mHoverRubberBand->hide();
    mCurrentHoverLabel = LabelDetails();
    return;
  }

  mCurrentHoverLabel = newHoverLabel;

  mHoverRubberBand->show();
  mHoverRubberBand->reset( Qgis::GeometryType::Line );
  if ( const QgsLabelingResults *labelingResults = mCanvas->labelingResults( false ) )
  {
    if ( labelPos.groupedLabelId != 0 )
    {
      // if it's a curved label, we need to highlight ALL characters
      const QList< QgsLabelPosition > allPositions = labelingResults->groupedLabelPositions( labelPos.groupedLabelId );
      for ( const QgsLabelPosition &position : allPositions )
      {
        mHoverRubberBand->addGeometry( position.labelGeometry );
      }
    }
    else
    {
      mHoverRubberBand->addGeometry( labelPos.labelGeometry );
    }
  }
  else
  {
    mHoverRubberBand->addGeometry( labelPos.labelGeometry );
  }
  QgisApp::instance()->statusBarIface()->showMessage( tr( "Label “%1” in %2" ).arg( labelPos.labelText, mCurrentHoverLabel.layer->name() ), 2000 );
}

void QgsMapToolLabel::clearHoveredLabel()
{
  if ( mHoverRubberBand )
    mHoverRubberBand->hide();
  if ( mCalloutOtherPointsRubberBand )
    mCalloutOtherPointsRubberBand->hide();

  mCurrentHoverLabel = LabelDetails();
}

bool QgsMapToolLabel::canModifyLabel( const QgsMapToolLabel::LabelDetails & )
{
  return true;
}

bool QgsMapToolLabel::canModifyCallout( const QgsCalloutPosition &, bool, int &, int & )
{
  return false;
}
