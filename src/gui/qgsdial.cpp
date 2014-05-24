#include "qgsdial.h"

#include <QRect>

QgsDial::QgsDial( QWidget *parent ) : QDial( parent )
{
  setMinimumSize( QSize( 50, 50 ) );
}

void QgsDial::paintEvent( QPaintEvent *event )
{
  QDial::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.65, rect.width(), rect.height() ),
                    Qt::AlignHCenter, QString::number( value() ), 0 );
  painter.end();
}
