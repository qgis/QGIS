/***************************************************************************
                              qgsformannotationitem.h
                              ------------------------
  begin                : February 26, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsformannotationitem.h"
#include "qgsattributeeditor.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptool.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QSettings>
#include <QUiLoader>
#include <QWidget>

QgsFormAnnotationItem::QgsFormAnnotationItem( QgsMapCanvas* canvas, QgsVectorLayer* vlayer, bool hasFeature, int feature )
    : QgsAnnotationItem( canvas )
    , mWidgetContainer( 0 )
    , mDesignerWidget( 0 )
    , mVectorLayer( vlayer )
    , mHasAssociatedFeature( hasFeature )
    , mFeature( feature )
{
  mWidgetContainer = new QGraphicsProxyWidget( this );
  mWidgetContainer->setData( 0, "AnnotationItem" ); //mark embedded widget as belonging to an annotation item (composer knows it needs to be printed)
  if ( mVectorLayer && mMapCanvas ) //default to the layers edit form
  {
    mDesignerForm = mVectorLayer->annotationForm();
    QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
  }

  setFeatureForMapPosition();
}

QgsFormAnnotationItem::~QgsFormAnnotationItem()
{
  delete mDesignerWidget;
}

void QgsFormAnnotationItem::setDesignerForm( const QString& uiFile )
{
  mDesignerForm = uiFile;
  mWidgetContainer->setWidget( 0 );
  delete mDesignerWidget;
  mDesignerWidget = createDesignerWidget( uiFile );
  if ( mDesignerWidget )
  {
    mFrameBackgroundColor = mDesignerWidget->palette().color( QPalette::Window );
    mWidgetContainer->setWidget( mDesignerWidget );
    setFrameSize( preferredFrameSize() );
  }
}

QWidget* QgsFormAnnotationItem::createDesignerWidget( const QString& filePath )
{
  QFile file( filePath );
  if ( !file.open( QFile::ReadOnly ) )
  {
    return 0;
  }

  QUiLoader loader;
  QFileInfo fi( file );
  loader.setWorkingDirectory( fi.dir() );
  QWidget* widget = loader.load( &file, 0 );
  file.close();

  //get feature and set attribute information
  if ( mVectorLayer && mHasAssociatedFeature )
  {
    QgsFeature f;
    if ( mVectorLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
    {
      const QgsFields& fields = mVectorLayer->fields();
      QgsAttributes attrs = f.attributes();
      for ( int i = 0; i < attrs.count(); ++i )
      {
        if ( i < fields.count() )
        {
          QWidget* attWidget = widget->findChild<QWidget*>( fields[i].name() );
          if ( attWidget )
          {
            QgsAttributeEditor::createAttributeEditor( widget, attWidget, mVectorLayer, i, attrs[i] );
          }
        }
      }
    }
  }
  return widget;
}

void QgsFormAnnotationItem::setMapPosition( const QgsPoint& pos )
{
  QgsAnnotationItem::setMapPosition( pos );
  setFeatureForMapPosition();
}

void QgsFormAnnotationItem::paint( QPainter * painter )
{
  Q_UNUSED( painter );
}

void QgsFormAnnotationItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  if ( !painter || !mWidgetContainer )
  {
    return;
  }

  drawFrame( painter );
  if ( mMapPositionFixed )
  {
    drawMarkerSymbol( painter );
  }

  mWidgetContainer->setGeometry( QRectF( mOffsetFromReferencePoint.x() + mFrameBorderWidth / 2.0, mOffsetFromReferencePoint.y()
                                         + mFrameBorderWidth / 2.0, mFrameSize.width() - mFrameBorderWidth, mFrameSize.height()
                                         - mFrameBorderWidth ) );

  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

QSizeF QgsFormAnnotationItem::minimumFrameSize() const
{
  if ( mDesignerWidget )
  {
    QSizeF widgetMinSize = mDesignerWidget->minimumSize();
    return QSizeF( 2 * mFrameBorderWidth + widgetMinSize.width(), 2 * mFrameBorderWidth + widgetMinSize.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

QSizeF QgsFormAnnotationItem::preferredFrameSize() const
{
  if ( mDesignerWidget )
  {
    return mDesignerWidget->sizeHint();
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

void QgsFormAnnotationItem::writeXML( QDomDocument& doc ) const
{
  QDomElement documentElem = doc.documentElement();
  if ( documentElem.isNull() )
  {
    return;
  }

  QDomElement formAnnotationElem = doc.createElement( "FormAnnotationItem" );
  if ( mVectorLayer )
  {
    formAnnotationElem.setAttribute( "vectorLayer", mVectorLayer->id() );
  }
  formAnnotationElem.setAttribute( "hasFeature", mHasAssociatedFeature );
  formAnnotationElem.setAttribute( "feature", mFeature );
  formAnnotationElem.setAttribute( "designerForm", mDesignerForm );
  _writeXML( doc, formAnnotationElem );
  documentElem.appendChild( formAnnotationElem );
}

void QgsFormAnnotationItem::readXML( const QDomDocument& doc, const QDomElement& itemElem )
{
  mVectorLayer = 0;
  if ( itemElem.hasAttribute( "vectorLayer" ) )
  {
    mVectorLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( itemElem.attribute( "vectorLayer", "" ) ) );
    if ( mVectorLayer )
    {
      QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
    }
  }
  mHasAssociatedFeature = itemElem.attribute( "hasFeature", "0" ).toInt();
  mFeature = itemElem.attribute( "feature", "0" ).toInt();
  mDesignerForm = itemElem.attribute( "designerForm", "" );
  QDomElement annotationElem = itemElem.firstChildElement( "AnnotationItem" );
  if ( !annotationElem.isNull() )
  {
    _readXML( doc, annotationElem );
  }

  mDesignerWidget = createDesignerWidget( mDesignerForm );
  if ( mDesignerWidget )
  {
    mFrameBackgroundColor = mDesignerWidget->palette().color( QPalette::Window );
    mWidgetContainer->setWidget( mDesignerWidget );
  }
  updateVisibility();
}

void QgsFormAnnotationItem::setFeatureForMapPosition()
{
  if ( !mVectorLayer || !mMapCanvas )
  {
    return;
  }

  double halfIdentifyWidth = QgsMapTool::searchRadiusMU( mMapCanvas );
  QgsRectangle searchRect( mMapPosition.x() - halfIdentifyWidth, mMapPosition.y() - halfIdentifyWidth,
                           mMapPosition.x() + halfIdentifyWidth, mMapPosition.y() + halfIdentifyWidth );

  QgsFeatureIterator fit = mVectorLayer->getFeatures( QgsFeatureRequest().setFilterRect( searchRect ).setFlags( QgsFeatureRequest::NoGeometry | QgsFeatureRequest::ExactIntersect ).setSubsetOfAttributes( QgsAttributeList() ) );

  QgsFeature currentFeature;
  QgsFeatureId currentFeatureId = 0;
  bool featureFound = false;

  while ( fit.nextFeature( currentFeature ) )
  {
    currentFeatureId = currentFeature.id();
    featureFound = true;
    break;
  }

  mHasAssociatedFeature = featureFound;
  mFeature = currentFeatureId;

  //create new embedded widget
  mWidgetContainer->setWidget( 0 );
  delete mDesignerWidget;
  mDesignerWidget = createDesignerWidget( mDesignerForm );
  if ( mDesignerWidget )
  {
    mFrameBackgroundColor = mDesignerWidget->palette().color( QPalette::Window );
    mWidgetContainer->setWidget( mDesignerWidget );
  }
}

void QgsFormAnnotationItem::updateVisibility()
{
  bool visible = true;
  if ( mVectorLayer && mMapCanvas )
  {
    visible = mMapCanvas->layers().contains( mVectorLayer );
  }
  setVisible( visible );
}



