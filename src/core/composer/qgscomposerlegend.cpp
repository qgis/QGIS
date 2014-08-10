/***************************************************************************
                         qgscomposerlegend.cpp  -  description
                         ---------------------
    begin                : June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <limits>

#include "qgscomposerlegendstyle.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegenditem.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgscomposermodel.h"
#include "qgslegendrenderer.h"
#include "qgslogger.h"
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

QgsComposerLegend::QgsComposerLegend( QgsComposition* composition )
    : QgsComposerItem( composition )
    , mComposerMap( 0 )
{

  adjustBoxSize();

  connect( &mLegendModel, SIGNAL( layersChanged() ), this, SLOT( synchronizeWithModel() ) );
}

QgsComposerLegend::QgsComposerLegend(): QgsComposerItem( 0 ), mComposerMap( 0 )
{

}

QgsComposerLegend::~QgsComposerLegend()
{
}

void QgsComposerLegend::paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );


  if ( mComposition )
    mSettings.setUseAdvancedEffects( mComposition->useAdvancedEffects() );
  if ( mComposerMap )
    mSettings.setMmPerMapUnit( mComposerMap->mapUnitsToMM() );

  if ( !painter )
    return;

  drawBackground( painter );
  painter->save();
  //antialiasing on
  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setPen( QPen( QColor( 0, 0, 0 ) ) );

  QgsLegendRenderer legendRenderer( &mLegendModel, mSettings );
  legendRenderer.setLegendSize( rect().size() );
  legendRenderer.drawLegend( painter );

  painter->restore();

  //draw frame and selection boxes if necessary
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

QSizeF QgsComposerLegend::paintAndDetermineSize( QPainter* painter )
{
  QgsLegendRenderer legendRenderer( &mLegendModel, mSettings );
  QSizeF size = legendRenderer.minimumSize();
  if ( !painter )
    legendRenderer.drawLegend( painter );
  return size;
}


void QgsComposerLegend::adjustBoxSize()
{
  QgsLegendRenderer legendRenderer( &mLegendModel, mSettings );
  QSizeF size = legendRenderer.minimumSize();
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( size.width() ).arg( size.height() ) );
  if ( size.isValid() )
  {
    setSceneRect( QRectF( pos().x(), pos().y(), size.width(), size.height() ) );
  }
}

void QgsComposerLegend::setTitle( const QString& t )
{
  mSettings.setTitle( t );

  if ( mComposition && id().isEmpty() )
  {
    //notify the model that the display name has changed
    mComposition->itemsModel()->updateItemDisplayName( this );
  }
}
QString QgsComposerLegend::title() const { return mSettings.title(); }

Qt::AlignmentFlag QgsComposerLegend::titleAlignment() const { return mSettings.titleAlignment(); }
void QgsComposerLegend::setTitleAlignment( Qt::AlignmentFlag alignment ) { mSettings.setTitleAlignment( alignment ); }

QgsComposerLegendStyle& QgsComposerLegend::rstyle( QgsComposerLegendStyle::Style s ) { return mSettings.rstyle( s ); }
QgsComposerLegendStyle QgsComposerLegend::style( QgsComposerLegendStyle::Style s ) const { return mSettings.style( s ); }
void QgsComposerLegend::setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style ) { mSettings.setStyle( s, style ); }

QFont QgsComposerLegend::styleFont( QgsComposerLegendStyle::Style s ) const { return mSettings.style( s ).font(); }
void QgsComposerLegend::setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f ) { rstyle( s ).setFont( f ); }

void QgsComposerLegend::setStyleMargin( QgsComposerLegendStyle::Style s, double margin ) { rstyle( s ).setMargin( margin ); }
void QgsComposerLegend::setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin ) { rstyle( s ).setMargin( side, margin ); }

double QgsComposerLegend::boxSpace() const { return mSettings.boxSpace(); }
void QgsComposerLegend::setBoxSpace( double s ) { mSettings.setBoxSpace( s ); }

double QgsComposerLegend::columnSpace() const { return mSettings.columnSpace(); }
void QgsComposerLegend::setColumnSpace( double s ) { mSettings.setColumnSpace( s ); }

QColor QgsComposerLegend::fontColor() const { return mSettings.fontColor(); }
void QgsComposerLegend::setFontColor( const QColor& c ) { mSettings.setFontColor( c ); }

double QgsComposerLegend::symbolWidth() const { return mSettings.symbolSize().width(); }
void QgsComposerLegend::setSymbolWidth( double w ) { mSettings.setSymbolSize( QSizeF( w, mSettings.symbolSize().height() ) ); }

double QgsComposerLegend::symbolHeight() const { return mSettings.symbolSize().height(); }
void QgsComposerLegend::setSymbolHeight( double h ) { mSettings.setSymbolSize( QSizeF( mSettings.symbolSize().width(), h ) ); }

double QgsComposerLegend::wmsLegendWidth() const { return mSettings.wmsLegendSize().width(); }
void QgsComposerLegend::setWmsLegendWidth( double w ) { mSettings.setWmsLegendSize( QSizeF( w, mSettings.wmsLegendSize().height() ) ); }

double QgsComposerLegend::wmsLegendHeight() const {return mSettings.wmsLegendSize().height(); }
void QgsComposerLegend::setWmsLegendHeight( double h ) { mSettings.setWmsLegendSize( QSizeF( mSettings.wmsLegendSize().width(), h ) ); }

void QgsComposerLegend::setWrapChar( const QString& t ) { mSettings.setWrapChar( t ); }
QString QgsComposerLegend::wrapChar() const {return mSettings.wrapChar(); }

int QgsComposerLegend::columnCount() const { return mSettings.columnCount(); }
void QgsComposerLegend::setColumnCount( int c ) { mSettings.setColumnCount( c ); }

int QgsComposerLegend::splitLayer() const { return mSettings.splitLayer(); }
void QgsComposerLegend::setSplitLayer( bool s ) { mSettings.setSplitLayer( s ); }

int QgsComposerLegend::equalColumnWidth() const { return mSettings.equalColumnWidth(); }
void QgsComposerLegend::setEqualColumnWidth( bool s ) { mSettings.setEqualColumnWidth( s ); }


void QgsComposerLegend::synchronizeWithModel()
{
  QgsDebugMsg( "Entered" );
  adjustBoxSize();
  update();
}

void QgsComposerLegend::updateLegend()
{
  // take layer list from map renderer (to have legend order)
  mLegendModel.setLayerSet( mComposition ? mComposition->mapSettings().layers() : QStringList() );
  adjustBoxSize();
  update();
}

bool QgsComposerLegend::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  if ( elem.isNull() )
  {
    return false;
  }

  QDomElement composerLegendElem = doc.createElement( "ComposerLegend" );
  elem.appendChild( composerLegendElem );

  //write general properties
  composerLegendElem.setAttribute( "title", mSettings.title() );
  composerLegendElem.setAttribute( "titleAlignment", QString::number(( int ) mSettings.titleAlignment() ) );
  composerLegendElem.setAttribute( "columnCount", QString::number( mSettings.columnCount() ) );
  composerLegendElem.setAttribute( "splitLayer", QString::number( mSettings.splitLayer() ) );
  composerLegendElem.setAttribute( "equalColumnWidth", QString::number( mSettings.equalColumnWidth() ) );

  composerLegendElem.setAttribute( "boxSpace", QString::number( mSettings.boxSpace() ) );
  composerLegendElem.setAttribute( "columnSpace", QString::number( mSettings.columnSpace() ) );

  composerLegendElem.setAttribute( "symbolWidth", QString::number( mSettings.symbolSize().width() ) );
  composerLegendElem.setAttribute( "symbolHeight", QString::number( mSettings.symbolSize().height() ) );
  composerLegendElem.setAttribute( "wmsLegendWidth", QString::number( mSettings.wmsLegendSize().width() ) );
  composerLegendElem.setAttribute( "wmsLegendHeight", QString::number( mSettings.wmsLegendSize().height() ) );
  composerLegendElem.setAttribute( "wrapChar", mSettings.wrapChar() );
  composerLegendElem.setAttribute( "fontColor", mSettings.fontColor().name() );

  if ( mComposerMap )
  {
    composerLegendElem.setAttribute( "map", mComposerMap->id() );
  }

  QDomElement composerLegendStyles = doc.createElement( "styles" );
  composerLegendElem.appendChild( composerLegendStyles );

  style( QgsComposerLegendStyle::Title ).writeXML( "title", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Group ).writeXML( "group", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Subgroup ).writeXML( "subgroup", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::Symbol ).writeXML( "symbol", composerLegendStyles, doc );
  style( QgsComposerLegendStyle::SymbolLabel ).writeXML( "symbolLabel", composerLegendStyles, doc );

  //write model properties
  mLegendModel.writeXML( composerLegendElem, doc );

  return _writeXML( composerLegendElem, doc );
}

bool QgsComposerLegend::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  if ( itemElem.isNull() )
  {
    return false;
  }

  //read general properties
  mSettings.setTitle( itemElem.attribute( "title" ) );
  if ( !itemElem.attribute( "titleAlignment" ).isEmpty() )
  {
    mSettings.setTitleAlignment(( Qt::AlignmentFlag )itemElem.attribute( "titleAlignment" ).toInt() );
  }
  int colCount = itemElem.attribute( "columnCount", "1" ).toInt();
  if ( colCount < 1 ) colCount = 1;
  mSettings.setColumnCount( colCount );
  mSettings.setSplitLayer( itemElem.attribute( "splitLayer", "0" ).toInt() == 1 );
  mSettings.setEqualColumnWidth( itemElem.attribute( "equalColumnWidth", "0" ).toInt() == 1 );

  QDomNodeList stylesNodeList = itemElem.elementsByTagName( "styles" );
  if ( stylesNodeList.size() > 0 )
  {
    QDomNode stylesNode = stylesNodeList.at( 0 );
    for ( int i = 0; i < stylesNode.childNodes().size(); i++ )
    {
      QDomElement styleElem = stylesNode.childNodes().at( i ).toElement();
      QgsComposerLegendStyle style;
      style.readXML( styleElem, doc );
      QString name = styleElem.attribute( "name" );
      QgsComposerLegendStyle::Style s;
      if ( name == "title" ) s = QgsComposerLegendStyle::Title;
      else if ( name == "group" ) s = QgsComposerLegendStyle::Group;
      else if ( name == "subgroup" ) s = QgsComposerLegendStyle::Subgroup;
      else if ( name == "symbol" ) s = QgsComposerLegendStyle::Symbol;
      else if ( name == "symbolLabel" ) s = QgsComposerLegendStyle::SymbolLabel;
      else continue;
      setStyle( s, style );
    }
  }

  //font color
  QColor fontClr;
  fontClr.setNamedColor( itemElem.attribute( "fontColor", "#000000" ) );
  mSettings.setFontColor( fontClr );

  //spaces
  mSettings.setBoxSpace( itemElem.attribute( "boxSpace", "2.0" ).toDouble() );
  mSettings.setColumnSpace( itemElem.attribute( "columnSpace", "2.0" ).toDouble() );

  mSettings.setSymbolSize( QSizeF( itemElem.attribute( "symbolWidth", "7.0" ).toDouble(), itemElem.attribute( "symbolHeight", "14.0" ).toDouble() ) );
  mSettings.setWmsLegendSize( QSizeF( itemElem.attribute( "wmsLegendWidth", "50" ).toDouble(), itemElem.attribute( "wmsLegendHeight", "25" ).toDouble() ) );

  mSettings.setWrapChar( itemElem.attribute( "wrapChar" ) );

  //composer map
  if ( !itemElem.attribute( "map" ).isEmpty() )
  {
    mComposerMap = mComposition->getComposerMapById( itemElem.attribute( "map" ).toInt() );
  }

  //read model properties
  QDomNodeList modelNodeList = itemElem.elementsByTagName( "Model" );
  if ( modelNodeList.size() > 0 )
  {
    QDomElement modelElem = modelNodeList.at( 0 ).toElement();
    mLegendModel.readXML( modelElem, doc );
  }

  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  // < 2.0 projects backward compatibility >>>>>
  //title font
  QString titleFontString = itemElem.attribute( "titleFont" );
  if ( !titleFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Title ).rfont().fromString( titleFontString );
  }
  //group font
  QString groupFontString = itemElem.attribute( "groupFont" );
  if ( !groupFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Group ).rfont().fromString( groupFontString );
  }

  //layer font
  QString layerFontString = itemElem.attribute( "layerFont" );
  if ( !layerFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Subgroup ).rfont().fromString( layerFontString );
  }
  //item font
  QString itemFontString = itemElem.attribute( "itemFont" );
  if ( !itemFontString.isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::SymbolLabel ).rfont().fromString( itemFontString );
  }

  if ( !itemElem.attribute( "groupSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Group ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "groupSpace", "3.0" ).toDouble() );
  }
  if ( !itemElem.attribute( "layerSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Subgroup ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "layerSpace", "3.0" ).toDouble() );
  }
  if ( !itemElem.attribute( "symbolSpace" ).isEmpty() )
  {
    rstyle( QgsComposerLegendStyle::Symbol ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "symbolSpace", "2.0" ).toDouble() );
    rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Top, itemElem.attribute( "symbolSpace", "2.0" ).toDouble() );
  }
  // <<<<<<< < 2.0 projects backward compatibility

  emit itemChanged();
  return true;
}

QString QgsComposerLegend::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  //if no id, default to portion of title text
  QString text = mSettings.title();
  if ( text.isEmpty() )
  {
    return tr( "<legend>" );
  }
  if ( text.length() > 25 )
  {
    return QString( tr( "%1..." ) ).arg( text.left( 25 ) );
  }
  else
  {
    return text;
  }
}

void QgsComposerLegend::setComposerMap( const QgsComposerMap* map )
{
  mComposerMap = map;
  if ( map )
  {
    QObject::connect( map, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  }
}

void QgsComposerLegend::invalidateCurrentMap()
{
  if ( mComposerMap )
  {
    disconnect( mComposerMap, SIGNAL( destroyed( QObject* ) ), this, SLOT( invalidateCurrentMap() ) );
  }
  mComposerMap = 0;
}
