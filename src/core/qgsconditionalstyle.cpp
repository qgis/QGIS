#include <QPainter>

#include "qgsconditionalstyle.h"
#include "qgsexpression.h"
#include "qgsfontutils.h"

QgsConditionalStyle::QgsConditionalStyle()
    : mValid( false )
{}

QgsConditionalStyle::QgsConditionalStyle( QString rule )
    : mValid( false )
{
  setRule( rule );
}

bool QgsConditionalStyle::matches( QVariant value, QgsFeature *feature )
{
  // TODO Replace with expression context
  QgsExpression exp( QString( mRule ).replace( "@value", value.toString() ) );
  if ( feature )
  {
    return exp.evaluate( feature, *feature->fields() ).toBool();
  }
  {
    return exp.evaluate().toBool();
  }
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
  painter.drawPixmap( 8, 8, icon() );

  if ( mTextColor.isValid() )
    painter.setPen( mTextColor );
  else
    painter.setPen( Qt::black );

  painter.setRenderHint( QPainter::Antialiasing );
  painter.setRenderHint( QPainter::HighQualityAntialiasing );
  painter.setFont( font() );
  rect = QRect( 32, 0, 32, 32 );
  painter.drawText( rect, Qt::AlignCenter, "abc\n123" );
  painter.end();
  return pixmap;
}

bool QgsConditionalStyle::writeXml( QDomNode &node, QDomDocument &doc )
{
  QDomElement stylesel = doc.createElement( "style" );
  stylesel.setAttribute( "rule", mRule );
  stylesel.setAttribute( "background_color", mBackColor.name() );
  stylesel.setAttribute( "text_color", mTextColor.name() );
  QDomElement labelFontElem = QgsFontUtils::toXmlElement( mFont, doc, "font" );
  stylesel.appendChild( labelFontElem );
  // TODO
  // stylesel.setAttribute( "icon", mRule );

  node.appendChild( stylesel );
}

bool QgsConditionalStyle::readXml( const QDomNode &node )
{
  QDomElement styleElm = node.toElement();
  setRule( styleElm.attribute( "rule" ) );
  setBackgroundColor( QColor( styleElm.attribute( "background_color" ) ) );
  setTextColor( QColor( styleElm.attribute( "text_color" ) ) );
  QgsFontUtils::setFromXmlChildNode( mFont, styleElm, "font" );
  // TODO Load symbol
}

