/***************************************************************************
    qgshighlightablecombobox.cpp
     ---------------------------
    Date                 : 20/12/2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgshighlightablecombobox.h"
#include <QPainter>

QgsHighlightableComboBox::QgsHighlightableComboBox( QWidget *parent )
  : QComboBox( parent )
{}

void QgsHighlightableComboBox::paintEvent( QPaintEvent *e )
{
  QComboBox::paintEvent( e );
  if ( mHighlight )
  {
    QPainter p( this );
    int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

void QgsHighlightableComboBox::setHighlighted( bool highlighted )
{
  mHighlight = highlighted;
  update();
}
