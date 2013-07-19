#include "qgscustomwidgets.h"

#include <QRect>

EQDial::EQDial( QWidget *parent ) : QDial( parent )
{
  setMinimumSize( QSize(60, 60) );
}

void EQDial::paintEvent( QPaintEvent *event )
{
  QDial::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.66, rect.width(), rect.height() ),
                    Qt::AlignHCenter, QString::number( value() ), 0 );
  painter.end();
}
