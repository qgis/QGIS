/***************************************************************************
                          qgsmaptoolrotatelabel.cpp
                          -------------------------
    begin                : 2010-11-09
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

#include "qgsmaptoolrotatelabel.h"
#include "qgsmapcanvas.h"
#include "qgspallabeling.h"
#include "qgspointrotationitem.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"

#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"

QgsMapToolRotateLabel::QgsMapToolRotateLabel( QgsMapCanvas *canvas )
  : QgsMapToolLabel( canvas )
  , mStartRotation( 0.0 )
  , mCurrentRotation( 0.0 )
  , mCurrentMouseAzimuth( 0.0 )
  , mCtrlPressed( false )
{
  mPalProperties << QgsPalLayerSettings::LabelRotation;
}

QgsMapToolRotateLabel::~QgsMapToolRotateLabel()
{
  delete mRotationItem;
  delete mRotationPreviewBox;
}

void QgsMapToolRotateLabel::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mLabelRubberBand )
  {
    QgsPointXY currentPoint = toMapCoordinates( e->pos() );
    double azimuth = convertAzimuth( mRotationPoint.azimuth( currentPoint ) );
    double azimuthDiff = azimuth - mCurrentMouseAzimuth;
    azimuthDiff = azimuthDiff > 180 ? azimuthDiff - 360 : azimuthDiff;

    mCurrentRotation += azimuthDiff;
    if ( mCurrentRotation >= 360 || mCurrentRotation <= -360 )
      mCurrentRotation = std::fmod( mCurrentRotation, 360.0 );
    if ( mCurrentRotation < 0 )
      mCurrentRotation += 360.0;

    mCurrentMouseAzimuth = std::fmod( azimuth, 360.0 );

    //if shift-modifier is pressed, round to 15 degrees
    int displayValue;
    if ( e->modifiers() & Qt::ControlModifier )
    {
      displayValue = roundTo15Degrees( mCurrentRotation );
      mCtrlPressed = true;
    }
    else
    {
      displayValue = static_cast< int >( mCurrentRotation );
      mCtrlPressed = false;
    }

    if ( mRotationItem )
    {
      mRotationItem->setSymbolRotation( displayValue );
      setRotationPreviewBox( displayValue - mStartRotation );
      mRotationItem->update();
    }
  }
}

void QgsMapToolRotateLabel::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mLabelRubberBand )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    // first click starts rotation tool
    deleteRubberBands();

    QgsLabelPosition labelPos;
    if ( !labelAtPosition( e, labelPos ) )
    {
      mCurrentLabel = LabelDetails();
      return;
    }

    mCurrentLabel = LabelDetails( labelPos );

    if ( !mCurrentLabel.valid )
      return;

    // only rotate non-pinned OverPoint placements until other placements are supported in pal::Feature

    if ( !mCurrentLabel.pos.isPinned
         && mCurrentLabel.settings.placement != QgsPalLayerSettings::OverPoint )
    {
      return;
    }

    // rotate unpinned labels (i.e. no hali/vali settings) as if hali/vali was Center/Half
    if ( !currentLabelRotationPoint( mRotationPoint, false, !mCurrentLabel.pos.isPinned ) )
    {
      return;
    }

    {
      mCurrentMouseAzimuth = convertAzimuth( mRotationPoint.azimuth( toMapCoordinates( e->pos() ) ) );

      bool hasRotationValue;
      int rotationCol;

      if ( !labelIsRotatable( mCurrentLabel.layer, mCurrentLabel.settings, rotationCol ) )
      {
        QgsPalIndexes indexes;
        if ( createAuxiliaryFields( indexes ) )
          return;

        if ( !labelIsRotatable( mCurrentLabel.layer, mCurrentLabel.settings, rotationCol ) )
          return;
      }
      else
      {
        const bool usesAuxField = mCurrentLabel.layer->fields().fieldOrigin( rotationCol ) == QgsFields::OriginJoin;
        if ( !usesAuxField && !mCurrentLabel.layer->isEditable() )
        {
          if ( mCurrentLabel.layer->startEditing() )
          {
            QgisApp::instance()->messageBar()->pushInfo( tr( "Rotate Label" ), tr( "Layer “%1” was made editable" ).arg( mCurrentLabel.layer->name() ) );
          }
          else
          {
            QgisApp::instance()->messageBar()->pushWarning( tr( "Rotate Label" ), tr( "Cannot rotate “%1” — the layer “%2” could not be made editable" ).arg( mCurrentLabel.pos.labelText, mCurrentLabel.layer->name() ) );
            return;
          }
        }
      }

      if ( currentLabelDataDefinedRotation( mCurrentRotation, hasRotationValue, rotationCol, true ) )
      {
        if ( !hasRotationValue )
        {
          mCurrentRotation = 0;
        }
        mStartRotation = mCurrentRotation;
        createRubberBands();

        mRotationPreviewBox = createRotationPreviewBox();

        mRotationItem = new QgsPointRotationItem( mCanvas );
        mRotationItem->setOrientation( QgsPointRotationItem::Clockwise );
        mRotationItem->setPointLocation( mRotationPoint );
        mRotationItem->setSymbolRotation( static_cast< int >( mCurrentRotation ) );
      }
    }
  }
  else
  {
    switch ( e->button() )
    {
      case Qt::RightButton:
      {
        // right click is cancel
        deleteRubberBands();
        delete mRotationItem;
        mRotationItem = nullptr;
        delete mRotationPreviewBox;
        mRotationPreviewBox = nullptr;
        return;
      }

      case Qt::LeftButton:
      {
        // second click locks in rotation
        deleteRubberBands();
        delete mRotationItem;
        mRotationItem = nullptr;
        delete mRotationPreviewBox;
        mRotationPreviewBox = nullptr;

        QgsVectorLayer *vlayer = mCurrentLabel.layer;
        if ( !vlayer )
        {
          return;
        }

        int rotationCol;
        if ( !labelIsRotatable( vlayer, mCurrentLabel.settings, rotationCol ) )
        {
          return;
        }

        double rotation = mCtrlPressed ? roundTo15Degrees( mCurrentRotation ) : mCurrentRotation;
        if ( qgsDoubleNear( rotation, mStartRotation ) ) //mouse button pressed / released, but no rotation
        {
          return;
        }

        vlayer->beginEditCommand( tr( "Rotated label" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
        if ( !vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, rotationCol, rotation ) )
        {
          if ( !vlayer->isEditable() )
          {
            QgisApp::instance()->messageBar()->pushWarning( tr( "Rotate Label" ), tr( "Layer “%1” must be editable in order to rotate labels from it" ).arg( vlayer->name() ) );
          }
          else
          {
            QgisApp::instance()->messageBar()->pushWarning( tr( "Rotate Label" ), tr( "Error encountered while storing new label rotation" ) );
          }
        }
        vlayer->endEditCommand();
        vlayer->triggerRepaint();
        break;
      }

      default:
        break;
    }
  }
}

void QgsMapToolRotateLabel::keyReleaseEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand && e->key() == Qt::Key_Escape )
  {
    // escape is cancel
    deleteRubberBands();
    delete mRotationItem;
    mRotationItem = nullptr;
    delete mRotationPreviewBox;
    mRotationPreviewBox = nullptr;
  }
}

int QgsMapToolRotateLabel::roundTo15Degrees( double n )
{
  int m = static_cast< int >( n / 15.0 + 0.5 );
  return ( m * 15 );
}

double QgsMapToolRotateLabel::convertAzimuth( double a )
{
  a -= 90; // convert from 0 = north to 0 = east
  return ( a <= -180.0 ? 360 + a : a );
}

QgsRubberBand *QgsMapToolRotateLabel::createRotationPreviewBox()
{
  delete mRotationPreviewBox;
  QVector< QgsPointXY > boxPoints = mCurrentLabel.pos.cornerPoints;
  if ( boxPoints.empty() )
  {
    return nullptr;
  }

  mRotationPreviewBox = new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry );
  mRotationPreviewBox->setColor( QColor( 0, 0, 255, 65 ) );
  mRotationPreviewBox->setWidth( 3 );
  setRotationPreviewBox( mCurrentRotation - mStartRotation );
  return mRotationPreviewBox;
}

void QgsMapToolRotateLabel::setRotationPreviewBox( double rotation )
{
  if ( !mRotationPreviewBox )
  {
    return;
  }

  mRotationPreviewBox->reset();
  QVector< QgsPointXY > boxPoints = mCurrentLabel.pos.cornerPoints;
  if ( boxPoints.empty() )
  {
    return;
  }

  for ( int i = 0; i < boxPoints.size(); ++i )
  {
    mRotationPreviewBox->addPoint( rotatePointClockwise( boxPoints.at( i ), mRotationPoint, rotation ) );
  }
  mRotationPreviewBox->addPoint( rotatePointClockwise( boxPoints.at( 0 ), mRotationPoint, rotation ) );
  mRotationPreviewBox->show();
}

QgsPointXY QgsMapToolRotateLabel::rotatePointClockwise( const QgsPointXY &input, const QgsPointXY &centerPoint, double degrees ) const
{
  double rad = -degrees / 180 * M_PI;
  double v1x = input.x() - centerPoint.x();
  double v1y = input.y() - centerPoint.y();

  double v2x = std::cos( rad ) * v1x - std::sin( rad ) * v1y;
  double v2y = std::sin( rad ) * v1x + std::cos( rad ) * v1y;

  return QgsPointXY( centerPoint.x() + v2x, centerPoint.y() + v2y );
}
