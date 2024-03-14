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
      QAction *setRangeWidthAction = new QAction( tr( "Set Fixed Range Width…" ), controller );
      controller->menu()->addAction( setRangeWidthAction );
      connect( setRangeWidthAction, &QAction::triggered, QgisApp::instance(), [controller ]
      {
        const double existingWidth = controller->fixedRangeWidth();
        QgsElevationControllerFixedWidthDialog dialog( controller );
        dialog.setFixedRangeWidth( existingWidth >= 0 ? existingWidth : -1 );
        if ( dialog.exec() )
        {
          controller->setFixedRangeWidth( dialog.fixedRangeWidth() );
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
QgsElevationControllerFixedWidthDialog::QgsElevationControllerFixedWidthDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  mWidthSpin->setClearValue( -1, tr( "Not set" ) );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsElevationControllerFixedWidthDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsElevationControllerFixedWidthDialog::reject );
}

void QgsElevationControllerFixedWidthDialog::setFixedRangeWidth( double width )
{
  mWidthSpin->setValue( width );
}

double QgsElevationControllerFixedWidthDialog::fixedRangeWidth() const
{
  return mWidthSpin->value();
}
