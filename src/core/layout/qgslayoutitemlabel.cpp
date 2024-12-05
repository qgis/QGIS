/***************************************************************************
                             qgslayoutitemlabel.cpp
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemlabel.h"
#include "moc_qgslayoutitemlabel.cpp"
#include "qgslayoutitemregistry.h"
#include "qgslayoututils.h"
#include "qgslayoutmodel.h"
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsdistancearea.h"
#include "qgsfontutils.h"
#include "qgstextformat.h"
#include "qgstextrenderer.h"
#include "qgsexpressioncontext.h"
#include "qgslayoutitemmap.h"
#include "qgssettings.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"

#include <QCoreApplication>
#include <QDate>
#include <QDomElement>
#include <QPainter>
#include <QTextDocument>

QgsLayoutItemLabel::QgsLayoutItemLabel( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  mDistanceArea.reset( new QgsDistanceArea() );
  mHtmlUnitsToLayoutUnits = htmlUnitsToLayoutUnits();

  //get default layout font from settings
  const QgsSettings settings;
  const QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    QFont f = mFormat.font();
    QgsFontUtils::setFontFamily( f, defaultFontString );
    mFormat.setFont( f );
  }

  //default to a 10 point font size
  mFormat.setSize( 10 );
  mFormat.setSizeUnit( Qgis::RenderUnit::Points );

  connect( this, &QgsLayoutItem::sizePositionChanged, this, [this]
  {
    updateBoundingRect();
  } );

  //default to no background
  setBackgroundEnabled( false );

  //a label added while atlas preview is enabled needs to have the expression context set,
  //otherwise fields in the label aren't correctly evaluated until atlas preview feature changes (#9457)
  refreshExpressionContext();
}

QgsLayoutItemLabel *QgsLayoutItemLabel::create( QgsLayout *layout )
{
  return new QgsLayoutItemLabel( layout );
}

int QgsLayoutItemLabel::type() const
{
  return QgsLayoutItemRegistry::LayoutLabel;
}

QIcon QgsLayoutItemLabel::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemLabel.svg" ) );
}

void QgsLayoutItemLabel::draw( QgsLayoutItemRenderContext &context )
{
  QPainter *painter = context.renderContext().painter();
  const QgsScopedQPainterState painterState( painter );

  const double penWidth = frameEnabled() ? ( pen().widthF() / 2.0 ) : 0;
  const double xPenAdjust = mMarginX < 0 ? -penWidth : penWidth;
  const double yPenAdjust = mMarginY < 0 ? -penWidth : penWidth;

  QRectF painterRect;
  if ( mMode == QgsLayoutItemLabel::ModeFont )
  {
    const double rectScale = context.renderContext().scaleFactor();
    painterRect = QRectF( ( xPenAdjust + mMarginX ) * rectScale,
                          ( yPenAdjust + mMarginY ) * rectScale,
                          ( rect().width() - 2 * xPenAdjust - 2 * mMarginX ) * rectScale,
                          ( rect().height() - 2 * yPenAdjust - 2 * mMarginY ) * rectScale );
  }
  else
  {
    // The 3.77 adjustment value was found through trial and error, the author has however no clue as to where it comes from
    const double adjustmentFactor = 3.77;
    const double rectScale = context.renderContext().scaleFactor() * adjustmentFactor;
    // The left/right margin is handled by the stylesheet while the top/bottom margin is ignored by QTextDocument
    painterRect = QRectF( 0, 0,
                          ( rect().width() ) * rectScale,
                          ( rect().height() - yPenAdjust - mMarginY ) * rectScale );
    painter->translate( 0, ( yPenAdjust + mMarginY ) * context.renderContext().scaleFactor() );
    painter->scale( context.renderContext().scaleFactor() / adjustmentFactor, context.renderContext().scaleFactor() / adjustmentFactor );
  }

  switch ( mMode )
  {
    case ModeHtml:
    {
      QTextDocument document;
      document.setDocumentMargin( 0 );
      document.setPageSize( QSizeF( painterRect.width() / context.renderContext().scaleFactor(), painterRect.height() / context.renderContext().scaleFactor() ) );
      document.setDefaultStyleSheet( createStylesheet() );

      document.setDefaultFont( createDefaultFont() );

      QTextOption textOption = document.defaultTextOption();
      textOption.setAlignment( mHAlignment );
      document.setDefaultTextOption( textOption );

      document.setHtml( QStringLiteral( "<body>%1</body>" ).arg( currentText() ) );
      document.drawContents( painter, painterRect );
      break;
    }

    case ModeFont:
    {
      context.renderContext().setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );
      QgsTextRenderer::drawText( painterRect, 0,
                                 QgsTextRenderer::convertQtHAlignment( mHAlignment ),
                                 currentText().split( '\n' ),
                                 context.renderContext(),
                                 mFormat,
                                 true,
                                 QgsTextRenderer::convertQtVAlignment( mVAlignment ),
                                 Qgis::TextRendererFlag::WrapLines );
      break;
    }
  }
}

void QgsLayoutItemLabel::contentChanged()
{
  switch ( mMode )
  {
    case ModeHtml:
    {
      invalidateCache();
      break;
    }
    case ModeFont:
      invalidateCache();
      break;
  }
}

void QgsLayoutItemLabel::setText( const QString &text )
{
  mText = text;
  emit changed();

  contentChanged();

  if ( mLayout && id().isEmpty() && mMode != ModeHtml )
  {
    //notify the model that the display name has changed
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
}

void QgsLayoutItemLabel::setMode( Mode mode )
{
  if ( mode == mMode )
  {
    return;
  }

  mMode = mode;
  contentChanged();

  if ( mLayout && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mLayout->itemsModel()->updateItemDisplayName( this );
  }
}

void QgsLayoutItemLabel::refreshExpressionContext()
{
  if ( !mLayout )
    return;

  QgsVectorLayer *layer = mLayout->reportContext().layer();
  //setup distance area conversion
  if ( layer )
  {
    mDistanceArea->setSourceCrs( layer->crs(), mLayout->project()->transformContext() );
  }
  else
  {
    //set to composition's reference map's crs
    QgsLayoutItemMap *referenceMap = mLayout->referenceMap();
    if ( referenceMap )
      mDistanceArea->setSourceCrs( referenceMap->crs(), mLayout->project()->transformContext() );
  }
  mDistanceArea->setEllipsoid( mLayout->project()->ellipsoid() );
  contentChanged();

  update();
}

void QgsLayoutItemLabel::updateBoundingRect()
{
  QRectF rectangle = rect();
  const double frameExtension = frameEnabled() ? pen().widthF() / 2.0 : 0.0;
  if ( frameExtension > 0 )
    rectangle.adjust( -frameExtension, -frameExtension, frameExtension, frameExtension );

  if ( mMarginX < 0 )
  {
    rectangle.adjust( mMarginX, 0, -mMarginX, 0 );
  }
  if ( mMarginY < 0 )
  {
    rectangle.adjust( 0, mMarginY, 0, -mMarginY );
  }

  if ( rectangle != mCurrentRectangle )
  {
    prepareGeometryChange();
    mCurrentRectangle = rectangle;
  }
  invalidateCache();
}

QString QgsLayoutItemLabel::currentText() const
{
  QString displayText = mText;
  replaceDateText( displayText );

  const QgsExpressionContext context = createExpressionContext();

  return QgsExpression::replaceExpressionText( displayText, &context, mDistanceArea.get() );
}

void QgsLayoutItemLabel::replaceDateText( QString &text ) const
{
  const QString constant = QStringLiteral( "$CURRENT_DATE" );
  const int currentDatePos = text.indexOf( constant );
  if ( currentDatePos != -1 )
  {
    //check if there is a bracket just after $CURRENT_DATE
    QString formatText;
    const int openingBracketPos = text.indexOf( '(', currentDatePos );
    const int closingBracketPos = text.indexOf( ')', openingBracketPos + 1 );
    if ( openingBracketPos != -1 &&
         closingBracketPos != -1 &&
         ( closingBracketPos - openingBracketPos ) > 1 &&
         openingBracketPos == currentDatePos + constant.size() )
    {
      formatText = text.mid( openingBracketPos + 1, closingBracketPos - openingBracketPos - 1 );
      text.replace( currentDatePos, closingBracketPos - currentDatePos + 1, QDate::currentDate().toString( formatText ) );
    }
    else //no bracket
    {
      text.replace( QLatin1String( "$CURRENT_DATE" ), QDate::currentDate().toString() );
    }
  }
}

void QgsLayoutItemLabel::setFont( const QFont &f )
{
  mFormat.setFont( f );
  if ( f.pointSizeF() > 0 )
    mFormat.setSize( f.pointSizeF() );
  invalidateCache();
}

QgsTextFormat QgsLayoutItemLabel::textFormat() const
{
  return mFormat;
}

void QgsLayoutItemLabel::setTextFormat( const QgsTextFormat &format )
{
  mFormat = format;
  invalidateCache();
}

void QgsLayoutItemLabel::setMargin( const double m )
{
  mMarginX = m;
  mMarginY = m;
  updateBoundingRect();
}

void QgsLayoutItemLabel::setMarginX( const double margin )
{
  mMarginX = margin;
  updateBoundingRect();
}

void QgsLayoutItemLabel::setMarginY( const double margin )
{
  mMarginY = margin;
  updateBoundingRect();
}

void QgsLayoutItemLabel::adjustSizeToText()
{
  const QSizeF newSize = sizeForText();

  //keep alignment point constant
  double xShift = 0;
  double yShift = 0;

  itemShiftAdjustSize( newSize.width(), newSize.height(), xShift, yShift );

  //update rect for data defined size and position
  attemptSetSceneRect( QRectF( pos().x() + xShift, pos().y() + yShift, newSize.width(), newSize.height() ) );
}

void QgsLayoutItemLabel::adjustSizeToText( ReferencePoint referencePoint )
{
  const QSizeF newSize = sizeForText();
  const double newWidth = newSize.width();
  const double newHeight = newSize.height();
  const double currentWidth = rect().width();
  const double currentHeight = rect().height();

  //keep reference point constant
  double xShift = 0;
  double yShift = 0;
  switch ( referencePoint )
  {
    case QgsLayoutItem::UpperLeft:
      xShift = 0;
      yShift = 0;
      break;
    case QgsLayoutItem::UpperMiddle:
      xShift = - ( newWidth - currentWidth ) / 2.0;
      yShift = 0;
      break;

    case QgsLayoutItem::UpperRight:
      xShift = - ( newWidth - currentWidth );
      yShift = 0;
      break;

    case QgsLayoutItem::MiddleLeft:
      xShift = 0;
      yShift = -( newHeight - currentHeight ) / 2.0;
      break;

    case QgsLayoutItem::Middle:
      xShift = - ( newWidth - currentWidth ) / 2.0;
      yShift = -( newHeight - currentHeight ) / 2.0;
      break;

    case QgsLayoutItem::MiddleRight:
      xShift = - ( newWidth - currentWidth );
      yShift = -( newHeight - currentHeight ) / 2.0;
      break;

    case QgsLayoutItem::LowerLeft:
      xShift = 0;
      yShift = - ( newHeight - currentHeight );
      break;

    case QgsLayoutItem::LowerMiddle:
      xShift = - ( newWidth - currentWidth ) / 2.0;
      yShift = - ( newHeight - currentHeight );
      break;

    case QgsLayoutItem::LowerRight:
      xShift = - ( newWidth - currentWidth );
      yShift = - ( newHeight - currentHeight );
      break;
  }

  //update rect for data defined size and position
  attemptSetSceneRect( QRectF( pos().x() + xShift, pos().y() + yShift, newSize.width(), newSize.height() ) );
}

QSizeF QgsLayoutItemLabel::sizeForText() const
{
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );
  context.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering );

  const QStringList lines = currentText().split( '\n' );
  const double textWidth = std::ceil( QgsTextRenderer::textWidth( context, mFormat, lines ) + 1 ) / context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );
  const double fontHeight = std::ceil( QgsTextRenderer::textHeight( context, mFormat, lines ) + 1 ) / context.convertToPainterUnits( 1, Qgis::RenderUnit::Millimeters );

  const double penWidth = frameEnabled() ? ( pen().widthF() / 2.0 ) : 0;

  const double width = textWidth + 2 * mMarginX + 2 * penWidth;
  const double height = fontHeight + 2 * mMarginY + 2 * penWidth;

  return mLayout->convertToLayoutUnits( QgsLayoutSize( width, height, Qgis::LayoutUnit::Millimeters ) );
}

QFont QgsLayoutItemLabel::font() const
{
  return mFormat.font();
}

bool QgsLayoutItemLabel::writePropertiesToElement( QDomElement &layoutLabelElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  layoutLabelElem.setAttribute( QStringLiteral( "htmlState" ), static_cast< int >( mMode ) );

  layoutLabelElem.setAttribute( QStringLiteral( "labelText" ), mText );
  layoutLabelElem.setAttribute( QStringLiteral( "marginX" ), QString::number( mMarginX ) );
  layoutLabelElem.setAttribute( QStringLiteral( "marginY" ), QString::number( mMarginY ) );
  layoutLabelElem.setAttribute( QStringLiteral( "halign" ), mHAlignment );
  layoutLabelElem.setAttribute( QStringLiteral( "valign" ), mVAlignment );

  QDomElement textElem = mFormat.writeXml( doc, rwContext );
  layoutLabelElem.appendChild( textElem );

  return true;
}

bool QgsLayoutItemLabel::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  //restore label specific properties

  //text
  mText = itemElem.attribute( QStringLiteral( "labelText" ) );

  //html state
  mMode = static_cast< Mode >( itemElem.attribute( QStringLiteral( "htmlState" ) ).toInt() );

  //margin
  bool marginXOk = false;
  bool marginYOk = false;
  mMarginX = itemElem.attribute( QStringLiteral( "marginX" ) ).toDouble( &marginXOk );
  mMarginY = itemElem.attribute( QStringLiteral( "marginY" ) ).toDouble( &marginYOk );
  if ( !marginXOk || !marginYOk )
  {
    //upgrade old projects where margins where stored in a single attribute
    const double margin = itemElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "1.0" ) ).toDouble();
    mMarginX = margin;
    mMarginY = margin;
  }

  //Horizontal alignment
  mHAlignment = static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "halign" ) ).toInt() );

  //Vertical alignment
  mVAlignment = static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "valign" ) ).toInt() );

  //font
  QDomNodeList textFormatNodeList = itemElem.elementsByTagName( QStringLiteral( "text-style" ) );
  if ( !textFormatNodeList.isEmpty() )
  {
    QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mFormat.readXml( textFormatElem, context );
  }
  else
  {
    QFont f;
    if ( !QgsFontUtils::setFromXmlChildNode( f, itemElem, QStringLiteral( "LabelFont" ) ) )
    {
      f.fromString( itemElem.attribute( QStringLiteral( "font" ), QString() ) );
    }
    mFormat.setFont( f );
    if ( f.pointSizeF() > 0 )
    {
      mFormat.setSize( f.pointSizeF() );
      mFormat.setSizeUnit( Qgis::RenderUnit::Points );
    }
    else if ( f.pixelSize() > 0 )
    {
      mFormat.setSize( f.pixelSize() );
      mFormat.setSizeUnit( Qgis::RenderUnit::Pixels );
    }

    //font color
    const QDomNodeList fontColorList = itemElem.elementsByTagName( QStringLiteral( "FontColor" ) );
    if ( !fontColorList.isEmpty() )
    {
      const QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
      const int red = fontColorElem.attribute( QStringLiteral( "red" ), QStringLiteral( "0" ) ).toInt();
      const int green = fontColorElem.attribute( QStringLiteral( "green" ), QStringLiteral( "0" ) ).toInt();
      const int blue = fontColorElem.attribute( QStringLiteral( "blue" ), QStringLiteral( "0" ) ).toInt();
      const int alpha = fontColorElem.attribute( QStringLiteral( "alpha" ), QStringLiteral( "255" ) ).toInt();
      mFormat.setColor( QColor( red, green, blue, alpha ) );
    }
    else if ( textFormatNodeList.isEmpty() )
    {
      mFormat.setColor( QColor( 0, 0, 0 ) );
    }
  }

  updateBoundingRect();

  return true;
}

QString QgsLayoutItemLabel::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  switch ( mMode )
  {
    case ModeHtml:
      return tr( "<HTML Label>" );

    case ModeFont:
    {

      //if no id, default to portion of label text
      const QString text = mText;
      if ( text.isEmpty() )
      {
        return tr( "<Label>" );
      }
      if ( text.length() > 25 )
      {
        return QString( tr( "%1…" ) ).arg( text.left( 25 ).simplified() );
      }
      else
      {
        return text.simplified();
      }
    }
  }
  return QString(); // no warnings
}

QRectF QgsLayoutItemLabel::boundingRect() const
{
  return mCurrentRectangle;
}

void QgsLayoutItemLabel::setFrameEnabled( bool drawFrame )
{
  QgsLayoutItem::setFrameEnabled( drawFrame );
  updateBoundingRect();
}

void QgsLayoutItemLabel::setFrameStrokeWidth( QgsLayoutMeasurement strokeWidth )
{
  QgsLayoutItem::setFrameStrokeWidth( strokeWidth );
  updateBoundingRect();
}

void QgsLayoutItemLabel::refresh()
{
  invalidateCache();
  QgsLayoutItem::refresh();
  refreshExpressionContext();
}

void QgsLayoutItemLabel::convertToStaticText()
{
  const QString evaluated = currentText();
  if ( evaluated == mText )
    return; // no changes

  setText( evaluated );
}

void QgsLayoutItemLabel::itemShiftAdjustSize( double newWidth, double newHeight, double &xShift, double &yShift ) const
{
  //keep alignment point constant
  const double currentWidth = rect().width();
  const double currentHeight = rect().height();
  xShift = 0;
  yShift = 0;

  const double r = rotation();
  if ( r >= 0 && r < 90 )
  {
    if ( mHAlignment == Qt::AlignHCenter )
    {
      xShift = - ( newWidth - currentWidth ) / 2.0;
    }
    else if ( mHAlignment == Qt::AlignRight )
    {
      xShift = - ( newWidth - currentWidth );
    }
    if ( mVAlignment == Qt::AlignVCenter )
    {
      yShift = -( newHeight - currentHeight ) / 2.0;
    }
    else if ( mVAlignment == Qt::AlignBottom )
    {
      yShift = - ( newHeight - currentHeight );
    }
  }
  if ( r >= 90 && r < 180 )
  {
    if ( mHAlignment == Qt::AlignHCenter )
    {
      yShift = -( newHeight  - currentHeight ) / 2.0;
    }
    else if ( mHAlignment == Qt::AlignRight )
    {
      yShift = -( newHeight  - currentHeight );
    }
    if ( mVAlignment == Qt::AlignTop )
    {
      xShift = -( newWidth - currentWidth );
    }
    else if ( mVAlignment == Qt::AlignVCenter )
    {
      xShift = -( newWidth - currentWidth / 2.0 );
    }
  }
  else if ( r >= 180 && r < 270 )
  {
    if ( mHAlignment == Qt::AlignHCenter )
    {
      xShift = -( newWidth - currentWidth ) / 2.0;
    }
    else if ( mHAlignment == Qt::AlignLeft )
    {
      xShift = -( newWidth - currentWidth );
    }
    if ( mVAlignment == Qt::AlignVCenter )
    {
      yShift = ( newHeight - currentHeight ) / 2.0;
    }
    else if ( mVAlignment == Qt::AlignTop )
    {
      yShift = ( newHeight - currentHeight );
    }
  }
  else if ( r >= 270 && r < 360 )
  {
    if ( mHAlignment == Qt::AlignHCenter )
    {
      yShift = -( newHeight  - currentHeight ) / 2.0;
    }
    else if ( mHAlignment == Qt::AlignLeft )
    {
      yShift = -( newHeight  - currentHeight );
    }
    if ( mVAlignment == Qt::AlignBottom )
    {
      xShift = -( newWidth - currentWidth );
    }
    else if ( mVAlignment == Qt::AlignVCenter )
    {
      xShift = -( newWidth - currentWidth / 2.0 );
    }
  }
}

QFont QgsLayoutItemLabel::createDefaultFont() const
{
  QFont f = mFormat.font();
  switch ( mFormat.sizeUnit() )
  {
    case Qgis::RenderUnit::Millimeters:
      f.setPointSizeF( mFormat.size() / 0.352778 );
      break;
    case Qgis::RenderUnit::Pixels:
      f.setPixelSize( mFormat.size() );
      break;
    case Qgis::RenderUnit::Points:
      f.setPointSizeF( mFormat.size() );
      break;
    case Qgis::RenderUnit::Inches:
      f.setPointSizeF( mFormat.size() * 72 );
      break;
    case Qgis::RenderUnit::Unknown:
    case Qgis::RenderUnit::Percentage:
    case Qgis::RenderUnit::MetersInMapUnits:
    case Qgis::RenderUnit::MapUnits:
      break;
  }
  return f;
}

double QgsLayoutItemLabel::htmlUnitsToLayoutUnits()
{
  if ( !mLayout )
  {
    return 1.0;
  }

  //TODO : fix this more precisely so that the label's default text size is the same with or without "display as html"
  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( mLayout->renderContext().dpi() / 72.0, Qgis::LayoutUnit::Millimeters ) ); //webkit seems to assume a standard dpi of 72
}

QString QgsLayoutItemLabel::createStylesheet() const
{
  QString stylesheet;

  stylesheet += QStringLiteral( "body { margin: %1 %2;" ).arg( std::max( mMarginY * mHtmlUnitsToLayoutUnits, 0.0 ) ).arg( std::max( mMarginX * mHtmlUnitsToLayoutUnits, 0.0 ) );
  stylesheet += mFormat.asCSS( 0.352778 * mHtmlUnitsToLayoutUnits );
  stylesheet += QStringLiteral( "text-align: %1; }" ).arg( mHAlignment == Qt::AlignLeft ? QStringLiteral( "left" ) : mHAlignment == Qt::AlignRight ? QStringLiteral( "right" ) : mHAlignment == Qt::AlignHCenter ? QStringLiteral( "center" ) : QStringLiteral( "justify" ) );

  return stylesheet;
}

QUrl QgsLayoutItemLabel::createStylesheetUrl() const
{
  QByteArray ba;
  ba.append( createStylesheet().toUtf8() );
  QUrl cssFileURL = QUrl( QString( "data:text/css;charset=utf-8;base64," + ba.toBase64() ) );

  return cssFileURL;
}
