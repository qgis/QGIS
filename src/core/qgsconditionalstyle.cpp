/***************************************************************************
    qgsconditionalstyle.cpp
    ---------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsconditionalstyle.h"

#include "qgsexpression.h"
#include "qgsfontutils.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"

#include <QPainter>

#include "moc_qgsconditionalstyle.cpp"

QgsConditionalLayerStyles::QgsConditionalLayerStyles( QObject *parent )
  : QObject( parent )
{
}

QgsConditionalStyle QgsConditionalLayerStyles::constraintFailureStyles( QgsFieldConstraints::ConstraintStrength strength )
{
  switch ( strength )
  {
    case QgsFieldConstraints::ConstraintStrengthHard:
    {
      QgsConditionalStyle hardConstraintFailureStyle;
      hardConstraintFailureStyle.setBackgroundColor( QColor( 255, 152, 0 ) );
      hardConstraintFailureStyle.setTextColor( QColor( 0, 0, 0 ) );
      return hardConstraintFailureStyle;
    }

    case QgsFieldConstraints::ConstraintStrengthSoft:
    {
      QgsConditionalStyle softConstraintFailureStyle;
      softConstraintFailureStyle.setBackgroundColor( QColor( 255, 191, 12 ) );
      softConstraintFailureStyle.setTextColor( QColor( 0, 0, 0 ) );
      return softConstraintFailureStyle;
    }

    case QgsFieldConstraints::ConstraintStrengthNotSet:
      return QgsConditionalStyle();
  }

  return QgsConditionalStyle();
}

QgsConditionalStyles QgsConditionalLayerStyles::rowStyles() const
{
  return mRowStyles;
}

void QgsConditionalLayerStyles::setRowStyles( const QgsConditionalStyles &styles )
{
  if ( styles == mRowStyles )
    return;

  mRowStyles = styles;
  emit changed();
}

void QgsConditionalLayerStyles::setFieldStyles( const QString &fieldName, const QList<QgsConditionalStyle> &styles )
{
  if ( mFieldStyles.value( fieldName ) == styles )
    return;

  mFieldStyles.insert( fieldName, styles );
  emit changed();
}

QList<QgsConditionalStyle> QgsConditionalLayerStyles::fieldStyles( const QString &fieldName ) const
{
  return mFieldStyles.value( fieldName );
}

bool QgsConditionalLayerStyles::writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement stylesel = doc.createElement( u"conditionalstyles"_s );

  QDomElement rowel = doc.createElement( u"rowstyles"_s );
  const auto constMRowStyles = mRowStyles;
  for ( const QgsConditionalStyle &style : constMRowStyles )
  {
    style.writeXml( rowel, doc, context );
  }
  stylesel.appendChild( rowel );

  QDomElement fieldsel = doc.createElement( u"fieldstyles"_s );
  QHash<QString, QgsConditionalStyles>::const_iterator it = mFieldStyles.constBegin();
  for ( ; it != mFieldStyles.constEnd(); ++it )
  {
    QDomElement fieldel = doc.createElement( u"fieldstyle"_s );
    fieldel.setAttribute( u"fieldname"_s, it.key() );
    const QgsConditionalStyles styles = it.value();
    const auto constStyles = styles;
    for ( const QgsConditionalStyle &style : constStyles )
    {
      style.writeXml( fieldel, doc, context );
    }
    fieldsel.appendChild( fieldel );
  }
  stylesel.appendChild( fieldsel );

  node.appendChild( stylesel );
  return true;
}

bool QgsConditionalLayerStyles::rulesNeedGeometry() const
{
  for ( const QgsConditionalStyle &style : std::as_const( mRowStyles ) )
  {
    if ( QgsExpression( style.rule() ).needsGeometry() )
    {
      return true;
    }
  }
  return false;
}

bool QgsConditionalLayerStyles::readXml( const QDomNode &node, const QgsReadWriteContext &context )
{
  mRowStyles.clear();
  mFieldStyles.clear();

  const QDomElement condel = node.firstChildElement( u"conditionalstyles"_s );

  const QDomElement rowstylesel = condel.firstChildElement( u"rowstyles"_s );
  QDomNodeList nodelist = rowstylesel.toElement().elementsByTagName( u"style"_s );
  for ( int i = 0; i < nodelist.count(); i++ )
  {
    const QDomElement styleElm = nodelist.at( i ).toElement();
    QgsConditionalStyle style = QgsConditionalStyle();
    style.readXml( styleElm, context );
    mRowStyles.append( style );
  }

  const QDomElement fieldstylesel = condel.firstChildElement( u"fieldstyles"_s );
  nodelist = fieldstylesel.toElement().elementsByTagName( u"fieldstyle"_s );
  QList<QgsConditionalStyle> styles;
  for ( int i = 0; i < nodelist.count(); i++ )
  {
    styles.clear();
    const QDomElement fieldel = nodelist.at( i ).toElement();
    const QString fieldName = fieldel.attribute( u"fieldname"_s );
    const QDomNodeList stylenodelist = fieldel.toElement().elementsByTagName( u"style"_s );
    styles.reserve( stylenodelist.count() );
    for ( int j = 0; j < stylenodelist.count(); j++ )
    {
      const QDomElement styleElm = stylenodelist.at( j ).toElement();
      QgsConditionalStyle style = QgsConditionalStyle();
      style.readXml( styleElm, context );
      styles.append( style );
    }
    mFieldStyles.insert( fieldName, styles );
  }

  return true;
}

QgsConditionalStyle::QgsConditionalStyle()
{}

QgsConditionalStyle::QgsConditionalStyle( const QString &rule )
{
  setRule( rule );
}

QgsConditionalStyle::~QgsConditionalStyle() = default;

QgsConditionalStyle::QgsConditionalStyle( const QgsConditionalStyle &other )
//****** IMPORTANT! editing this? make sure you update the move constructor too! *****
  : mValid( other.mValid )
  , mName( other.mName )
  , mRule( other.mRule )
  , mFont( other.mFont )
  , mBackColor( other.mBackColor )
  , mTextColor( other.mTextColor )
  , mIcon( other.mIcon )
    //****** IMPORTANT! editing this? make sure you update the move constructor too! *****
{
  if ( other.mSymbol )
    mSymbol.reset( other.mSymbol->clone() );
}

QgsConditionalStyle::QgsConditionalStyle( QgsConditionalStyle &&other )
  : mValid( other.mValid )
  , mName( std::move( other.mName ) )
  , mRule( std::move( other.mRule ) )
  , mSymbol( std::move( other.mSymbol ) )
  , mFont( std::move( other.mFont ) )
  , mBackColor( std::move( other.mBackColor ) )
  , mTextColor( std::move( other.mTextColor ) )
  , mIcon( std::move( other.mIcon ) )
{

}

QgsConditionalStyle &QgsConditionalStyle::operator=( const QgsConditionalStyle &other )
{
  if ( &other == this )
    return *this;

  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  mValid = other.mValid;
  mRule = other.mRule;
  mFont = other.mFont;
  mBackColor = other.mBackColor;
  mTextColor = other.mTextColor;
  mIcon = other.mIcon;
  mName = other.mName;
  if ( other.mSymbol )
  {
    mSymbol.reset( other.mSymbol->clone() );
  }
  else
  {
    mSymbol.reset();
  }
  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  return ( *this );
}

QgsConditionalStyle &QgsConditionalStyle::operator=( QgsConditionalStyle &&other )
{
  if ( &other == this )
    return *this;

  mValid = other.mValid;
  mRule = std::move( other.mRule );
  mFont = std::move( other.mFont );
  mBackColor = std::move( other.mBackColor );
  mTextColor = std::move( other.mTextColor );
  mIcon = std::move( other.mIcon );
  mName = std::move( other.mName );
  mSymbol = std::move( other.mSymbol );
  return ( *this );
}

QString QgsConditionalStyle::displayText() const
{
  if ( name().isEmpty() )
    return rule();
  else
    return u"%1 \n%2"_s.arg( name(), rule() );
}

void QgsConditionalStyle::setSymbol( QgsSymbol *value )
{
  mValid = true;
  if ( value )
  {
    mSymbol.reset( value->clone() );
    mIcon = QgsSymbolLayerUtils::symbolPreviewPixmap( mSymbol.get(), QSize( 16, 16 ) );
  }
  else
  {
    mSymbol.reset();
  }
}

bool QgsConditionalStyle::matches( const QVariant &value, QgsExpressionContext &context ) const
{
  QgsExpression exp( mRule );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( u"value"_s, value, true ) );
  return exp.evaluate( &context ).toBool();
}

QPixmap QgsConditionalStyle::renderPreview( const QSize &size ) const
{
  QPixmap pixmap( size.isValid() ? size.width() : 64, size.isValid() ? size.height() : 32 );
  pixmap.fill( Qt::transparent );

  QPainter painter( &pixmap );

  if ( validBackgroundColor() )
    painter.setBrush( mBackColor );

  QRect rect = QRect( 0, 0, pixmap.width(), pixmap.height() );
  painter.setPen( Qt::NoPen );
  painter.drawRect( rect );
  const QPixmap symbolIcon = icon();
  if ( !symbolIcon.isNull() )
  {
    painter.drawPixmap( ( pixmap.width() / 3 - symbolIcon.width() ) / 2, ( pixmap.height() - symbolIcon.height() ) / 2, symbolIcon );
  }

  if ( validTextColor() )
    painter.setPen( mTextColor );
  else
    painter.setPen( Qt::black );

  painter.setRenderHint( QPainter::Antialiasing );
  painter.setFont( font() );
  rect = QRect( pixmap.width() / 3, 0, 2 * pixmap.width() / 3, pixmap.height() );
  painter.drawText( rect, Qt::AlignCenter, u"abc\n123"_s );
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

QList<QgsConditionalStyle> QgsConditionalStyle::matchingConditionalStyles( const QList<QgsConditionalStyle> &styles, const QVariant &value, QgsExpressionContext &context )
{
  QList<QgsConditionalStyle> matchingstyles;
  const auto constStyles = styles;
  for ( const QgsConditionalStyle &style : constStyles )
  {
    if ( style.matches( value, context ) )
      matchingstyles.append( style );
  }
  return matchingstyles;
}

QgsConditionalStyle QgsConditionalStyle::matchingConditionalStyle( const QList<QgsConditionalStyle> &styles, const QVariant &value,  QgsExpressionContext &context )
{
  const auto constStyles = styles;
  for ( const QgsConditionalStyle &style : constStyles )
  {
    if ( style.matches( value, context ) )
      return style;
  }
  return QgsConditionalStyle();
}

QgsConditionalStyle QgsConditionalStyle::compressStyles( const QList<QgsConditionalStyle> &styles )
{
  QgsConditionalStyle style;
  for ( const QgsConditionalStyle &s : styles )
  {
    if ( !s.isValid() )
      continue;

    style.setFont( s.font() );
    if ( s.backgroundColor().isValid() && s.backgroundColor().alpha() != 0 )
      style.setBackgroundColor( s.backgroundColor() );
    if ( s.textColor().isValid() && s.textColor().alpha() != 0 )
      style.setTextColor( s.textColor() );
    if ( auto *lSymbol = s.symbol() )
      style.setSymbol( lSymbol );
  }
  return style;
}

bool QgsConditionalStyle::writeXml( QDomNode &node, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement stylesel = doc.createElement( u"style"_s );
  stylesel.setAttribute( u"rule"_s, mRule );
  stylesel.setAttribute( u"name"_s, mName );
  if ( mBackColor.isValid() )
  {
    stylesel.setAttribute( u"background_color"_s, mBackColor.name() );
    stylesel.setAttribute( u"background_color_alpha"_s, mBackColor.alpha() );
  }
  if ( mTextColor.isValid() )
  {
    stylesel.setAttribute( u"text_color"_s, mTextColor.name() );
    stylesel.setAttribute( u"text_color_alpha"_s, mTextColor.alpha() );
  }
  const QDomElement labelFontElem = QgsFontUtils::toXmlElement( mFont, doc, u"font"_s );
  stylesel.appendChild( labelFontElem );
  if ( mSymbol )
  {
    const QDomElement symbolElm = QgsSymbolLayerUtils::saveSymbol( u"icon"_s, mSymbol.get(), doc, context );
    stylesel.appendChild( symbolElm );
  }
  node.appendChild( stylesel );
  return true;
}

bool QgsConditionalStyle::operator==( const QgsConditionalStyle &other ) const
{
  return mValid == other.mValid
         && mName == other.mName
         && mRule == other.mRule
         && mFont == other.mFont
         && mBackColor == other.mBackColor
         && mTextColor == other.mTextColor
         && static_cast< bool >( mSymbol ) == static_cast< bool >( other.mSymbol )
         && ( ! mSymbol || QgsSymbolLayerUtils::symbolProperties( mSymbol.get() ) == QgsSymbolLayerUtils::symbolProperties( other.mSymbol.get() ) );
}

bool QgsConditionalStyle::operator!=( const QgsConditionalStyle &other ) const
{
  return !( *this == other );
}

bool QgsConditionalStyle::readXml( const QDomNode &node, const QgsReadWriteContext &context )
{
  const QDomElement styleElm = node.toElement();
  setRule( styleElm.attribute( u"rule"_s ) );
  setName( styleElm.attribute( u"name"_s ) );
  if ( styleElm.hasAttribute( u"background_color"_s ) )
  {
    QColor bColor = QColor( styleElm.attribute( u"background_color"_s ) );
    if ( styleElm.hasAttribute( u"background_color_alpha"_s ) )
    {
      bColor.setAlpha( styleElm.attribute( u"background_color_alpha"_s ).toInt() );
    }
    if ( bColor.alpha() == 0 )
      setBackgroundColor( QColor() );
    else
      setBackgroundColor( bColor );
  }
  else
  {
    setBackgroundColor( QColor() );
  }
  if ( styleElm.hasAttribute( u"text_color"_s ) )
  {
    QColor tColor = QColor( styleElm.attribute( u"text_color"_s ) );
    if ( styleElm.hasAttribute( u"text_color_alpha"_s ) )
    {
      tColor.setAlpha( styleElm.attribute( u"text_color_alpha"_s ).toInt() );
    }
    if ( tColor.alpha() == 0 )
      setTextColor( QColor() );
    else
      setTextColor( tColor );
  }
  else
  {
    setTextColor( QColor() );
  }
  QgsFontUtils::setFromXmlChildNode( mFont, styleElm, u"font"_s );
  const QDomElement symbolElm = styleElm.firstChildElement( u"symbol"_s );
  if ( !symbolElm.isNull() )
  {
    std::unique_ptr< QgsSymbol > symbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElm, context );
    setSymbol( symbol.release() );
  }
  return true;
}

