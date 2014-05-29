#include "qgsslider.h"

#include <QRect>

QgsSlider::QgsSlider( QWidget * parent ) : QSlider( parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

QgsSlider::QgsSlider( Qt::Orientation orientation, QWidget * parent ) : QSlider( orientation, parent )
{
  setMinimumSize( QSize( 100, 40 ) );
}

void QgsSlider::paintEvent( QPaintEvent *event )
{
  QSlider::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.5, rect.width(), rect.height() ),
                    Qt::AlignHCenter, QString::number( value() ), 0 );
  painter.end();
}
