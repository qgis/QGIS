#include "qgslabelpreview.h"

#include <QPainter>

#include "qgspallabeling.h"

QgsLabelPreview::QgsLabelPreview( QWidget* parent )
    : QLabel( parent )
{
}

void QgsLabelPreview::setTextColor( QColor color )
{
  mTextColor = color;
  update();
}

void QgsLabelPreview::setBuffer( double size, QColor color )
{
  mBufferSize = size * 88 / 25.4; //assume standard dpi for preview
  mBufferColor = color;
  update();
}

void QgsLabelPreview::paintEvent( QPaintEvent *e )
{
  Q_UNUSED( e );
  QPainter p( this );

  p.setRenderHint( QPainter::Antialiasing );
  p.setFont( font() );
  p.translate( 10, 20 ); // uhm...

  if ( mBufferSize != 0 )
    QgsPalLabeling::drawLabelBuffer( &p, text(), font(), mBufferSize, mBufferColor );

  p.setPen( mTextColor );
  p.drawText( 0, 0, text() );
}
