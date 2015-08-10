#include <QPainter>

#include "qgsconditionalstyle.h"
#include "qgsexpression.h"

QgsConditionalStyle::QgsConditionalStyle()
    : mValid( false )
{}

QgsConditionalStyle::QgsConditionalStyle( QString rule )
    : mValid( false )
{
  setRule( rule );
}

bool QgsConditionalStyle::matchForFeature( QgsFeature *feature, QgsFields fields )
{
  QgsExpression exp( mRule );
  return exp.evaluate( feature, fields ).toBool();
}

bool QgsConditionalStyle::matchForValue( QVariant value )
{
  // TODO Replace with Nyall's context based expressions.
  QgsExpression exp( QString( mRule ).replace( "@value", value.toString() ) );
  return exp.evaluate().toBool();
}

QPixmap QgsConditionalStyle::renderPreview()
{
  QPixmap pixmap( 64, 32 );
  QPainter painter( &pixmap );

  if ( mBackColor.isValid() )
    painter.setBrush( mBackColor );
  else
    painter.setBrush( QColor( Qt::white ) );

  QRect rect = QRect( 0, 0, 64, 32 );
  painter.setPen( Qt::NoPen );
  painter.drawRect( rect );
  painter.drawPixmap(8, 8, icon() );

  if ( mTextColor.isValid() )
    painter.setPen( mTextColor );
  else
    painter.setPen( Qt::black );

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.setFont( font() );
  rect = QRect( 32, 0, 32, 32 );
  painter.drawText( rect, Qt::AlignCenter, "abc\n123" );
  painter.end();
  return pixmap;
}
