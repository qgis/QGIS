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
#include "qgslayout.h"
#include "qgslayoututils.h"
#include "qgslayoutmodel.h"
#include "qgsexpression.h"
#include "qgsnetworkaccessmanager.h"
#include "qgscomposermodel.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsdistancearea.h"
#include "qgsfontutils.h"
#include "qgsexpressioncontext.h"
#include "qgsmapsettings.h"
#include "qgscomposermap.h"
#include "qgssettings.h"

#include "qgswebview.h"
#include "qgswebframe.h"
#include "qgswebpage.h"

#include <QCoreApplication>
#include <QDate>
#include <QDomElement>
#include <QPainter>
#include <QTimer>
#include <QEventLoop>

QgsLayoutItemLabel::QgsLayoutItemLabel( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  mDistanceArea.reset( new QgsDistanceArea() );
  mHtmlUnitsToLayoutUnits = htmlUnitsToLayoutUnits();

  //get default composer font from settings
  QgsSettings settings;
  QString defaultFontString = settings.value( QStringLiteral( "Composer/defaultFont" ) ).toString();
  if ( !defaultFontString.isEmpty() )
  {
    mFont.setFamily( defaultFontString );
  }

  //default to a 10 point font size
  mFont.setPointSizeF( 10 );

  //default to no background
  setBackgroundEnabled( false );

  //a label added while atlas preview is enabled needs to have the expression context set,
  //otherwise fields in the label aren't correctly evaluated until atlas preview feature changes (#9457)
  refreshExpressionContext();

  if ( mLayout )
  {
#if 0 //TODO
    //connect to atlas feature changes
    //to update the expression context
    connect( &mLayout->atlasComposition(), &QgsAtlasComposition::featureChanged, this, &QgsLayoutItemLabel::refreshExpressionContext );
#endif
  }

  mWebPage.reset( new QgsWebPage( this ) );
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

QgsLayoutItemLabel *QgsLayoutItemLabel::create( QgsLayout *layout )
{
  return new QgsLayoutItemLabel( layout );
}

int QgsLayoutItemLabel::type() const
{
  return QgsLayoutItemRegistry::LayoutLabel;
}

QString QgsLayoutItemLabel::stringType() const
{
  return QStringLiteral( "ItemLabel" );
}

void QgsLayoutItemLabel::draw( QgsRenderContext &context, const QStyleOptionGraphicsItem * )
{
  QPainter *painter = context.painter();
  painter->save();

  // painter is scaled to dots, so scale back to layout units
  painter->scale( context.scaleFactor(), context.scaleFactor() );

  double penWidth = hasFrame() ? ( pen().widthF() / 2.0 ) : 0;
  double xPenAdjust = mMarginX < 0 ? -penWidth : penWidth;
  double yPenAdjust = mMarginY < 0 ? -penWidth : penWidth;
  QRectF painterRect( xPenAdjust + mMarginX, yPenAdjust + mMarginY, rect().width() - 2 * xPenAdjust - 2 * mMarginX, rect().height() - 2 * yPenAdjust - 2 * mMarginY );

  switch ( mMode )
  {
    case ModeHtml:
    {
      if ( mFirstRender )
      {
        contentChanged();
        mFirstRender = false;
      }
      painter->scale( 1.0 / mHtmlUnitsToLayoutUnits / 10.0, 1.0 / mHtmlUnitsToLayoutUnits / 10.0 );
      mWebPage->setViewportSize( QSize( painterRect.width() * mHtmlUnitsToLayoutUnits * 10.0, painterRect.height() * mHtmlUnitsToLayoutUnits * 10.0 ) );
      mWebPage->settings()->setUserStyleSheetUrl( createStylesheetUrl() );
      mWebPage->mainFrame()->render( painter );
      break;
    }

    case ModeFont:
    {
      const QString textToDraw = currentText();
      painter->setFont( mFont );
      QgsLayoutUtils::drawText( painter, painterRect, textToDraw, mFont, mFontColor, mHAlignment, mVAlignment, Qt::TextWordWrap );
      break;
    }
  }

  painter->restore();
}

void QgsLayoutItemLabel::contentChanged()
{
  switch ( mMode )
  {
    case ModeHtml:
    {
      const QString textToDraw = currentText();

      //mHtmlLoaded tracks whether the QWebPage has completed loading
      //its html contents, set it initially to false. The loadingHtmlFinished slot will
      //set this to true after html is loaded.
      mHtmlLoaded = false;

      const QUrl baseUrl = QUrl::fromLocalFile( mLayout->project()->fileInfo().absoluteFilePath() );
      mWebPage->mainFrame()->setHtml( textToDraw, baseUrl );

      //For very basic html labels with no external assets, the html load will already be
      //complete before we even get a chance to start the QEventLoop. Make sure we check
      //this before starting the loop
      if ( !mHtmlLoaded )
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
      break;
  }
}

void QgsLayoutItemLabel::loadingHtmlFinished( bool result )
{
  Q_UNUSED( result );
  mHtmlLoaded = true;
}

double QgsLayoutItemLabel::htmlUnitsToLayoutUnits()
{
  if ( !mLayout )
  {
    return 1.0;
  }

  //TODO : fix this more precisely so that the label's default text size is the same with or without "display as html"
  return mLayout->convertToLayoutUnits( QgsLayoutMeasurement( mLayout->context().dpi() / 72.0, QgsUnitTypes::LayoutMillimeters ) ); //webkit seems to assume a standard dpi of 72
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

  QgsVectorLayer *layer = nullptr;
#if 0 //TODO
  if ( mComposition->atlasComposition().enabled() )
  {
    layer = mComposition->atlasComposition().coverageLayer();
  }
#endif

  //setup distance area conversion
  if ( layer )
  {
    mDistanceArea->setSourceCrs( layer->crs() );
  }
  else
  {
#if 0 //TODO
    //set to composition's reference map's crs
    QgsLayoutItemMap *referenceMap = mComposition->referenceMap();
    if ( referenceMap )
      mDistanceArea->setSourceCrs( referenceMap->crs() );
#endif
  }
  mDistanceArea->setEllipsoid( mLayout->project()->ellipsoid() );
  contentChanged();

  update();
}

QString QgsLayoutItemLabel::currentText() const
{
  QString displayText = mText;
  replaceDateText( displayText );

  QgsExpressionContext context = createExpressionContext();

  return QgsExpression::replaceExpressionText( displayText, &context, mDistanceArea.get() );
}

void QgsLayoutItemLabel::replaceDateText( QString &text ) const
{
  QString constant = QStringLiteral( "$CURRENT_DATE" );
  int currentDatePos = text.indexOf( constant );
  if ( currentDatePos != -1 )
  {
    //check if there is a bracket just after $CURRENT_DATE
    QString formatText;
    int openingBracketPos = text.indexOf( '(', currentDatePos );
    int closingBracketPos = text.indexOf( ')', openingBracketPos + 1 );
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
  mFont = f;
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
  QSizeF newSize = sizeForText();

  //keep alignment point constant
  double xShift = 0;
  double yShift = 0;

  itemShiftAdjustSize( newSize.width(), newSize.height(), xShift, yShift );

  //update rect for data defined size and position
  attemptSetSceneRect( QRectF( pos().x() + xShift, pos().y() + yShift, newSize.width(), newSize.height() ) );
}

QSizeF QgsLayoutItemLabel::sizeForText() const
{
  double textWidth = QgsLayoutUtils::textWidthMM( mFont, currentText() );
  double fontHeight = QgsLayoutUtils::fontHeightMM( mFont );

  double penWidth = hasFrame() ? ( pen().widthF() / 2.0 ) : 0;

  double width = textWidth + 2 * mMarginX + 2 * penWidth + 1;
  double height = fontHeight + 2 * mMarginY + 2 * penWidth;

  return mLayout->convertToLayoutUnits( QgsLayoutSize( width, height, QgsUnitTypes::LayoutMillimeters ) );
}

QFont QgsLayoutItemLabel::font() const
{
  return mFont;
}

bool QgsLayoutItemLabel::writePropertiesToElement( QDomElement &composerLabelElem, QDomDocument &doc, const QgsReadWriteContext & ) const
{
  composerLabelElem.setAttribute( QStringLiteral( "htmlState" ), static_cast< int >( mMode ) );

  composerLabelElem.setAttribute( QStringLiteral( "labelText" ), mText );
  composerLabelElem.setAttribute( QStringLiteral( "marginX" ), QString::number( mMarginX ) );
  composerLabelElem.setAttribute( QStringLiteral( "marginY" ), QString::number( mMarginY ) );
  composerLabelElem.setAttribute( QStringLiteral( "halign" ), mHAlignment );
  composerLabelElem.setAttribute( QStringLiteral( "valign" ), mVAlignment );

  //font
  QDomElement labelFontElem = QgsFontUtils::toXmlElement( mFont, doc, QStringLiteral( "LabelFont" ) );
  composerLabelElem.appendChild( labelFontElem );

  //font color
  QDomElement fontColorElem = doc.createElement( QStringLiteral( "FontColor" ) );
  fontColorElem.setAttribute( QStringLiteral( "red" ), mFontColor.red() );
  fontColorElem.setAttribute( QStringLiteral( "green" ), mFontColor.green() );
  fontColorElem.setAttribute( QStringLiteral( "blue" ), mFontColor.blue() );
  composerLabelElem.appendChild( fontColorElem );

  return true;
}

bool QgsLayoutItemLabel::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext & )
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
    double margin = itemElem.attribute( QStringLiteral( "margin" ), QStringLiteral( "1.0" ) ).toDouble();
    mMarginX = margin;
    mMarginY = margin;
  }

  //Horizontal alignment
  mHAlignment = static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "halign" ) ).toInt() );

  //Vertical alignment
  mVAlignment = static_cast< Qt::AlignmentFlag >( itemElem.attribute( QStringLiteral( "valign" ) ).toInt() );

  //font
  QgsFontUtils::setFromXmlChildNode( mFont, itemElem, QStringLiteral( "LabelFont" ) );

  //font color
  QDomNodeList fontColorList = itemElem.elementsByTagName( QStringLiteral( "FontColor" ) );
  if ( !fontColorList.isEmpty() )
  {
    QDomElement fontColorElem = fontColorList.at( 0 ).toElement();
    int red = fontColorElem.attribute( QStringLiteral( "red" ), QStringLiteral( "0" ) ).toInt();
    int green = fontColorElem.attribute( QStringLiteral( "green" ), QStringLiteral( "0" ) ).toInt();
    int blue = fontColorElem.attribute( QStringLiteral( "blue" ), QStringLiteral( "0" ) ).toInt();
    mFontColor = QColor( red, green, blue );
  }
  else
  {
    mFontColor = QColor( 0, 0, 0 );
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
      QString text = mText;
      if ( text.isEmpty() )
      {
        return tr( "<Label>" );
      }
      if ( text.length() > 25 )
      {
        return QString( tr( "%1..." ) ).arg( text.left( 25 ).simplified() );
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
  double penWidth = hasFrame() ? ( pen().widthF() / 2.0 ) : 0;
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

void QgsLayoutItemLabel::setFrameStrokeWidth( const QgsLayoutMeasurement &strokeWidth )
{
  QgsLayoutItem::setFrameStrokeWidth( strokeWidth );
  prepareGeometryChange();
}

void QgsLayoutItemLabel::refresh()
{
  contentChanged();
}

void QgsLayoutItemLabel::itemShiftAdjustSize( double newWidth, double newHeight, double &xShift, double &yShift ) const
{
  //keep alignment point constant
  double currentWidth = rect().width();
  double currentHeight = rect().height();
  xShift = 0;
  yShift = 0;

  double r = rotation();
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
  stylesheet += QgsFontUtils::asCSS( mFont, 0.352778 * mHtmlUnitsToLayoutUnits );
  stylesheet += QStringLiteral( "color: %1;" ).arg( mFontColor.name() );
  stylesheet += QStringLiteral( "text-align: %1; }" ).arg( mHAlignment == Qt::AlignLeft ? QStringLiteral( "left" ) : mHAlignment == Qt::AlignRight ? QStringLiteral( "right" ) : mHAlignment == Qt::AlignHCenter ? QStringLiteral( "center" ) : QStringLiteral( "justify" ) );

  QByteArray ba;
  ba.append( stylesheet.toUtf8() );
  QUrl cssFileURL = QUrl( "data:text/css;charset=utf-8;base64," + ba.toBase64() );

  return cssFileURL;
}
