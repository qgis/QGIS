/***************************************************************************
  qgstextformat.cpp
  ---------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextformat.h"
#include "qgstextrenderer_p.h"
#include "qgstextrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsfontutils.h"
#include "qgssymbollayerutils.h"
#include "qgspainting.h"
#include "qgstextrendererutils.h"
#include "qgspallabeling.h"
#include "qgsconfig.h"
#include <QFontDatabase>
#include <QMimeData>
#include <QWidget>
#include <QScreen>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif

QgsTextFormat::QgsTextFormat()
{
  d = new QgsTextSettingsPrivate();
}

QgsTextFormat::QgsTextFormat( const QgsTextFormat &other ) //NOLINT
  : mBufferSettings( other.mBufferSettings )
  , mBackgroundSettings( other.mBackgroundSettings )
  , mShadowSettings( other.mShadowSettings )
  , mMaskSettings( other.mMaskSettings )
  , mTextFontFamily( other.mTextFontFamily )
  , mTextFontFound( other.mTextFontFound )
  , d( other.d )
{

}

QgsTextFormat &QgsTextFormat::operator=( const QgsTextFormat &other )  //NOLINT
{
  d = other.d;
  mBufferSettings = other.mBufferSettings;
  mBackgroundSettings = other.mBackgroundSettings;
  mShadowSettings = other.mShadowSettings;
  mMaskSettings = other.mMaskSettings;
  mTextFontFamily = other.mTextFontFamily;
  mTextFontFound = other.mTextFontFound;
  return *this;
}

QgsTextFormat::~QgsTextFormat() //NOLINT
{

}

bool QgsTextFormat::operator==( const QgsTextFormat &other ) const
{
  if ( d->isValid != other.isValid()
       || d->textFont != other.font()
       || namedStyle() != other.namedStyle()
       || d->fontSizeUnits != other.sizeUnit()
       || d->fontSizeMapUnitScale != other.sizeMapUnitScale()
       || d->fontSize != other.size()
       || d->textColor != other.color()
       || d->opacity != other.opacity()
       || d->blendMode != other.blendMode()
       || d->multilineHeight != other.lineHeight()
       || d->orientation != other.orientation()
       || d->previewBackgroundColor != other.previewBackgroundColor()
       || d->allowHtmlFormatting != other.allowHtmlFormatting()
       || d->forcedBold != other.forcedBold()
       || d->forcedItalic != other.forcedItalic()
       || d->capitalization != other.capitalization()
       || mBufferSettings != other.mBufferSettings
       || mBackgroundSettings != other.mBackgroundSettings
       || mShadowSettings != other.mShadowSettings
       || mMaskSettings != other.mMaskSettings
       || d->families != other.families()
       || d->mDataDefinedProperties != other.dataDefinedProperties() )
    return false;

  return true;
}

bool QgsTextFormat::operator!=( const QgsTextFormat &other ) const
{
  return !( *this == other );
}

bool QgsTextFormat::isValid() const
{
  return d->isValid;
}

void QgsTextFormat::setValid()
{
  d->isValid = true;
}

QgsTextBufferSettings &QgsTextFormat::buffer()
{
  d->isValid = true;
  return mBufferSettings;
}

void QgsTextFormat::setBuffer( const QgsTextBufferSettings &bufferSettings )
{
  d->isValid = true;
  mBufferSettings = bufferSettings;
}

QgsTextBackgroundSettings &QgsTextFormat::background()
{
  d->isValid = true;
  return mBackgroundSettings;
}

void QgsTextFormat::setBackground( const QgsTextBackgroundSettings &backgroundSettings )
{
  d->isValid = true;
  mBackgroundSettings = backgroundSettings;
}

QgsTextShadowSettings &QgsTextFormat::shadow()
{
  d->isValid = true;
  return mShadowSettings;
}

void QgsTextFormat::setShadow( const QgsTextShadowSettings &shadowSettings )
{
  d->isValid = true;
  mShadowSettings = shadowSettings;
}

QgsTextMaskSettings &QgsTextFormat::mask()
{
  d->isValid = true;
  return mMaskSettings;
}

void QgsTextFormat::setMask( const QgsTextMaskSettings &maskSettings )
{
  d->isValid = true;
  mMaskSettings = maskSettings;
}

QFont QgsTextFormat::font() const
{
  return d->textFont;
}

QFont QgsTextFormat::scaledFont( const QgsRenderContext &context, double scaleFactor, bool *isZeroSize ) const
{
  if ( isZeroSize )
    *isZeroSize = false;

  QFont font = d->textFont;
  if ( scaleFactor == 1 )
  {
    int fontPixelSize = QgsTextRenderer::sizeToPixel( d->fontSize, context, d->fontSizeUnits,
                        d->fontSizeMapUnitScale );
    if ( fontPixelSize == 0 )
    {
      if ( isZeroSize )
        *isZeroSize = true;
      return QFont();
    }

    font.setPixelSize( fontPixelSize );
  }
  else
  {
    double fontPixelSize = context.convertToPainterUnits( d->fontSize, d->fontSizeUnits, d->fontSizeMapUnitScale );
    if ( qgsDoubleNear( fontPixelSize, 0 ) )
    {
      if ( isZeroSize )
        *isZeroSize = true;
      return QFont();
    }
    const int roundedPixelSize = static_cast< int >( std::round( scaleFactor * fontPixelSize + 0.5 ) );
    font.setPixelSize( roundedPixelSize );
  }

  font.setLetterSpacing( QFont::AbsoluteSpacing, context.convertToPainterUnits( d->textFont.letterSpacing(), d->fontSizeUnits, d->fontSizeMapUnitScale ) * scaleFactor );
  font.setWordSpacing( context.convertToPainterUnits( d->textFont.wordSpacing(), d->fontSizeUnits, d->fontSizeMapUnitScale ) * scaleFactor  * scaleFactor );

  if ( d->capitalization == Qgis::Capitalization::SmallCaps
       || d->capitalization == Qgis::Capitalization::AllSmallCaps )
    font.setCapitalization( QFont::SmallCaps );

  return font;
}

void QgsTextFormat::setFont( const QFont &font )
{
  d->isValid = true;
  d->textFont = font;
}

QString QgsTextFormat::namedStyle() const
{
  if ( !d->textNamedStyle.isEmpty() )
    return d->textNamedStyle;

  QFontDatabase db;
  return db.styleString( d->textFont );
}

void QgsTextFormat::setNamedStyle( const QString &style )
{
  d->isValid = true;
  QgsFontUtils::updateFontViaStyle( d->textFont, style );
  d->textNamedStyle = style;
}

bool QgsTextFormat::forcedBold() const
{
  return d->forcedBold;
}

void QgsTextFormat::setForcedBold( bool forced )
{
  d->isValid = true;
  d->textFont.setBold( forced );
  d->forcedBold = true;
}

bool QgsTextFormat::forcedItalic() const
{
  return d->forcedItalic;
}

void QgsTextFormat::setForcedItalic( bool forced )
{
  d->isValid = true;
  d->textFont.setItalic( forced );
  d->forcedItalic = true;
}

QStringList QgsTextFormat::families() const
{
  return d->families;
}

void QgsTextFormat::setFamilies( const QStringList &families )
{
  d->isValid = true;
  d->families = families;
}

QgsUnitTypes::RenderUnit QgsTextFormat::sizeUnit() const
{
  return d->fontSizeUnits;
}

void QgsTextFormat::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  d->isValid = true;
  d->fontSizeUnits = unit;
}

QgsMapUnitScale QgsTextFormat::sizeMapUnitScale() const
{
  return d->fontSizeMapUnitScale;
}

void QgsTextFormat::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  d->isValid = true;
  d->fontSizeMapUnitScale = scale;
}

double QgsTextFormat::size() const
{
  return d->fontSize;
}

void QgsTextFormat::setSize( double size )
{
  d->isValid = true;
  d->fontSize = size;
}

QColor QgsTextFormat::color() const
{
  return d->textColor;
}

void QgsTextFormat::setColor( const QColor &color )
{
  d->isValid = true;
  d->textColor = color;
}

double QgsTextFormat::opacity() const
{
  return d->opacity;
}

void QgsTextFormat::setOpacity( double opacity )
{
  d->isValid = true;
  d->opacity = opacity;
}

int QgsTextFormat::stretchFactor() const
{
  return d->textFont.stretch() > 0 ? d->textFont.stretch() : 100;
}

void QgsTextFormat::setStretchFactor( int factor )
{
  d->isValid = true;
  d->textFont.setStretch( factor );
}

QPainter::CompositionMode QgsTextFormat::blendMode() const
{
  return d->blendMode;
}

void QgsTextFormat::setBlendMode( QPainter::CompositionMode mode )
{
  d->isValid = true;
  d->blendMode = mode;
}

double QgsTextFormat::lineHeight() const
{
  return d->multilineHeight;
}

void QgsTextFormat::setLineHeight( double height )
{
  d->isValid = true;
  d->multilineHeight = height;
}

QgsTextFormat::TextOrientation QgsTextFormat::orientation() const
{
  return d->orientation;
}

void QgsTextFormat::setOrientation( TextOrientation orientation )
{
  d->isValid = true;
  d->orientation = orientation;
}

Qgis::Capitalization QgsTextFormat::capitalization() const
{
  // bit of complexity here to maintain API..
  return d->capitalization == Qgis::Capitalization::MixedCase && d->textFont.capitalization() != QFont::MixedCase
         ? static_cast< Qgis::Capitalization >( d->textFont.capitalization() )
         : d->capitalization ;
}

void QgsTextFormat::setCapitalization( Qgis::Capitalization capitalization )
{
  d->isValid = true;
  d->capitalization = capitalization;
#if defined(HAS_KDE_QT5_SMALL_CAPS_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  d->textFont.setCapitalization( capitalization == Qgis::Capitalization::SmallCaps || capitalization == Qgis::Capitalization::AllSmallCaps ? QFont::SmallCaps : QFont::MixedCase );
#else
  d->textFont.setCapitalization( QFont::MixedCase );
#endif
}

bool QgsTextFormat::allowHtmlFormatting() const
{
  return d->allowHtmlFormatting;
}

void QgsTextFormat::setAllowHtmlFormatting( bool allow )
{
  d->isValid = true;
  d->allowHtmlFormatting = allow;
}

QColor QgsTextFormat::previewBackgroundColor() const
{
  return d->previewBackgroundColor;
}

void QgsTextFormat::setPreviewBackgroundColor( const QColor &color )
{
  d->isValid = true;
  d->previewBackgroundColor = color;
}

void QgsTextFormat::readFromLayer( QgsVectorLayer *layer )
{
  d->isValid = true;
  QFont appFont = QApplication::font();
  mTextFontFamily = layer->customProperty( QStringLiteral( "labeling/fontFamily" ), QVariant( appFont.family() ) ).toString();
  QString fontFamily = mTextFontFamily;
  if ( mTextFontFamily != appFont.family() && !QgsFontUtils::fontFamilyMatchOnSystem( mTextFontFamily ) )
  {
    // trigger to notify about font family substitution
    mTextFontFound = false;

    // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
    // currently only defaults to matching algorithm for resolving [foundry], if a font of similar family is found (default for QFont)

    // for now, do not use matching algorithm for substitution if family not found, substitute default instead
    fontFamily = appFont.family();
  }
  else
  {
    mTextFontFound = true;
  }

  if ( !layer->customProperty( QStringLiteral( "labeling/fontSize" ) ).isValid() )
  {
    d->fontSize = appFont.pointSizeF();
  }
  else
  {
    d->fontSize = layer->customProperty( QStringLiteral( "labeling/fontSize" ) ).toDouble();
  }

  if ( layer->customProperty( QStringLiteral( "labeling/fontSizeUnit" ) ).toString().isEmpty() )
  {
    d->fontSizeUnits = layer->customProperty( QStringLiteral( "labeling/fontSizeInMapUnits" ), QVariant( false ) ).toBool() ?
                       QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderPoints;
  }
  else
  {
    bool ok = false;
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/fontSizeUnit" ) ).toString(), &ok );
    if ( !ok )
      d->fontSizeUnits = QgsUnitTypes::RenderPoints;
  }
  if ( layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->fontSizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitMaxScale" ), 0.0 ).toDouble();
    d->fontSizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitScale" ) ).toString() );
  }
  int fontWeight = layer->customProperty( QStringLiteral( "labeling/fontWeight" ) ).toInt();
  bool fontItalic = layer->customProperty( QStringLiteral( "labeling/fontItalic" ) ).toBool();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( layer->customProperty( QStringLiteral( "labeling/namedStyle" ), QVariant( "" ) ).toString() );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->capitalization = static_cast< Qgis::Capitalization >( layer->customProperty( QStringLiteral( "labeling/fontCapitals" ), QVariant( 0 ) ).toUInt() );
  d->textFont.setUnderline( layer->customProperty( QStringLiteral( "labeling/fontUnderline" ) ).toBool() );
  d->textFont.setStrikeOut( layer->customProperty( QStringLiteral( "labeling/fontStrikeout" ) ).toBool() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( QStringLiteral( "labeling/fontLetterSpacing" ), QVariant( 0.0 ) ).toDouble() );
  d->textFont.setWordSpacing( layer->customProperty( QStringLiteral( "labeling/fontWordSpacing" ), QVariant( 0.0 ) ).toDouble() );
  d->textColor = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/textColor" ), Qt::black, false );
  if ( layer->customProperty( QStringLiteral( "labeling/textOpacity" ) ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( QStringLiteral( "labeling/textTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( QStringLiteral( "labeling/textOpacity" ) ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/blendMode" ), QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->multilineHeight = layer->customProperty( QStringLiteral( "labeling/multilineHeight" ), QVariant( 1.0 ) ).toDouble();
  d->previewBackgroundColor = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/previewBkgrdColor" ), QColor( 255, 255, 255 ), false );

  mBufferSettings.readFromLayer( layer );
  mShadowSettings.readFromLayer( layer );
  mBackgroundSettings.readFromLayer( layer );
}

void QgsTextFormat::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  d->isValid = true;
  QDomElement textStyleElem;
  if ( elem.nodeName() == QLatin1String( "text-style" ) )
    textStyleElem = elem;
  else
    textStyleElem = elem.firstChildElement( QStringLiteral( "text-style" ) );
  QFont appFont = QApplication::font();
  mTextFontFamily = textStyleElem.attribute( QStringLiteral( "fontFamily" ), appFont.family() );
  QString fontFamily = mTextFontFamily;

  const QDomElement familiesElem = textStyleElem.firstChildElement( QStringLiteral( "families" ) );
  const QDomNodeList familyNodes = familiesElem.childNodes();
  QStringList families;
  families.reserve( familyNodes.size() );
  for ( int i = 0; i < familyNodes.count(); ++i )
  {
    const QDomElement familyElem = familyNodes.at( i ).toElement();
    families << familyElem.attribute( QStringLiteral( "name" ) );
  }
  d->families = families;

  mTextFontFound = false;
  if ( mTextFontFamily != appFont.family() && !QgsFontUtils::fontFamilyMatchOnSystem( mTextFontFamily ) )
  {
    for ( const QString &family : std::as_const( families ) )
    {
      if ( QgsFontUtils::fontFamilyMatchOnSystem( family ) )
      {
        mTextFontFound = true;
        fontFamily = family;
        break;
      }
    }

    if ( !mTextFontFound )
    {
      // couldn't even find a matching font in the backup list -- substitute default instead
      fontFamily = appFont.family();
    }
  }
  else
  {
    mTextFontFound = true;
  }

  if ( !mTextFontFound )
  {
    context.pushMessage( QObject::tr( "Font “%1” not available on system" ).arg( mTextFontFamily ) );
  }

  if ( textStyleElem.hasAttribute( QStringLiteral( "fontSize" ) ) )
  {
    d->fontSize = textStyleElem.attribute( QStringLiteral( "fontSize" ) ).toDouble();
  }
  else
  {
    d->fontSize = appFont.pointSizeF();
  }

  if ( !textStyleElem.hasAttribute( QStringLiteral( "fontSizeUnit" ) ) )
  {
    d->fontSizeUnits = textStyleElem.attribute( QStringLiteral( "fontSizeInMapUnits" ) ).toUInt() == 0 ? QgsUnitTypes::RenderPoints
                       : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( textStyleElem.attribute( QStringLiteral( "fontSizeUnit" ) ) );
  }

  if ( !textStyleElem.hasAttribute( QStringLiteral( "fontSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->fontSizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->fontSizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitScale" ) ) );
  }
  int fontWeight = textStyleElem.attribute( QStringLiteral( "fontWeight" ) ).toInt();
  bool fontItalic = textStyleElem.attribute( QStringLiteral( "fontItalic" ) ).toInt();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textFont.setPointSizeF( d->fontSize ); //double precision needed because of map units
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( textStyleElem.attribute( QStringLiteral( "namedStyle" ) ) );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->forcedBold = textStyleElem.attribute( QStringLiteral( "forcedBold" ) ).toInt();
  d->forcedItalic = textStyleElem.attribute( QStringLiteral( "forcedItalic" ) ).toInt();
  d->textFont.setUnderline( textStyleElem.attribute( QStringLiteral( "fontUnderline" ) ).toInt() );
  d->textFont.setStrikeOut( textStyleElem.attribute( QStringLiteral( "fontStrikeout" ) ).toInt() );
  d->textFont.setKerning( textStyleElem.attribute( QStringLiteral( "fontKerning" ), QStringLiteral( "1" ) ).toInt() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, textStyleElem.attribute( QStringLiteral( "fontLetterSpacing" ), QStringLiteral( "0" ) ).toDouble() );
  d->textFont.setWordSpacing( textStyleElem.attribute( QStringLiteral( "fontWordSpacing" ), QStringLiteral( "0" ) ).toDouble() );
  d->textColor = QgsSymbolLayerUtils::decodeColor( textStyleElem.attribute( QStringLiteral( "textColor" ), QgsSymbolLayerUtils::encodeColor( Qt::black ) ) );
  if ( !textStyleElem.hasAttribute( QStringLiteral( "textOpacity" ) ) )
  {
    d->opacity = ( 1 - textStyleElem.attribute( QStringLiteral( "textTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( textStyleElem.attribute( QStringLiteral( "textOpacity" ) ).toDouble() );
  }
#ifdef HAS_KDE_QT5_FONT_STRETCH_FIX
  d->textFont.setStretch( textStyleElem.attribute( QStringLiteral( "stretchFactor" ), QStringLiteral( "100" ) ).toInt() );
#endif
  d->orientation = QgsTextRendererUtils::decodeTextOrientation( textStyleElem.attribute( QStringLiteral( "textOrientation" ) ) );
  d->previewBackgroundColor = QgsSymbolLayerUtils::decodeColor( textStyleElem.attribute( QStringLiteral( "previewBkgrdColor" ), QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( textStyleElem.attribute( QStringLiteral( "blendMode" ), QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );

  if ( !textStyleElem.hasAttribute( QStringLiteral( "multilineHeight" ) ) )
  {
    QDomElement textFormatElem = elem.firstChildElement( QStringLiteral( "text-format" ) );
    d->multilineHeight = textFormatElem.attribute( QStringLiteral( "multilineHeight" ), QStringLiteral( "1" ) ).toDouble();
  }
  else
  {
    d->multilineHeight = textStyleElem.attribute( QStringLiteral( "multilineHeight" ), QStringLiteral( "1" ) ).toDouble();
  }

  if ( textStyleElem.hasAttribute( QStringLiteral( "capitalization" ) ) )
    d->capitalization = static_cast< Qgis::Capitalization >( textStyleElem.attribute( QStringLiteral( "capitalization" ), QString::number( static_cast< int >( Qgis::Capitalization::MixedCase ) ) ).toInt() );
  else
    d->capitalization = static_cast< Qgis::Capitalization >( textStyleElem.attribute( QStringLiteral( "fontCapitals" ), QStringLiteral( "0" ) ).toUInt() );

  if ( d->capitalization == Qgis::Capitalization::SmallCaps || d->capitalization == Qgis::Capitalization::AllSmallCaps )
    d->textFont.setCapitalization( QFont::SmallCaps );

  d->allowHtmlFormatting = textStyleElem.attribute( QStringLiteral( "allowHtml" ), QStringLiteral( "0" ) ).toInt();

  if ( textStyleElem.firstChildElement( QStringLiteral( "text-buffer" ) ).isNull() )
  {
    mBufferSettings.readXml( elem );
  }
  else
  {
    mBufferSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "text-mask" ) ).isNull() )
  {
    mMaskSettings.readXml( elem );
  }
  else
  {
    mMaskSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "shadow" ) ).isNull() )
  {
    mShadowSettings.readXml( elem );
  }
  else
  {
    mShadowSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "background" ) ).isNull() )
  {
    mBackgroundSettings.readXml( elem, context );
  }
  else
  {
    mBackgroundSettings.readXml( textStyleElem, context );
  }

  QDomElement ddElem = textStyleElem.firstChildElement( QStringLiteral( "dd_properties" ) );
  if ( ddElem.isNull() )
  {
    ddElem = elem.firstChildElement( QStringLiteral( "dd_properties" ) );
  }
  if ( !ddElem.isNull() )
  {
    d->mDataDefinedProperties.readXml( ddElem, QgsPalLayerSettings::propertyDefinitions() );
    mBackgroundSettings.upgradeDataDefinedProperties( d->mDataDefinedProperties );
  }
  else
  {
    d->mDataDefinedProperties.clear();
  }
}

QDomElement QgsTextFormat::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  // text style
  QDomElement textStyleElem = doc.createElement( QStringLiteral( "text-style" ) );
  textStyleElem.setAttribute( QStringLiteral( "fontFamily" ), d->textFont.family() );

  QDomElement familiesElem = doc.createElement( QStringLiteral( "families" ) );
  for ( const QString &family : std::as_const( d->families ) )
  {
    QDomElement familyElem = doc.createElement( QStringLiteral( "family" ) );
    familyElem.setAttribute( QStringLiteral( "name" ), family );
    familiesElem.appendChild( familyElem );
  }
  textStyleElem.appendChild( familiesElem );

  textStyleElem.setAttribute( QStringLiteral( "namedStyle" ), QgsFontUtils::untranslateNamedStyle( d->textNamedStyle ) );
  textStyleElem.setAttribute( QStringLiteral( "fontSize" ), d->fontSize );
  textStyleElem.setAttribute( QStringLiteral( "fontSizeUnit" ), QgsUnitTypes::encodeUnit( d->fontSizeUnits ) );
  textStyleElem.setAttribute( QStringLiteral( "fontSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->fontSizeMapUnitScale ) );
  textStyleElem.setAttribute( QStringLiteral( "fontWeight" ), d->textFont.weight() );
  textStyleElem.setAttribute( QStringLiteral( "fontItalic" ), d->textFont.italic() );
  textStyleElem.setAttribute( QStringLiteral( "fontStrikeout" ), d->textFont.strikeOut() );
  textStyleElem.setAttribute( QStringLiteral( "fontUnderline" ), d->textFont.underline() );
  textStyleElem.setAttribute( QStringLiteral( "forcedBold" ), d->forcedBold );
  textStyleElem.setAttribute( QStringLiteral( "forcedItalic" ), d->forcedItalic );
  textStyleElem.setAttribute( QStringLiteral( "textColor" ), QgsSymbolLayerUtils::encodeColor( d->textColor ) );
  textStyleElem.setAttribute( QStringLiteral( "previewBkgrdColor" ), QgsSymbolLayerUtils::encodeColor( d->previewBackgroundColor ) );
  textStyleElem.setAttribute( QStringLiteral( "fontLetterSpacing" ), d->textFont.letterSpacing() );
  textStyleElem.setAttribute( QStringLiteral( "fontWordSpacing" ), d->textFont.wordSpacing() );
  textStyleElem.setAttribute( QStringLiteral( "fontKerning" ), d->textFont.kerning() );
  textStyleElem.setAttribute( QStringLiteral( "textOpacity" ), d->opacity );
#ifdef HAS_KDE_QT5_FONT_STRETCH_FIX
  if ( d->textFont.stretch() > 0 )
    textStyleElem.setAttribute( QStringLiteral( "stretchFactor" ), d->textFont.stretch() );
#endif
  textStyleElem.setAttribute( QStringLiteral( "textOrientation" ), QgsTextRendererUtils::encodeTextOrientation( d->orientation ) );
  textStyleElem.setAttribute( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( d->blendMode ) );
  textStyleElem.setAttribute( QStringLiteral( "multilineHeight" ), d->multilineHeight );
  textStyleElem.setAttribute( QStringLiteral( "allowHtml" ), d->allowHtmlFormatting ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  textStyleElem.setAttribute( QStringLiteral( "capitalization" ), QString::number( static_cast< int >( d->capitalization ) ) );

  QDomElement ddElem = doc.createElement( QStringLiteral( "dd_properties" ) );
  d->mDataDefinedProperties.writeXml( ddElem, QgsPalLayerSettings::propertyDefinitions() );

  textStyleElem.appendChild( mBufferSettings.writeXml( doc ) );
  textStyleElem.appendChild( mMaskSettings.writeXml( doc ) );
  textStyleElem.appendChild( mBackgroundSettings.writeXml( doc, context ) );
  textStyleElem.appendChild( mShadowSettings.writeXml( doc ) );
  textStyleElem.appendChild( ddElem );

  return textStyleElem;
}

QMimeData *QgsTextFormat::toMimeData() const
{
  //set both the mime color data, and the text (format settings).

  QMimeData *mimeData = new QMimeData;
  mimeData->setColorData( QVariant( color() ) );

  QgsReadWriteContext rwContext;
  QDomDocument textDoc;
  QDomElement textElem = writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  mimeData->setText( textDoc.toString() );

  return mimeData;
}

QgsTextFormat QgsTextFormat::fromQFont( const QFont &font )
{
  QgsTextFormat format;
  format.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    format.setSize( font.pointSizeF() );
    format.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    format.setSize( font.pixelSize() );
    format.setSizeUnit( QgsUnitTypes::RenderPixels );
  }

  return format;
}

QFont QgsTextFormat::toQFont() const
{
  QFont f = font();
  switch ( sizeUnit() )
  {
    case QgsUnitTypes::RenderPoints:
      f.setPointSizeF( size() );
      break;

    case QgsUnitTypes::RenderMillimeters:
      f.setPointSizeF( size() * 2.83464567 );
      break;

    case QgsUnitTypes::RenderInches:
      f.setPointSizeF( size() * 72 );
      break;

    case QgsUnitTypes::RenderPixels:
      f.setPixelSize( static_cast< int >( std::round( size() ) ) );
      break;

    case QgsUnitTypes::RenderMapUnits:
    case QgsUnitTypes::RenderMetersInMapUnits:
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      // no meaning here
      break;
  }
  return f;
}

QgsTextFormat QgsTextFormat::fromMimeData( const QMimeData *data, bool *ok )
{
  if ( ok )
    *ok = false;
  QgsTextFormat format;
  if ( !data )
    return format;

  QString text = data->text();
  if ( !text.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;
    QgsReadWriteContext rwContext;

    if ( doc.setContent( text ) )
    {
      elem = doc.documentElement();

      format.readXml( elem, rwContext );
      if ( ok )
        *ok = true;
      return format;
    }
  }
  return format;
}

bool QgsTextFormat::containsAdvancedEffects() const
{
  if ( d->blendMode != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mBufferSettings.enabled() && mBufferSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mBackgroundSettings.enabled() && mBackgroundSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mShadowSettings.enabled() && mShadowSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  return false;
}

QgsPropertyCollection &QgsTextFormat::dataDefinedProperties()
{
  d->isValid = true;
  return d->mDataDefinedProperties;
}

const QgsPropertyCollection &QgsTextFormat::dataDefinedProperties() const
{
  return d->mDataDefinedProperties;
}

QSet<QString> QgsTextFormat::referencedFields( const QgsRenderContext &context ) const
{
  QSet< QString > fields = d->mDataDefinedProperties.referencedFields( context.expressionContext(), true );
  fields.unite( mBufferSettings.referencedFields( context ) );
  fields.unite( mBackgroundSettings.referencedFields( context ) );
  fields.unite( mShadowSettings.referencedFields( context ) );
  fields.unite( mMaskSettings.referencedFields( context ) );
  return fields;
}

void QgsTextFormat::setDataDefinedProperties( const QgsPropertyCollection &collection )
{
  d->isValid = true;
  d->mDataDefinedProperties = collection;
}

void QgsTextFormat::updateDataDefinedProperties( QgsRenderContext &context )
{
  d->isValid = true;
  if ( !d->mDataDefinedProperties.hasActiveProperties() )
    return;

  QString ddFontFamily;
  context.expressionContext().setOriginalValueVariable( d->textFont.family() );
  QVariant exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::Family, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    QString family = exprVal.toString().trimmed();
    if ( d->textFont.family() != family )
    {
      // testing for ddFontFamily in QFontDatabase.families() may be slow to do for every feature
      // (i.e. don't use QgsFontUtils::fontFamilyMatchOnSystem( family ) here)
      if ( QgsFontUtils::fontFamilyOnSystem( family ) )
      {
        ddFontFamily = family;
      }
    }
  }

  // data defined named font style?
  QString ddFontStyle;
  context.expressionContext().setOriginalValueVariable( d->textNamedStyle );
  exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontStyle, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    QString fontstyle = exprVal.toString().trimmed();
    ddFontStyle = fontstyle;
  }

  bool ddBold = false;
  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Bold ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.bold() );
    ddBold = d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Bold, context.expressionContext(), false ) ;
  }

  bool ddItalic = false;
  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Italic ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.italic() );
    ddItalic = d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Italic, context.expressionContext(), false );
  }

  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  //       (currently defaults to what has been read in from layer settings)
  QFont newFont;
  QFontDatabase fontDb;
  QFont appFont = QApplication::font();
  bool newFontBuilt = false;
  if ( ddBold || ddItalic )
  {
    // new font needs built, since existing style needs removed
    newFont = QFont( !ddFontFamily.isEmpty() ? ddFontFamily : d->textFont.family() );
    newFontBuilt = true;
    newFont.setBold( ddBold );
    newFont.setItalic( ddItalic );
  }
  else if ( !ddFontStyle.isEmpty()
            && ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
  {
    if ( !ddFontFamily.isEmpty() )
    {
      // both family and style are different, build font from database
      QFont styledfont = fontDb.font( ddFontFamily, ddFontStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }

    // update the font face style
    QgsFontUtils::updateFontViaStyle( newFontBuilt ? newFont : d->textFont, ddFontStyle );
  }
  else if ( !ddFontFamily.isEmpty() )
  {
    if ( ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
    {
      // just family is different, build font from database
      QFont styledfont = fontDb.font( ddFontFamily, d->textNamedStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }
    else
    {
      newFont = QFont( ddFontFamily );
      newFontBuilt = true;
    }
  }

  if ( newFontBuilt )
  {
    // copy over existing font settings
    newFont.setUnderline( d->textFont.underline() );
    newFont.setStrikeOut( d->textFont.strikeOut() );
    newFont.setWordSpacing( d->textFont.wordSpacing() );
    newFont.setLetterSpacing( QFont::AbsoluteSpacing, d->textFont.letterSpacing() );
    d->textFont = newFont;
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Underline ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.underline() );
    d->textFont.setUnderline( d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Underline, context.expressionContext(), d->textFont.underline() ) );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Strikeout ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.strikeOut() );
    d->textFont.setStrikeOut( d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Strikeout, context.expressionContext(), d->textFont.strikeOut() ) );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Color ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->textColor ) );
    d->textColor = d->mDataDefinedProperties.valueAsColor( QgsPalLayerSettings::Color, context.expressionContext(), d->textColor );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Size ) )
  {
    context.expressionContext().setOriginalValueVariable( size() );
    d->fontSize = d->mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Size, context.expressionContext(), d->fontSize );
  }

  exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontSizeUnit, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->fontSizeUnits = res;
    }
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !val.isNull() )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

#ifdef HAS_KDE_QT5_FONT_STRETCH_FIX
  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontStretchFactor ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.stretch() );
    const QVariant val = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontStretchFactor, context.expressionContext(), d->textFont.stretch() );
    if ( !val.isNull() )
    {
      d->textFont.setStretch( val.toInt() );
    }
  }
#endif

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::TextOrientation ) )
  {
    const QString encoded = QgsTextRendererUtils::encodeTextOrientation( d->orientation );
    context.expressionContext().setOriginalValueVariable( encoded );
    d->orientation = QgsTextRendererUtils::decodeTextOrientation( d->mDataDefinedProperties.value( QgsPalLayerSettings::TextOrientation, context.expressionContext(), encoded ).toString() );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontLetterSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.letterSpacing() );
    const QVariant val = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontLetterSpacing, context.expressionContext(), d->textFont.letterSpacing() );
    if ( !val.isNull() )
    {
      d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, val.toDouble() );
    }
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontWordSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.wordSpacing() );
    const QVariant val = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontWordSpacing, context.expressionContext(), d->textFont.wordSpacing() );
    if ( !val.isNull() )
    {
      d->textFont.setWordSpacing( val.toDouble() );
    }
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontBlendMode ) )
  {
    exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontBlendMode, context.expressionContext() );
    QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  mShadowSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mBackgroundSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mBufferSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mMaskSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
}

QPixmap QgsTextFormat::textFormatPreviewPixmap( const QgsTextFormat &format, QSize size, const QString &previewText, int padding )
{
  QgsTextFormat tempFormat = format;
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );

  painter.setRenderHint( QPainter::Antialiasing );

  QRect rect( 0, 0, size.width(), size.height() );

  // shameless eye candy - use a subtle gradient when drawing background
  painter.setPen( Qt::NoPen );
  QColor background1 = tempFormat.previewBackgroundColor();
  if ( ( background1.lightnessF() < 0.7 ) )
  {
    background1 = background1.darker( 125 );
  }
  else
  {
    background1 = background1.lighter( 125 );
  }
  QColor background2 = tempFormat.previewBackgroundColor();
  QLinearGradient linearGrad( QPointF( 0, 0 ), QPointF( 0, rect.height() ) );
  linearGrad.setColorAt( 0, background1 );
  linearGrad.setColorAt( 1, background2 );
  painter.setBrush( QBrush( linearGrad ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }
  painter.setBrush( Qt::NoBrush );
  painter.setPen( Qt::NoPen );
  padding += 1; // move text away from background border

  QgsRenderContext context;
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
  context.setMapToPixel( newCoordXForm );

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  const double logicalDpiX = QgsApplication::desktop()->logicalDpiX();
#else
  QWidget *activeWindow = QApplication::activeWindow();
  const double logicalDpiX = activeWindow && activeWindow->screen() ? activeWindow->screen()->logicalDotsPerInchX() : 96.0;
#endif
  context.setScaleFactor( logicalDpiX / 25.4 );

  context.setUseAdvancedEffects( true );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  context.setPainter( &painter );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );

  // slightly inset text to account for buffer/background
  const double fontSize = context.convertToPainterUnits( tempFormat.size(), tempFormat.sizeUnit(), tempFormat.sizeMapUnitScale() );
  double xtrans = 0;
  if ( tempFormat.buffer().enabled() )
    xtrans = tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
             ? fontSize * tempFormat.buffer().size() / 100
             : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() );
  if ( tempFormat.background().enabled() && tempFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat.background().size().width(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  double ytrans = 0.0;
  if ( tempFormat.buffer().enabled() )
    ytrans = std::max( ytrans, tempFormat.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                       ? fontSize * tempFormat.buffer().size() / 100
                       : context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() ) );
  if ( tempFormat.background().enabled() )
    ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.background().size().height(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  const QStringList text = QStringList() << ( previewText.isEmpty() ? QObject::tr( "Aa" ) : previewText );
  const double textHeight = QgsTextRenderer::textHeight( context, tempFormat, text, QgsTextRenderer::Rect );
  QRectF textRect = rect;
  textRect.setLeft( xtrans + padding );
  textRect.setWidth( rect.width() - xtrans - 2 * padding );

  if ( textRect.width() > 2000 )
    textRect.setWidth( 2000 - 2 * padding );

  const double bottom = textRect.height() / 2 + textHeight / 2;
  textRect.setTop( bottom - textHeight );
  textRect.setBottom( bottom );

  QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignCenter, text, context, tempFormat );

  // draw border on top of text
  painter.setBrush( Qt::NoBrush );
  painter.setPen( QPen( tempFormat.previewBackgroundColor().darker( 150 ), 0 ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }
  painter.end();
  return pixmap;
}
