/***************************************************************************
    qgshighlightablelineedit.h
     -------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#include "qgshighlightablelineedit.h"
#include <QPainter>

QgsHighlightableLineEdit::QgsHighlightableLineEdit( QWidget *parent )
  : QgsFilterLineEdit( parent )
{}

void QgsHighlightableLineEdit::paintEvent( QPaintEvent *e )
{
  QgsFilterLineEdit::paintEvent( e );
  if ( mHighlight )
  {
    QPainter p( this );
    int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}

void QgsHighlightableLineEdit::setHighlighted( bool highlighted )
{
  mHighlight = highlighted;
  update();
}
