#include <QPainter>

#include "qgsconditionalstyle.h"
#include "qgsexpression.h"
#include "qgsfontutils.h"
#include "qgssymbollayerv2utils.h"
#include "qgsmarkersymbollayerv2.h"

QgsConditionalLayerStyles::QgsConditionalLayerStyles()
    : mRowStyles( QList<QgsConditionalStyle>() )
{}

QList<QgsConditionalStyle> QgsConditionalLayerStyles::rowStyles()
{
  return mRowStyles;
}

void QgsConditionalLayerStyles::setRowStyles( QList<QgsConditionalStyle> styles )
{
  mRowStyles = styles;
}

void QgsConditionalLayerStyles::setFieldStyles( QString fieldName, QList<QgsConditionalStyle> styles )
{
  mFieldStyles.insert( fieldName, styles );
}

QList<QgsConditionalStyle> QgsConditionalLayerStyles::fieldStyles( QString fieldName )
{
  if ( mFieldStyles.contains( fieldName ) )
  {
    return mFieldStyles[fieldName];
  }
  return QList<QgsConditionalStyle>();
}

bool QgsConditionalLayerStyles::writeXml( QDomNode &node, QDomDocument &doc ) const
{
  QDomElement stylesel = doc.createElement( "conditionalstyles" );
  QDomElement rowel = doc.createElement( "rowstyles" );
  foreach ( QgsConditionalStyle style, mRowStyles )
  {
    style.writeXml( rowel, doc );
  }

  stylesel.appendChild( rowel );

  QDomElement fieldsel = doc.createElement( "fieldstyles" );
  foreach ( const QString field, mFieldStyles.keys() )
  {
    QDomElement fieldel = doc.createElement( "fieldstyle" );
    fieldel.setAttribute( "fieldname", field );
    QgsConditionalStyles styles = mFieldStyles[field];
    foreach ( QgsConditionalStyle style, styles )
    {
      style.writeXml( fieldel, doc );
    }
    fieldsel.appendChild( fieldel );
  }

  stylesel.appendChild( fieldsel );

  node.appendChild( stylesel );
  return true;
}

bool QgsConditionalLayerStyles::readXml( const QDomNode &node )
{
  QDomElement condel = node.firstChildElement( "conditionalstyles" );
  mRowStyles.clear();
  mFieldStyles.clear();
  QDomElement rowstylesel = condel.firstChildElement( "rowstyles" );
  QDomNodeList nodelist = rowstylesel.toElement().elementsByTagName( "style" );
  for ( int i = 0;i < nodelist.count(); i++ )
  {
    QDomElement styleElm = nodelist.at( i ).toElement();
    QgsConditionalStyle style = QgsConditionalStyle();
    style.readXml( styleElm );
    mRowStyles.append( style );
  }

  QDomElement fieldstylesel = condel.firstChildElement( "fieldstyles" );
  nodelist = fieldstylesel.toElement().elementsByTagName( "fieldstyle" );
  QList<QgsConditionalStyle> styles;
  for ( int i = 0;i < nodelist.count(); i++ )
  {
    styles.clear();
    QDomElement fieldel = nodelist.at( i ).toElement();
    QString fieldName = fieldel.attribute( "fieldname" );
    QDomNodeList stylenodelist = fieldel.toElement().elementsByTagName( "style" );
    for ( int i = 0;i < stylenodelist.count(); i++ )
    {
      QDomElement styleElm = stylenodelist.at( i ).toElement();
      QgsConditionalStyle style = QgsConditionalStyle();
      style.readXml( styleElm );
      styles.append( style );
    }
    mFieldStyles.insert( fieldName, styles );
  }

  return true;
}

QgsConditionalStyle::QgsConditionalStyle()
    : mValid( false )
    , mSymbol( 0 )
    , mBackColor( QColor( 0, 0, 0, 0 ) )
    , mTextColor( Qt::black )
{}

QgsConditionalStyle::QgsConditionalStyle( QString rule )
    : mValid( false )
    , mSymbol( 0 )
    , mBackColor( QColor( 0, 0, 0, 0 ) )
    , mTextColor( Qt::black )
{
  setRule( rule );
}

QgsConditionalStyle::QgsConditionalStyle( const QgsConditionalStyle &other )
    : mValid( other.mValid )
    , mName( other.mName )
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

bool QgsConditionalStyle::matches( QVariant value, QgsExpressionContext& context ) const
{
  QgsExpression exp( mRule );
  context.lastScope()->setVariable( "value", value );
  return exp.evaluate( &context ).toBool();
}

QPixmap QgsConditionalStyle::renderPreview()
{
  QPixmap pixmap( 64, 32 );
  pixmap.fill( Qt::transparent );

  QPainter painter( &pixmap );

  if ( validBackgroundColor() )
    painter.setBrush( mBackColor );

  QRect rect = QRect( 0, 0, 64, 32 );
  painter.setPen( Qt::NoPen );
  painter.drawRect( rect );
  painter.drawPixmap( 8, 8, icon() );

  if ( validTextColor() )
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

bool QgsConditionalStyle::validBackgroundColor() const
{
  return ( backgroundColor().isValid() && backgroundColor().alpha() != 0 );
}

bool QgsConditionalStyle::validTextColor() const
{
  return ( textColor().isValid() && textColor().alpha() != 0 );
}

QList<QgsConditionalStyle> QgsConditionalStyle::matchingConditionalStyles( QList<QgsConditionalStyle> styles, QVariant value,  QgsExpressionContext& context )
{
  QList<QgsConditionalStyle> matchingstyles;
  foreach ( QgsConditionalStyle style, styles )
  {
    if ( style.matches( value, context ) )
      matchingstyles.append( style );
  }
  return matchingstyles;
}

QgsConditionalStyle QgsConditionalStyle::matchingConditionalStyle( QList<QgsConditionalStyle> styles, QVariant value,  QgsExpressionContext& context )
{
  foreach ( QgsConditionalStyle style, styles )
  {
    if ( style.matches( value, context ) )
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

