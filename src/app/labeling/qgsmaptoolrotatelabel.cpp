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

QgsMapToolRotateLabel::QgsMapToolRotateLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolLabel( canvas, cadDock )
{
  mPalProperties << QgsPalLayerSettings::LabelRotation;
}

QgsMapToolRotateLabel::~QgsMapToolRotateLabel()
{
  delete mRotationItem;
}

void QgsMapToolRotateLabel::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mLabelRubberBand )
  {
    const QgsPointXY currentPoint = toMapCoordinates( e->pos() );
    const double azimuth = convertAzimuth( mRotationPoint.azimuth( currentPoint ) );
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
  else
  {
    updateHoveredLabel( e );
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
    clearHoveredLabel();

    QgsLabelPosition labelPos;
    if ( !labelAtPosition( e, labelPos ) )
    {
      mCurrentLabel = LabelDetails();
      return;
    }

    mCurrentLabel = LabelDetails( labelPos, canvas() );

    if ( !mCurrentLabel.valid )
      return;

    // Get label rotation point
    if ( !currentLabelRotationPoint( mRotationPoint, false ) )
      return;

    {
      mCurrentMouseAzimuth = convertAzimuth( mRotationPoint.azimuth( toMapCoordinates( e->pos() ) ) );

      bool hasRotationValue;
      int rotationCol;

      const PropertyStatus status = labelRotatableStatus( mCurrentLabel.layer, mCurrentLabel.settings, rotationCol );
      switch ( status )
      {
        case PropertyStatus::DoesNotExist:
        {
          QgsPalIndexes indexes;
          if ( createAuxiliaryFields( indexes ) )
            return;

          if ( labelRotatableStatus( mCurrentLabel.layer, mCurrentLabel.settings, rotationCol ) != PropertyStatus::Valid )
            return;
          break;
        }

        case PropertyStatus::Valid:
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
          break;
        }

        case PropertyStatus::CurrentExpressionInvalid:
        {
          QgisApp::instance()->messageBar()->pushWarning( tr( "Rotate Label" ), tr( "Cannot rotate “%1” — the layer “%2” has an invalid expression set for label rotation" ).arg( mCurrentLabel.pos.labelText, mCurrentLabel.layer->name() ) );
          return;
        }
      }

      if ( currentLabelDataDefinedRotation( mCurrentRotation, hasRotationValue, rotationCol, true ) )
      {
        if ( !hasRotationValue )
        {
          mCurrentRotation = 0;
        }

        // Convert to degree
        mCurrentRotation = mCurrentRotation
                           * QgsUnitTypes::fromUnitToUnitFactor( mCurrentLabel.settings.rotationUnit(),
                               QgsUnitTypes::AngleDegrees );

        mStartRotation = mCurrentRotation;
        createRubberBands();

        createRotationPreviewBox();

        mRotationItem = new QgsPointRotationItem( mCanvas );
        mRotationItem->setOrientation( QgsPointRotationItem::Clockwise );
        mRotationItem->setPointLocation( mRotationPoint );
        mRotationItem->setRotationUnit( mCurrentLabel.settings.rotationUnit() );
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
        mRotationPreviewBox.reset();
        return;
      }

      case Qt::LeftButton:
      {
        // second click locks in rotation
        deleteRubberBands();
        delete mRotationItem;
        mRotationItem = nullptr;
        mRotationPreviewBox.reset();

        QgsVectorLayer *vlayer = mCurrentLabel.layer;
        if ( !vlayer )
        {
          return;
        }

        int rotationCol;
        if ( labelRotatableStatus( vlayer, mCurrentLabel.settings, rotationCol ) != PropertyStatus::Valid )
        {
          return;
        }

        const double rotationDegree = mCtrlPressed ? roundTo15Degrees( mCurrentRotation ) : mCurrentRotation;
        if ( qgsDoubleNear( rotationDegree, mStartRotation ) ) //mouse button pressed / released, but no rotation
        {
          return;
        }

        // Convert back to settings unit
        const double rotation = rotationDegree * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleDegrees,
                                mCurrentLabel.settings.rotationUnit() );

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

void QgsMapToolRotateLabel::keyPressEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand )
  {
    switch ( e->key() )
    {
      case Qt::Key_Delete:
      {
        e->ignore();  // Override default shortcut management
        return;
      }
    }
  }

  QgsMapToolLabel::keyPressEvent( e );
}

