#include <QPainter>

#include "qgscondtionalstyle.h"
#include "qgsexpression.h"

QgsConditionalStyle::QgsConditionalStyle()
    : mValid( false )
{}

QgsConditionalStyle::QgsConditionalStyle( QString rule )
    : mValid( false )
{
  setRule( rule );
}

bool QgsConditionalStyle::matchForFeature( QString fieldName, QgsFeature *feature, QgsFields fields )
{
  fieldName = QString( """%1""" ).arg( fieldName );
  QgsExpression exp( QString( mRule ).replace( "@value", fieldName ) );
  return exp.evaluate( feature, fields ).toBool();
}

bool QgsConditionalStyle::matchForValue( QVariant value )
{
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

  QRect rect( 0, 0, 64, 32 );
  painter.drawRect( rect );

  if ( mTextColor.isValid() )
    painter.setPen( mTextColor );

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.drawText( rect, Qt::AlignCenter, "abc\n123" );
  painter.end();
  return pixmap;
}
