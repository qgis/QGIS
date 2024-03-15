/***************************************************************************
    qgsappcanvasfiltering.cpp
    -------------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappcanvasfiltering.h"
#include "qgselevationcontrollerwidget.h"
#include "qgsmapcanvas.h"
#include "qgisapp.h"
#include <QInputDialog>

QgsAppCanvasFiltering::QgsAppCanvasFiltering( QObject *parent )
  : QObject( parent )
{

}

void QgsAppCanvasFiltering::setupElevationControllerAction( QAction *action, QgsMapCanvas *canvas )
{
  action->setCheckable( true );
  connect( action, &QAction::toggled, canvas, [canvas, action, this]( bool checked )
  {
    if ( checked )
    {
      QgsElevationControllerWidget *controller = new QgsElevationControllerWidget();
      connect( controller, &QgsElevationControllerWidget::rangeChanged, canvas, &QgsMapCanvas::setZRange );

      QAction *setProjectLimitsAction = new QAction( tr( "Set Elevation Range…" ), controller );
      controller->menu()->addAction( setProjectLimitsAction );
      connect( setProjectLimitsAction, &QAction::triggered, QgisApp::instance(), []
      {
        QgisApp::instance()->showProjectProperties( tr( "Elevation" ) );
      } );
      QAction *setRangeSizeAction = new QAction( tr( "Set Fixed Range Size…" ), controller );
      controller->menu()->addAction( setRangeSizeAction );
      connect( setRangeSizeAction, &QAction::triggered, QgisApp::instance(), [controller ]
      {
        const double existingSize = controller->fixedRangeSize();
        QgsElevationControllerFixedSizeDialog dialog( controller );
        dialog.setFixedRangeSize( existingSize >= 0 ? existingSize : -1 );
        if ( dialog.exec() )
        {
          controller->setFixedRangeSize( dialog.fixedRangeSize() );
        }
      } );
      QAction *disableAction = new QAction( tr( "Disable Elevation Filter" ), controller );
      controller->menu()->addAction( disableAction );
      connect( disableAction, &QAction::triggered, action, [action]
      {
        action->setChecked( false );
      } );

      canvas->addOverlayWidget( controller, Qt::Edge::LeftEdge );
      mCanvasElevationControllerMap.insert( canvas, controller );
      connect( canvas, &QObject::destroyed, this, [canvas, this]
      {
        mCanvasElevationControllerMap.remove( canvas );
      } );
      connect( controller, &QObject::destroyed, this, [canvas, this]
      {
        mCanvasElevationControllerMap.remove( canvas );
      } );
    }
    else
    {
      canvas->setZRange( QgsDoubleRange() );
      if ( QgsElevationControllerWidget *controller = mCanvasElevationControllerMap.value( canvas ) )
      {
        controller->deleteLater();
      }
    }
  } );
}

//
// QgsElevationControllerFixedWidthDialog
//
QgsElevationControllerFixedSizeDialog::QgsElevationControllerFixedSizeDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  mSizeSpin->setClearValue( -1, tr( "Not set" ) );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsElevationControllerFixedSizeDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsElevationControllerFixedSizeDialog::reject );
}

void QgsElevationControllerFixedSizeDialog::setFixedRangeSize( double size )
{
  mSizeSpin->setValue( size );
}

double QgsElevationControllerFixedSizeDialog::fixedRangeSize() const
{
  return mSizeSpin->value();
}
