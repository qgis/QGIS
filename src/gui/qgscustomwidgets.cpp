#include "qgscustomwidgets.h"

#include <QRect>

EQDial::EQDial( QWidget *parent ) : QDial( parent )
{
  setMinimumSize( QSize(50, 50) );
}

void EQDial::paintEvent( QPaintEvent *event )
{
  QDial::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.65, rect.width(), rect.height() ),
                    Qt::AlignHCenter, QString::number( value() ), 0 );
  painter.end();
}

EQSlider::EQSlider ( QWidget * parent ) : QSlider ( parent )
{
  setMinimumSize( QSize(100, 40) );
}

EQSlider::EQSlider( Qt::Orientation orientation, QWidget * parent) : QSlider( orientation, parent )
{
  setMinimumSize( QSize(100, 40) );
}

void EQSlider::paintEvent( QPaintEvent *event )
{
  QSlider::paintEvent( event );
  QPainter painter( this );
  QRect rect = geometry();
  painter.setPen( QPen( palette().color( QPalette::WindowText ) ) );
  painter.drawText( QRectF( 0, rect.height() * 0.5, rect.width(), rect.height() ),
                    Qt::AlignHCenter, QString::number( value() ), 0 );
  painter.end();
}
