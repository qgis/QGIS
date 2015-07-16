#include <QPainter>

#include "qgsexpression.h"
#include "qgsfielduiproperties.h"

bool QgsConditionalStyle::matchForFeature( QString fieldName, QgsFeature *feature, QgsFields fields )
{
  fieldName = QString( """%1""" ).arg( fieldName );
  QgsExpression exp( QString( rule ).replace( "@value", fieldName ) );
  return exp.evaluate( feature, fields ).toBool();
}

bool QgsConditionalStyle::matchForValue( QVariant value )
{
  QgsExpression exp( QString( rule ).replace( "@value", value.toString() ) );
  return exp.evaluate().toBool();
}

QPixmap QgsConditionalStyle::renderPreview()
{
  QPixmap pixmap( 64, 32 );
  QPainter painter( &pixmap );

  if ( backColor.isValid() )
    painter.setBrush( backColor );
  else
    painter.setBrush( QColor( Qt::white ) );

  QRect rect( 0, 0, 64, 32 );
  painter.drawRect( rect );

  if ( textColor.isValid() )
    painter.setPen( textColor );

  painter.drawText( rect, Qt::AlignCenter, "abc\n123" );
  painter.end();
  return pixmap;
}


QgsFieldUIProperties::QgsFieldUIProperties()
    : mStyles( QList<QgsConditionalStyle>() )
{}

void QgsFieldUIProperties::setConditionalStyles( QList<QgsConditionalStyle> styles )
{
  mStyles = styles;
}

QList<QgsConditionalStyle> QgsFieldUIProperties::getConditionalStyles()
{
  return mStyles;
}
