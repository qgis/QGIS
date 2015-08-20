#include <QPainter>

#include "qgsconditionalstyle.h"
#include "qgsexpression.h"
#include "qgsfontutils.h"
#include "qgssymbollayerv2utils.h"
#include "qgsmarkersymbollayerv2.h"

QgsConditionalStyle::QgsConditionalStyle()
    : mValid( false )
    , mSymbol( 0 )
{}

QgsConditionalStyle::QgsConditionalStyle( QString rule )
    : mValid( false )
    , mSymbol( 0 )
{
  setRule( rule );
}

QgsConditionalStyle::QgsConditionalStyle( const QgsConditionalStyle &other )
    : mValid( other.mValid )
    , mRule( other.mRule )
    , mFont( other.mFont )
    , mBackColor( other.mBackColor )
    , mTextColor( other.mTextColor )
    , mIcon( other.mIcon )
{
  if ( other.mSymbol.data() )
    mSymbol.reset( other.mSymbol->clone() );
}

QgsConditionalStyle& QgsConditionalStyle::operator=( const QgsConditionalStyle & other )
{
  mValid = other.mValid;
  mRule = other.mRule;
  mFont = other.mFont;
  mBackColor = other.mBackColor;
  mTextColor = other.mTextColor;
  mIcon = other.mIcon;
  if ( other.mSymbol.data() )
  {
    mSymbol.reset( other.mSymbol->clone() );
  }
  else
  {
    mSymbol.reset();
  }
  return ( *this );
}

QgsConditionalStyle::~QgsConditionalStyle()
{
}

void QgsConditionalStyle::setSymbol( QgsSymbolV2* value )
{
  mValid = true;
  if ( value )
  {
    mSymbol.reset( value->clone() );
    mIcon = QgsSymbolLayerV2Utils::symbolPreviewPixmap( mSymbol.data(), QSize( 16, 16 ) );
  }
  else
  {
    mSymbol.reset();
  }
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
  if ( ! mSymbol.isNull() )
  {
    QDomElement symbolElm = QgsSymbolLayerV2Utils::saveSymbol( "icon", mSymbol.data(), doc );
    stylesel.appendChild( symbolElm );
  }
  node.appendChild( stylesel );
  return true;
}

bool QgsConditionalStyle::readXml( const QDomNode &node )
{
  QDomElement styleElm = node.toElement();
  setRule( styleElm.attribute( "rule" ) );
  setBackgroundColor( QColor( styleElm.attribute( "background_color" ) ) );
  setTextColor( QColor( styleElm.attribute( "text_color" ) ) );
  QgsFontUtils::setFromXmlChildNode( mFont, styleElm, "font" );
  QDomElement symbolElm = styleElm.firstChildElement( "symbol" );
  if ( !symbolElm.isNull() )
  {
    QgsSymbolV2* symbol = QgsSymbolLayerV2Utils::loadSymbol<QgsMarkerSymbolV2>( symbolElm );
    setSymbol( symbol );
  }
  return true;
}

