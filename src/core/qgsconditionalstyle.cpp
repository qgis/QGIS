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
    , mName( other.mName )
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
  mName = other.mName;
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

QString QgsConditionalStyle::displayText() const
{
  if ( name().isEmpty() )
    return rule();
  else
    return QString( "%1 \n%2" ).arg( name() ).arg( rule() );
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
  pixmap.fill( Qt::transparent );

  QPainter painter( &pixmap );

  if ( mBackColor.isValid() )
    painter.setBrush( mBackColor );

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

QList<QgsConditionalStyle> QgsConditionalStyle::matchingConditionalStyles( QList<QgsConditionalStyle> styles, QVariant value,  QgsFeature *feature )
{
  QList<QgsConditionalStyle> matchingstyles;
  foreach ( QgsConditionalStyle style, styles )
  {
    if ( style.matches( value, feature ) )
      matchingstyles.append( style );
  }
  return matchingstyles;
}

QgsConditionalStyle QgsConditionalStyle::matchingConditionalStyle( QList<QgsConditionalStyle> styles, QVariant value,  QgsFeature *feature )
{
  foreach ( QgsConditionalStyle style, styles )
  {
    if ( style.matches( value, feature ) )
      return style;
  }
  return QgsConditionalStyle();
}

QgsConditionalStyle QgsConditionalStyle::compressStyles( QList<QgsConditionalStyle> styles )
{
  QgsConditionalStyle style;
  foreach ( QgsConditionalStyle s, styles )
  {
    style.setFont( s.font() );
    if ( s.backgroundColor().isValid() && s.backgroundColor().alpha() != 0 )
      style.setBackgroundColor( s.backgroundColor() );
    if ( s.textColor().isValid() && s.textColor().alpha() != 0 )
      style.setTextColor( s.textColor() );
    if ( s.symbol() )
      style.setSymbol( s.symbol() );
  }
  return style;
}

bool QgsConditionalStyle::writeXml( QDomNode &node, QDomDocument &doc )
{
  QDomElement stylesel = doc.createElement( "style" );
  stylesel.setAttribute( "rule", mRule );
  stylesel.setAttribute( "name", mName );
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
  setName( styleElm.attribute( "name" ) );
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