void QgsMapToolRotateLabel::keyReleaseEvent( QKeyEvent *e )
{
  if ( mLabelRubberBand )
  {
    switch ( e->key() )
    {
      case Qt::Key_Delete:
      {
        // delete the label rotation
        QgsVectorLayer *vlayer = mCurrentLabel.layer;
        if ( vlayer )
        {
          int rotationCol;
          if ( labelRotatableStatus( vlayer, mCurrentLabel.settings, rotationCol ) == PropertyStatus::Valid )
          {
            vlayer->beginEditCommand( tr( "Delete Label Rotation" ) + QStringLiteral( " '%1'" ).arg( currentLabelText( 24 ) ) );
            if ( !vlayer->changeAttributeValue( mCurrentLabel.pos.featureId, rotationCol, QVariant() ) )
            {
              // if the edit command fails, it's likely because the label x/y is being stored in a physical field (not a auxiliary one!)
              // and the layer isn't in edit mode
              if ( !vlayer->isEditable() )
              {
                QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Rotation" ), tr( "Layer “%1” must be editable in order to move labels from it" ).arg( vlayer->name() ) );
              }
              else
              {
                QgisApp::instance()->messageBar()->pushWarning( tr( "Delete Label Rotation" ), tr( "Error encountered while storing new label position" ) );
              }

            }
            vlayer->endEditCommand();
            deleteRubberBands();
            delete mRotationItem;
            mRotationItem = nullptr;
            mRotationPreviewBox.reset();
            vlayer->triggerRepaint();
          }
        }
        e->ignore();  // Override default shortcut management
        break;
      }

      case Qt::Key_Escape:
      {
        // escape is cancel
        deleteRubberBands();
        delete mRotationItem;
        mRotationItem = nullptr;
        mRotationPreviewBox.reset();
      }
    }
  }
}

int QgsMapToolRotateLabel::roundTo15Degrees( double n )
{
  const int m = static_cast< int >( n / 15.0 + 0.5 );
  return ( m * 15 );
}

double QgsMapToolRotateLabel::convertAzimuth( double a )
{
  a -= 90; // convert from 0 = north to 0 = east
  return ( a <= -180.0 ? 360 + a : a );
}

void QgsMapToolRotateLabel::createRotationPreviewBox()
{
  mRotationPreviewBox.reset();
  const QVector< QgsPointXY > boxPoints = mCurrentLabel.pos.cornerPoints;
  if ( boxPoints.empty() )
    return;

  mRotationPreviewBox.reset( new QgsRubberBand( mCanvas, QgsWkbTypes::LineGeometry ) );
  mRotationPreviewBox->setColor( QColor( 0, 0, 255, 65 ) );
  mRotationPreviewBox->setWidth( 3 );
  setRotationPreviewBox( mCurrentRotation - mStartRotation );
}

void QgsMapToolRotateLabel::setRotationPreviewBox( double rotation )
{
  if ( !mRotationPreviewBox )
  {
    return;
  }

  mRotationPreviewBox->reset();
  if ( mCurrentLabel.pos.cornerPoints.empty() )
    return;

  const QVector< QgsPointXY > cornerPoints = mCurrentLabel.pos.cornerPoints;
  for ( const QgsPointXY &cornerPoint : cornerPoints )
    mRotationPreviewBox->addPoint( rotatePointClockwise( cornerPoint, mRotationPoint, rotation ) );
  mRotationPreviewBox->addPoint( rotatePointClockwise( mCurrentLabel.pos.cornerPoints.at( 0 ), mRotationPoint, rotation ) );
  mRotationPreviewBox->show();
}

QgsPointXY QgsMapToolRotateLabel::rotatePointClockwise( const QgsPointXY &input, const QgsPointXY &centerPoint, double degrees ) const
{
  const double rad = -degrees / 180 * M_PI;
  const double v1x = input.x() - centerPoint.x();
  const double v1y = input.y() - centerPoint.y();

  const double v2x = std::cos( rad ) * v1x - std::sin( rad ) * v1y;
  const double v2y = std::sin( rad ) * v1x + std::cos( rad ) * v1y;

  return QgsPointXY( centerPoint.x() + v2x, centerPoint.y() + v2y );
}
