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
#include "qgslayoutitemregistry.h"
#include "qgslayoututils.h"
#include "qgslayoutmodel.h"
#include "qgsexpression.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsvectorlayer.h"
#include "qgsdistancearea.h"
#include "qgsfontutils.h"
#include "qgstextformat.h"
#include "qgstextrenderer.h"
#include "qgsexpressioncontext.h"
#include "qgslayoutitemmap.h"
#include "qgssettings.h"
#include "qgslayout.h"

#include "qgswebpage.h"
#include "qgswebframe.h"

#include <QCoreApplication>
#include <QDate>
#include <QDomElement>
#include <QPainter>
#include <QTimer>
#include <QEventLoop>
#include <QThread>

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
    f.setFamily( defaultFontString );
    mFormat.setFont( f );
  }

  //default to a 10 point font size
  mFormat.setSize( 10 );
  mFormat.setSizeUnit( QgsUnitTypes::RenderPoints );

  //default to no background
  setBackgroundEnabled( false );

  //a label added while atlas preview is enabled needs to have the expression context set,
  //otherwise fields in the label aren't correctly evaluated until atlas preview feature changes (#9457)
  refreshExpressionContext();

  // only possible on the main thread!
  if ( QThread::currentThread() == QApplication::instance()->thread() )
  {
    mWebPage.reset( new QgsWebPage( this ) );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot load HTML based item label in background threads" ) );
  }
  if ( mWebPage )
  {
    mWebPage->setIdentifier( tr( "Layout label item" ) );
    mWebPage->setNetworkAccessManager( QgsNetworkAccessManager::instance() );

    //This makes the background transparent. Found on http://blog.qt.digia.com/blog/2009/06/30/transparent-qwebview-or-qwebpage/
    QPalette palette = mWebPage->palette();
    palette.setBrush( QPalette::Base, Qt::transparent );
    mWebPage->setPalette( palette );

    mWebPage->mainFrame()->setZoomFactor( 10.0 );
    mWebPage->mainFrame()->setScrollBarPolicy( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    mWebPage->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAlwaysOff );

    connect( mWebPage.get(), &QWebPage::loadFinished, this, &QgsLayoutItemLabel::loadingHtmlFinished );
  }
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

  double rectScale = 1.0;
  if ( mMode == QgsLayoutItemLabel::ModeFont )
  {
    rectScale = context.renderContext().scaleFactor();
  }
  else
  {
    // painter is scaled to dots, so scale back to layout units
    painter->scale( context.renderContext().scaleFactor(), context.renderContext().scaleFactor() );
  }

  const double penWidth = frameEnabled() ? ( pen().widthF() / 2.0 ) : 0;
  const double xPenAdjust = mMarginX < 0 ? -penWidth : penWidth;
  const double yPenAdjust = mMarginY < 0 ? -penWidth : penWidth;
  const QRectF painterRect( ( xPenAdjust + mMarginX ) * rectScale,
                            ( yPenAdjust + mMarginY ) * rectScale,
                            ( rect().width() - 2 * xPenAdjust - 2 * mMarginX ) * rectScale,
                            ( rect().height() - 2 * yPenAdjust - 2 * mMarginY ) * rectScale );

  switch ( mMode )
  {
    case ModeHtml:
    {
      if ( mFirstRender )
      {
        contentChanged();
        mFirstRender = false;
      }

      if ( mWebPage )
      {
        painter->scale( 1.0 / mHtmlUnitsToLayoutUnits / 10.0, 1.0 / mHtmlUnitsToLayoutUnits / 10.0 );
        mWebPage->setViewportSize( QSize( painterRect.width() * mHtmlUnitsToLayoutUnits * 10.0, painterRect.height() * mHtmlUnitsToLayoutUnits * 10.0 ) );
        mWebPage->settings()->setUserStyleSheetUrl( createStylesheetUrl() );
        mWebPage->mainFrame()->render( painter );
      }
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
      const QString textToDraw = currentText();
      if ( !mWebPage )
      {
        mHtmlLoaded = true;
        return;
      }

      //mHtmlLoaded tracks whether the QWebPage has completed loading
      //its html contents, set it initially to false. The loadingHtmlFinished slot will
      //set this to true after html is loaded.
      mHtmlLoaded = false;

      const QUrl baseUrl = QUrl::fromLocalFile( mLayout->project()->absoluteFilePath() );
      mWebPage->mainFrame()->setHtml( textToDraw, baseUrl );

      //For very basic html labels with no external assets, the html load will already be
      //complete before we even get a chance to start the QEventLoop. Make sure we check
      //this before starting the loop

      // important -- we CAN'T do this when it's a render inside the designer, otherwise the
      // event loop will mess with the paint event and cause it to be deleted, and BOOM!
      if ( !mHtmlLoaded && ( !mLayout || !mLayout->renderContext().isPreviewRender() ) )
      {
        //Setup event loop and timeout for rendering html
        QEventLoop loop;

        //Connect timeout and webpage loadFinished signals to loop
        connect( mWebPage.get(), &QWebPage::loadFinished, &loop, &QEventLoop::quit );

        // Start a 20 second timeout in case html loading will never complete
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot( true );
        connect( &timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit );
        timeoutTimer.start( 20000 );

        // Pause until html is loaded
        loop.exec( QEventLoop::ExcludeUserInputEvents );
      }
      break;
    }
    case ModeFont:
      invalidateCache();
      break;
  }
}

