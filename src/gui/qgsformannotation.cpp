/***************************************************************************
                              qgsformannotation.cpp
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

#include "qgsformannotation.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetregistry.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
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

QgsFormAnnotation::QgsFormAnnotation(QgsVectorLayer* vlayer, bool hasFeature, int feature )
    : QgsAnnotation()
    , mDesignerWidget( nullptr )
    , mVectorLayer( vlayer )
    , mHasAssociatedFeature( hasFeature )
    , mFeature( feature )
{
  if ( mVectorLayer ) //default to the layers edit form
  {
    mDesignerForm = mVectorLayer->annotationForm();
    QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
#if 0
    QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
    QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
#endif
  }

  setFeatureForMapPosition();
}

QgsFormAnnotation::~QgsFormAnnotation()
{
  delete mDesignerWidget;
}

void QgsFormAnnotation::setDesignerForm( const QString& uiFile )
{
  mDesignerForm = uiFile;
  delete mDesignerWidget;
  mDesignerWidget = createDesignerWidget( uiFile );
  if ( mDesignerWidget )
  {
    mMinimumSize = mDesignerWidget->minimumSize();
    setFrameBackgroundColor( mDesignerWidget->palette().color( QPalette::Window ) );
    setFrameSize( preferredFrameSize() );
  }
  emit appearanceChanged();
}

QWidget* QgsFormAnnotation::createDesignerWidget( const QString& filePath )
{
  QFile file( filePath );
  if ( !file.open( QFile::ReadOnly ) )
  {
    return nullptr;
  }

  QUiLoader loader;
  QFileInfo fi( file );
  loader.setWorkingDirectory( fi.dir() );
  QWidget* widget = loader.load( &file, nullptr );
  file.close();

  //get feature and set attribute information
  QgsAttributeEditorContext context;
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
          QWidget* attWidget = widget->findChild<QWidget*>( fields.at( i ).name() );
          if ( attWidget )
          {
            QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( mVectorLayer, i, attWidget, widget, context );
            if ( eww )
            {
              eww->setValue( attrs.at( i ) );
            }
          }
        }
      }
    }
  }
  return widget;
}

#if 0
void QgsFormAnnotation::setMapPosition( const QgsPoint& pos )
{
  QgsAnnotationItem::setMapPosition( pos );
  setFeatureForMapPosition();
}
#endif

void QgsFormAnnotation::renderAnnotation( QgsRenderContext& context, QSizeF size ) const
{
  if ( !mDesignerWidget )
    return;

  mDesignerWidget->setFixedSize( size.toSize() );
  context.painter()->setBrush( Qt::NoBrush );
  context.painter()->setPen( Qt::NoPen );
  mDesignerWidget->render( context.painter(), QPoint( 0, 0 ) );
}

QSizeF QgsFormAnnotation::minimumFrameSize() const
{
  if ( mDesignerWidget )
  {
    QSizeF widgetMinSize = mMinimumSize;
    return QSizeF( 2 * frameBorderWidth() + widgetMinSize.width(), 2 * frameBorderWidth() + widgetMinSize.height() );
  }
  else
  {
    return QSizeF( 0, 0 );
  }
}

QSizeF QgsFormAnnotation::preferredFrameSize() const
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

void QgsFormAnnotation::writeXml( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement formAnnotationElem = doc.createElement( QStringLiteral( "FormAnnotationItem" ) );
  if ( mVectorLayer )
  {
    formAnnotationElem.setAttribute( QStringLiteral( "vectorLayer" ), mVectorLayer->id() );
  }
  formAnnotationElem.setAttribute( QStringLiteral( "hasFeature" ), mHasAssociatedFeature );
  formAnnotationElem.setAttribute( QStringLiteral( "feature" ), mFeature );
  formAnnotationElem.setAttribute( QStringLiteral( "designerForm" ), mDesignerForm );
  _writeXml( formAnnotationElem, doc );
  elem.appendChild( formAnnotationElem );
}

void QgsFormAnnotation::readXml( const QDomElement& itemElem, const QDomDocument& doc )
{
  mVectorLayer = nullptr;
  if ( itemElem.hasAttribute( QStringLiteral( "vectorLayer" ) ) )
  {
    mVectorLayer = dynamic_cast<QgsVectorLayer*>( QgsProject::instance()->mapLayer( itemElem.attribute( QStringLiteral( "vectorLayer" ), QLatin1String( "" ) ) ) );
    if ( mVectorLayer )
    {
      QObject::connect( mVectorLayer, SIGNAL( layerModified() ), this, SLOT( setFeatureForMapPosition() ) );
#if 0
      QObject::connect( mMapCanvas, SIGNAL( renderComplete( QPainter* ) ), this, SLOT( setFeatureForMapPosition() ) );
      QObject::connect( mMapCanvas, SIGNAL( layersChanged() ), this, SLOT( updateVisibility() ) );
#endif
    }
  }
  mHasAssociatedFeature = itemElem.attribute( QStringLiteral( "hasFeature" ), QStringLiteral( "0" ) ).toInt();
  mFeature = itemElem.attribute( QStringLiteral( "feature" ), QStringLiteral( "0" ) ).toInt();
  mDesignerForm = itemElem.attribute( QStringLiteral( "designerForm" ), QLatin1String( "" ) );
  QDomElement annotationElem = itemElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
  if ( !annotationElem.isNull() )
  {
    _readXml( annotationElem, doc );
  }

  mDesignerWidget = createDesignerWidget( mDesignerForm );
  if ( mDesignerWidget )
  {
    setFrameBackgroundColor( mDesignerWidget->palette().color( QPalette::Window ) );
  }
  updateVisibility();
}

void QgsFormAnnotation::setFeatureForMapPosition()
{
  if ( !mVectorLayer)
  {
    return;
  }

  double halfIdentifyWidth = 0; //QgsMapTool::searchRadiusMU( mMapCanvas );
  QgsRectangle searchRect( mapPosition().x() - halfIdentifyWidth, mapPosition().y() - halfIdentifyWidth,
                           mapPosition().x() + halfIdentifyWidth, mapPosition().y() + halfIdentifyWidth );

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
  delete mDesignerWidget;
  mDesignerWidget = createDesignerWidget( mDesignerForm );
  if ( mDesignerWidget )
  {
    setFrameBackgroundColor( mDesignerWidget->palette().color( QPalette::Window ) );
  }
  emit appearanceChanged();
}

void QgsFormAnnotation::updateVisibility()
{
  bool visible = true;
#if 0
  if ( mVectorLayer && mMapCanvas )
  {
    visible = mMapCanvas->layers().contains( mVectorLayer );
  }
#endif
  setVisible( visible );
}



