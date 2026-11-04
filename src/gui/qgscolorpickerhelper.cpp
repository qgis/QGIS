/***************************************************************************
     qgscolorpickerhelper.cpp
     ---------------
    Date                 : November 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorpickerhelper.h"

#include "qgis.h"
#include "qgsapplication.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>
#include <QWindow>

#include "moc_qgscolorpickerhelper.cpp"

QgsColorPickerHelper::QgsColorPickerHelper( QObject *parent )
  : QObject( parent )
  , mSampleTimer( new QTimer( this ) )
{
  // avoid spamming signals when sampling and mouse moves quickly
  // defer actual color sampling to a small timeout
  mSampleTimer->setSingleShot( true );
  mSampleTimer->setInterval( 25 );
  connect( mSampleTimer, &QTimer::timeout, this, &QgsColorPickerHelper::hoverSampleTimeout );
}

QgsColorPickerHelper::~QgsColorPickerHelper()
{
  if ( mPickingColor )
  {
    stopPicking();
  }
}

void QgsColorPickerHelper::activate( int sampleRadius )
{
  if ( mPickingColor )
  {
    // already active
    return;
  }

  mSampleRadius = sampleRadius;
  mPickingColor = true;
  mCurrentPickingColor = QColor();
  mFinalColor = QColor();
  QApplication::setOverrideCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Sampler ) );

  // set all top level widgets (windows) as mouse tracking, so user can sample colors
  // from eg a different layout designer window or from the main canvas from the layout designer
  const QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
  for ( QWidget *widget : topLevelWidgets )
  {
    if ( !widget->hasMouseTracking() )
    {
      widget->setAttribute( Qt::WA_Hover, true );
      widget->setMouseTracking( true );
      mWidgetsToCleanup.append( QPointer( widget ) );
    }
  }

  // event filter gets installed for the entire application
  qApp->installEventFilter( this );
}

bool QgsColorPickerHelper::eventFilter( QObject *watched, QEvent *event )
{
  if ( !mPickingColor )
  {
    return QObject::eventFilter( watched, event );
  }

  switch ( event->type() )
  {
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    {
      const QPoint pos = static_cast<QSinglePointEvent *>( event )->position().toPoint();
      QPoint localPos;
      if ( QWidget *target = findEventWindowWidgetAndLocalPos( watched, pos, localPos ) )
      {
        mPendingTargetWidget = target;
        mPendingLocalPos = localPos;
        // avoid spamming signals when sampling and mouse moves quickly
        if ( !mSampleTimer->isActive() )
        {
          mSampleTimer->start();
        }
      }
      return false;
    }

    case QEvent::MouseButtonPress:
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        // left mouse button down = sample
        mSampleTimer->stop();
        QColor finalColor;
        QPoint localPos;
        if ( QWidget *target = findEventWindowWidgetAndLocalPos( watched, mouseEvent->position(), localPos ) )
        {
          finalColor = sampleColor( target, localPos );
        }
        mFinalColor = finalColor.isValid() ? finalColor : mCurrentPickingColor;
        return true;
      }
      else if ( mouseEvent->button() == Qt::RightButton )
      {
        return true;
      }
      break;
    }

    case QEvent::MouseButtonRelease:
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        if ( mFinalColor.isValid() )
          onColorPicked( mFinalColor );
        else
          cancelPicking();
        return true;
      }
      else if ( mouseEvent->button() == Qt::RightButton )
      {
        // right click = cancel picking
        cancelPicking();
        return true;
      }
      break;
    }

    case QEvent::KeyPress:
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
      switch ( keyEvent->key() )
      {
        case Qt::Key_Escape:
          cancelPicking();
          return true;

        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
          mSampleTimer->stop();
          if ( mPendingTargetWidget )
          {
            const QColor sampled = sampleColor( mPendingTargetWidget, mPendingLocalPos );
            if ( sampled.isValid() )
              mCurrentPickingColor = sampled;
          }
          onColorPicked( mCurrentPickingColor );
          return true;
          return true;

        default:
          if ( keyEvent->text().isEmpty() )
            return false; // don't swallow modifier keys
          return true;
      }
      break;
    }

    default:
      break;
  }

  return QObject::eventFilter( watched, event );
}

void QgsColorPickerHelper::hoverSampleTimeout()
{
  if ( !mPickingColor || !mPendingTargetWidget )
    return;

  const QColor sampled = sampleColor( mPendingTargetWidget, mPendingLocalPos );
  if ( sampled.isValid() )
  {
    mCurrentPickingColor = sampled;
    emit colorHovered( mCurrentPickingColor );
  }
}

QWidget *QgsColorPickerHelper::findEventWindowWidgetAndLocalPos( QObject *watched, const QPointF &eventPos, QPoint &outPos )
{
  // object may already be a widget
  if ( QWidget *w = qobject_cast<QWidget *>( watched ) )
  {
    // if so, we use the top level window associated with this widget -- that ensures that
    // sampled colors correctly handle semi-transparent widgets, by returning the ACTUAL
    // visible pixel color for the entire window at that point
    QWidget *topLevel = w->window();
    outPos = w->mapTo( topLevel, eventPos.toPoint() );
    return topLevel;
  }

  // or it might be a QWindow, in which case we need to match it to a top-level widget
  if ( QWindow *window = qobject_cast<QWindow *>( watched ) )
  {
    const QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
    for ( QWidget *widget : topLevelWidgets )
    {
      if ( widget->windowHandle() == window )
      {
        outPos = eventPos.toPoint();
        return widget;
      }
    }
  }

  return nullptr;
}

void QgsColorPickerHelper::onColorPicked( const QColor &color )
{
  stopPicking();
  emit colorPicked( color );
}

void QgsColorPickerHelper::cancelPicking()
{
  stopPicking();
  emit canceled();
}

void QgsColorPickerHelper::stopPicking()
{
  mSampleTimer->stop();
  mPendingTargetWidget = nullptr;

  qApp->removeEventFilter( this );

  for ( const QPointer< QWidget > &widget : std::as_const( mWidgetsToCleanup ) )
  {
    if ( widget )
    {
      widget->setAttribute( Qt::WA_Hover, false );
      widget->setMouseTracking( false );
    }
  }
  mWidgetsToCleanup.clear();

  QgsApplication::restoreOverrideCursor();
  mPickingColor = false;
}

QColor QgsColorPickerHelper::sampleColor( QWidget *widget, const QPoint &pos ) const
{
  if ( !widget )
    return QColor();

  const int radius = std::max( 0, mSampleRadius );
  QRect grabRect( pos.x() - radius, pos.y() - radius, 1 + radius * 2, 1 + radius * 2 );
  // don't try to grab stuff outside window bounds
  grabRect = grabRect.intersected( widget->rect() );
  if ( grabRect.isEmpty() )
    return QColor();

  const QPixmap pixmap = widget->grab( grabRect );
  if ( pixmap.isNull() )
    return QColor();

  const QImage image = pixmap.toImage();
  if ( radius == 0 )
  {
    return image.pixelColor( 0, 0 );
  }
  else
  {
    return averageColor( image );
  }
}

QColor QgsColorPickerHelper::averageColor( const QImage &image )
{
  QRgb tmpRgb;
  int colorCount = 0;
  quint64 sumRed = 0;
  quint64 sumBlue = 0;
  quint64 sumGreen = 0;
  //scan through image and sum rgb components
  for ( int heightIndex = 0; heightIndex < image.height(); ++heightIndex )
  {
    const QRgb *scanLine = reinterpret_cast<const QRgb *>( image.constScanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < image.width(); ++widthIndex )
    {
      tmpRgb = scanLine[widthIndex];
      sumRed += qRed( tmpRgb );
      sumBlue += qBlue( tmpRgb );
      sumGreen += qGreen( tmpRgb );
      colorCount++;
    }
  }
  //calculate average components as floats
  const double avgRed = static_cast<double>( sumRed ) / ( 255.0 * colorCount );
  const double avgGreen = static_cast<double>( sumGreen ) / ( 255.0 * colorCount );
  const double avgBlue = static_cast<double>( sumBlue ) / ( 255.0 * colorCount );

  //create a new color representing the average
  return QColor::fromRgbF( avgRed, avgGreen, avgBlue );
}
