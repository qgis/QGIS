/***************************************************************************
   qgsstatusbarcoordinateswidget.cpp
    --------------------------------------
   Date                 : 05.08.2015
   Copyright            : (C) 2015 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QSpacerItem>
#include <QTimer>
#include <QToolButton>

#include "qgsstatusbarcoordinateswidget.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgscoordinateutils.h"


QgsStatusBarCoordinatesWidget::QgsStatusBarCoordinatesWidget( QWidget *parent )
  : QWidget( parent )
  , mMousePrecisionDecimalPlaces( 0 )
{
  // add a label to show current position
  mLabel = new QLabel( QString(), this );
  mLabel->setObjectName( QStringLiteral( "mCoordsLabel" ) );
  mLabel->setMinimumWidth( 10 );
  //mCoordsLabel->setMaximumHeight( 20 );
  mLabel->setMargin( 3 );
  mLabel->setAlignment( Qt::AlignCenter );
  mLabel->setFrameStyle( QFrame::NoFrame );
  mLabel->setText( tr( "Coordinate" ) );
  mLabel->setToolTip( tr( "Current map coordinate" ) );

  mLineEdit = new QLineEdit( this );
  mLineEdit->setMinimumWidth( 10 );
  mLineEdit->setMaximumWidth( 300 );
  //mLineEdit->setMaximumHeight( 20 );
  mLineEdit->setContentsMargins( 0, 0, 0, 0 );
  mLineEdit->setAlignment( Qt::AlignCenter );
  connect( mLineEdit, &QLineEdit::returnPressed, this, &QgsStatusBarCoordinatesWidget::validateCoordinates );

  QRegExp coordValidator( "[+-]?\\d+\\.?\\d*\\s*,\\s*[+-]?\\d+\\.?\\d*" );
  mCoordsEditValidator = new QRegExpValidator( coordValidator, this );
  mLineEdit->setWhatsThis( tr( "Shows the map coordinates at the "
                               "current cursor position. The display is continuously updated "
                               "as the mouse is moved. It also allows editing to set the canvas "
                               "center to a given position. The format is longitude,latitude or east,north" ) );
  mLineEdit->setToolTip( tr( "Current map coordinate (longitude,latitude or east,north)" ) );

  //toggle to switch between mouse pos and extents display in status bar widget
  mToggleExtentsViewButton = new QToolButton( this );
  mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "tracking.svg" ) ) );
  mToggleExtentsViewButton->setToolTip( tr( "Toggle extents and mouse position display" ) );
  mToggleExtentsViewButton->setCheckable( true );
  mToggleExtentsViewButton->setAutoRaise( true );
  connect( mToggleExtentsViewButton, &QAbstractButton::toggled, this, &QgsStatusBarCoordinatesWidget::extentsViewToggled );

  QHBoxLayout *layout = new QHBoxLayout( this );
  setLayout( layout );
  layout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  layout->addWidget( mLabel );
  layout->addWidget( mLineEdit );
  layout->addWidget( mToggleExtentsViewButton );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setAlignment( Qt::AlignRight );
  layout->setSpacing( 0 );

  // When you feel dizzy
  mDizzyTimer = new QTimer( this );
  connect( mDizzyTimer, &QTimer::timeout, this, &QgsStatusBarCoordinatesWidget::dizzy );
}

void QgsStatusBarCoordinatesWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  if ( mMapCanvas )
  {
    disconnect( mMapCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsStatusBarCoordinatesWidget::showMouseCoordinates );
    disconnect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsStatusBarCoordinatesWidget::showExtent );
  }

  mMapCanvas = mapCanvas;
  connect( mMapCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsStatusBarCoordinatesWidget::showMouseCoordinates );
  connect( mMapCanvas, &QgsMapCanvas::extentsChanged, this, &QgsStatusBarCoordinatesWidget::showExtent );
}

void QgsStatusBarCoordinatesWidget::setFont( const QFont &myFont )
{
  mLineEdit->setFont( myFont );
  mLabel->setFont( myFont );
}

void QgsStatusBarCoordinatesWidget::setMouseCoordinatesPrecision( unsigned int precision )
{
  mMousePrecisionDecimalPlaces = precision;
}


void QgsStatusBarCoordinatesWidget::validateCoordinates()
{
  if ( !mMapCanvas )
  {
    return;
  }
  if ( mLineEdit->text() == QLatin1String( "dizzy" ) )
  {
    // sometimes you may feel a bit dizzy...
    if ( mDizzyTimer->isActive() )
    {
      mDizzyTimer->stop();
      mMapCanvas->setSceneRect( mMapCanvas->viewport()->rect() );
      mMapCanvas->setTransform( QTransform() );
    }
    else
    {
      mDizzyTimer->start( 100 );
    }
    return;
  }
  else if ( mLineEdit->text() == QLatin1String( "retro" ) )
  {
    mMapCanvas->setProperty( "retro", !mMapCanvas->property( "retro" ).toBool() );
    refreshMapCanvas();
    return;
  }

  bool xOk = false;
  bool  yOk = false;
  double x = 0., y = 0.;
  QString coordText = mLineEdit->text();
  coordText.replace( QRegExp( " {2,}" ), QStringLiteral( " " ) );

  QStringList parts = coordText.split( ',' );
  if ( parts.size() == 2 )
  {
    x = parts.at( 0 ).toDouble( &xOk );
    y = parts.at( 1 ).toDouble( &yOk );
  }

  if ( !xOk || !yOk )
  {
    parts = coordText.split( ' ' );
    if ( parts.size() == 2 )
    {
      x = parts.at( 0 ).toDouble( &xOk );
      y = parts.at( 1 ).toDouble( &yOk );
    }
  }

  if ( !xOk || !yOk )
    return;

  mMapCanvas->setCenter( QgsPointXY( x, y ) );
  mMapCanvas->refresh();
}


void QgsStatusBarCoordinatesWidget::dizzy()
{
  if ( !mMapCanvas )
  {
    return;
  }
  // constants should go to options so that people can customize them to their taste
  int d = 10; // max. translational dizziness offset
  int r = 4;  // max. rotational dizzines angle
  QRectF rect = mMapCanvas->sceneRect();
  if ( rect.x() < -d || rect.x() > d || rect.y() < -d || rect.y() > d )
    return; // do not affect panning
  rect.moveTo( ( qrand() % ( 2 * d ) ) - d, ( qrand() % ( 2 * d ) ) - d );
  mMapCanvas->setSceneRect( rect );
  QTransform matrix;
  matrix.rotate( ( qrand() % ( 2 * r ) ) - r );
  mMapCanvas->setTransform( matrix );
}

void QgsStatusBarCoordinatesWidget::extentsViewToggled( bool flag )
{
  if ( flag )
  {
    //extents view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "extents.svg" ) ) );
    mLineEdit->setToolTip( tr( "Map coordinates for the current view extents" ) );
    mLineEdit->setReadOnly( true );
    showExtent();
  }
  else
  {
    //mouse cursor pos view mode!
    mToggleExtentsViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "tracking.svg" ) ) );
    mLineEdit->setToolTip( tr( "Map coordinates at mouse cursor position" ) );
    mLineEdit->setReadOnly( false );
    mLabel->setText( tr( "Coordinate:" ) );
  }
}

void QgsStatusBarCoordinatesWidget::refreshMapCanvas()
{
  if ( !mMapCanvas )
    return;

  //stop any current rendering
  mMapCanvas->stopRendering();
  mMapCanvas->refreshAllLayers();
}

void QgsStatusBarCoordinatesWidget::showMouseCoordinates( const QgsPointXY &p )
{
  if ( !mMapCanvas || mToggleExtentsViewButton->isChecked() )
  {
    return;
  }

  mLineEdit->setText( QgsCoordinateUtils::formatCoordinateForProject( p, mMapCanvas->mapSettings().destinationCrs(),
                      mMousePrecisionDecimalPlaces ) );

  if ( mLineEdit->width() > mLineEdit->minimumWidth() )
  {
    mLineEdit->setMinimumWidth( mLineEdit->width() );
  }
}


void QgsStatusBarCoordinatesWidget::showExtent()
{
  if ( !mToggleExtentsViewButton->isChecked() )
  {
    return;
  }

  // update the statusbar with the current extents.
  QgsRectangle myExtents = mMapCanvas->extent();
  mLabel->setText( tr( "Extents:" ) );
  mLineEdit->setText( myExtents.toString( true ) );
  //ensure the label is big enough
  if ( mLineEdit->width() > mLineEdit->minimumWidth() )
  {
    mLineEdit->setMinimumWidth( mLineEdit->width() );
  }
}