void QgsLayoutItemLabel::loadingHtmlFinished( bool result )
{
  Q_UNUSED( result )
  mHtmlLoaded = true;
  invalidateCache();
  update();
}

double QgsLayoutItemLabel::htmlUnitsToLayoutUnits()
{
  if ( !mLayout )
  {
    return 1.0;
  }

  //TODO : fix this more precisely so that the label's default text size is the same with or without "display as html"
  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( mLayout->renderContext().dpi() / 72.0, QgsUnitTypes::LayoutMillimeters ) ); //webkit seems to assume a standard dpi of 72
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
}

QgsTextFormat QgsLayoutItemLabel::textFormat() const
{
  return mFormat;
}

void QgsLayoutItemLabel::setTextFormat( const QgsTextFormat &format )
{
  mFormat = format;
}

void QgsLayoutItemLabel::setMargin( const double m )
{
  mMarginX = m;
  mMarginY = m;
  prepareGeometryChange();
}

void QgsLayoutItemLabel::setMarginX( const double margin )
{
  mMarginX = margin;
  prepareGeometryChange();
}

void QgsLayoutItemLabel::setMarginY( const double margin )
{
  mMarginY = margin;
  prepareGeometryChange();
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

QSizeF QgsLayoutItemLabel::sizeForText() const
{
  QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, nullptr );

  const QStringList lines = currentText().split( '\n' );
  const double textWidth = QgsTextRenderer::textWidth( context, mFormat, lines ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );
  const double fontHeight = QgsTextRenderer::textHeight( context, mFormat, lines ) / context.convertToPainterUnits( 1, QgsUnitTypes::RenderMillimeters );

  const double penWidth = frameEnabled() ? ( pen().widthF() / 2.0 ) : 0;

  const double width = textWidth + 2 * mMarginX + 2 * penWidth;
  const double height = fontHeight + 2 * mMarginY + 2 * penWidth;

  return mLayout->convertToLayoutUnits( QgsLayoutSize( width, height, QgsUnitTypes::LayoutMillimeters ) );
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
      mFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
    }
    else if ( f.pixelSize() > 0 )
    {
      mFormat.setSize( f.pixelSize() );
      mFormat.setSizeUnit( QgsUnitTypes::RenderPixels );
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
  QRectF rectangle = rect();
  const double penWidth = frameEnabled() ? ( pen().widthF() / 2.0 ) : 0;
  rectangle.adjust( -penWidth, -penWidth, penWidth, penWidth );

  if ( mMarginX < 0 )
  {
    rectangle.adjust( mMarginX, 0, -mMarginX, 0 );
  }
  if ( mMarginY < 0 )
  {
    rectangle.adjust( 0, mMarginY, 0, -mMarginY );
  }

  return rectangle;
}

void QgsLayoutItemLabel::setFrameEnabled( const bool drawFrame )
{
  QgsLayoutItem::setFrameEnabled( drawFrame );
  prepareGeometryChange();
}

void QgsLayoutItemLabel::setFrameStrokeWidth( const QgsLayoutMeasurement strokeWidth )
{
  QgsLayoutItem::setFrameStrokeWidth( strokeWidth );
  prepareGeometryChange();
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

QUrl QgsLayoutItemLabel::createStylesheetUrl() const
{
  QString stylesheet;
  stylesheet += QStringLiteral( "body { margin: %1 %2;" ).arg( std::max( mMarginY * mHtmlUnitsToLayoutUnits, 0.0 ) ).arg( std::max( mMarginX * mHtmlUnitsToLayoutUnits, 0.0 ) );
  QFont f = mFormat.font();
  switch ( mFormat.sizeUnit() )
  {
    case QgsUnitTypes::RenderMillimeters:
      f.setPointSizeF( mFormat.size() / 0.352778 );
      break;
    case QgsUnitTypes::RenderPixels:
      f.setPixelSize( mFormat.size() );
      break;
    case QgsUnitTypes::RenderPoints:
      f.setPointSizeF( mFormat.size() );
      break;
    case QgsUnitTypes::RenderInches:
      f.setPointSizeF( mFormat.size() * 72 );
      break;
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
    case QgsUnitTypes::RenderMetersInMapUnits:
    case QgsUnitTypes::RenderMapUnits:
      break;
  }

  stylesheet += QgsFontUtils::asCSS( f, 0.352778 * mHtmlUnitsToLayoutUnits );
  stylesheet += QStringLiteral( "color: rgba(%1,%2,%3,%4);" ).arg( mFormat.color().red() ).arg( mFormat.color().green() ).arg( mFormat.color().blue() ).arg( QString::number( mFormat.color().alphaF(), 'f', 4 ) );
  stylesheet += QStringLiteral( "text-align: %1; }" ).arg( mHAlignment == Qt::AlignLeft ? QStringLiteral( "left" ) : mHAlignment == Qt::AlignRight ? QStringLiteral( "right" ) : mHAlignment == Qt::AlignHCenter ? QStringLiteral( "center" ) : QStringLiteral( "justify" ) );

  QByteArray ba;
  ba.append( stylesheet.toUtf8() );
  QUrl cssFileURL = QUrl( QString( "data:text/css;charset=utf-8;base64," + ba.toBase64() ) );

  return cssFileURL;
}
