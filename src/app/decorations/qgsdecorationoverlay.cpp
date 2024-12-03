/***************************************************************************
                         qgsdecorationoverlay.cpp
                         ----------------------
    begin                : July 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationoverlay.h"
#include "moc_qgsdecorationoverlay.cpp"
#include "qgsmapdecoration.h"
#include "qgsmapcanvas.h"
#include "qgisapp.h"

#include <QPainter>

QgsDecorationOverlay::QgsDecorationOverlay( QWidget *parent )
  : QWidget( parent )
{
  // Make the overlay transparent
  setAttribute( Qt::WA_NoSystemBackground );

  // Make the overlay transparent for mouse events
  setAttribute( Qt::WA_TransparentForMouseEvents );

  // Install the event filter to catch resize events
  parent->installEventFilter( this );
}


void QgsDecorationOverlay::paintEvent( QPaintEvent * )
{
  const QList<QgsMapDecoration *> decorations = QgisApp::instance()->activeDecorations();
  if ( decorations.empty() )
    return;

  QPainter p( this );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( QgisApp::instance()->mapCanvas()->mapSettings() );
  context.setPainter( &p );
  context.setDevicePixelRatio( 1 );

  for ( QgsMapDecoration *item : decorations )
  {
    // Do not render decorations with fixed map position they are rendered directly on the map canvas
    if ( item->hasFixedMapPosition() )
      continue;
    item->render( QgisApp::instance()->mapCanvas()->mapSettings(), context );
  }
}

bool QgsDecorationOverlay::eventFilter( QObject *obj, QEvent *event )
{
  // Resize the overlay to match the parent widget
  if ( event->type() == QEvent::Resize )
  {
    resize( parentWidget()->size() );
  }
  return QWidget::eventFilter( obj, event );
}
