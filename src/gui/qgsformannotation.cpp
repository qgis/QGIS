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
#include "qgsgui.h"
#include "qgsfillsymbol.h"

#include <QDomElement>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QSettings>
#include <QUiLoader>
#include <QWidget>

QgsFormAnnotation::QgsFormAnnotation( QObject *parent )
  : QgsAnnotation( parent )
{}

QgsFormAnnotation *QgsFormAnnotation::clone() const
{
  std::unique_ptr< QgsFormAnnotation > c( new QgsFormAnnotation() );
  copyCommonProperties( c.get() );
  c->setDesignerForm( mDesignerForm );
  return c.release();
}

void QgsFormAnnotation::setDesignerForm( const QString &uiFile )
{
  mDesignerForm = uiFile;
  mDesignerWidget.reset( createDesignerWidget( uiFile ) );
  if ( mDesignerWidget )
  {
    mMinimumSize = mDesignerWidget->minimumSize();
    if ( auto *lFillSymbol = fillSymbol() )
    {
      QgsFillSymbol *newFill = lFillSymbol->clone();
      newFill->setColor( mDesignerWidget->palette().color( QPalette::Window ) );
      setFillSymbol( newFill );
    }
    // convert from size in pixels at 96 dpi to mm
    setFrameSizeMm( preferredFrameSize() / 3.7795275 );
  }
  emit appearanceChanged();
}

QWidget *QgsFormAnnotation::createDesignerWidget( const QString &filePath )
{
  QFile file( filePath );
  if ( !file.open( QFile::ReadOnly ) )
  {
    return nullptr;
  }

  QUiLoader loader;
  const QFileInfo fi( file );
  loader.setWorkingDirectory( fi.dir() );
  QWidget *widget = loader.load( &file, nullptr );
  file.close();

  //get feature and set attribute information
  const QgsAttributeEditorContext context;
  QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( mapLayer() );
  if ( vectorLayer && associatedFeature().isValid() )
  {
    const QgsFields fields = vectorLayer->fields();
    const QgsAttributes attrs = associatedFeature().attributes();
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i < fields.count() )
      {
        QWidget *attWidget = widget->findChild<QWidget *>( fields.at( i ).name() );
        if ( attWidget )
        {
          QgsEditorWidgetWrapper *eww = QgsGui::editorWidgetRegistry()->create( vectorLayer, i, attWidget, widget, context );
          if ( eww )
          {
            const QStringList additionalFields = eww->additionalFields();
            QVariantList additionalFieldValues;
            for ( const QString &additionalField : additionalFields )
            {
              const int index = vectorLayer->fields().indexFromName( additionalField );
              additionalFieldValues.insert( index, attrs.at( index ) );
            }
            eww->setValues( attrs.at( i ), additionalFieldValues );
          }
        }
      }
    }
  }
  return widget;
}

void QgsFormAnnotation::renderAnnotation( QgsRenderContext &context, QSizeF size ) const
{
  if ( !mDesignerWidget )
    return;

  // scale painter back to 96 dpi, so that forms look good even in layout prints
  const QgsScopedQPainterState painterState( context.painter() );
  const double scaleFactor = context.painter()->device()->logicalDpiX() / 96.0;
  context.painter()->scale( scaleFactor, scaleFactor );
  size /= scaleFactor;

  mDesignerWidget->setFixedSize( size.toSize() );
  context.painter()->setBrush( Qt::NoBrush );
  context.painter()->setPen( Qt::NoPen );
  mDesignerWidget->render( context.painter(), QPoint( 0, 0 ) );
}

QSizeF QgsFormAnnotation::minimumFrameSize() const
{
  if ( mDesignerWidget )
  {
    const QSizeF widgetMinSize = mMinimumSize;
    return QSizeF( contentsMargin().left() + contentsMargin().right() + widgetMinSize.width(),
                   contentsMargin().top() + contentsMargin().bottom() + widgetMinSize.height() );
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

void QgsFormAnnotation::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement formAnnotationElem = doc.createElement( QStringLiteral( "FormAnnotationItem" ) );
  formAnnotationElem.setAttribute( QStringLiteral( "designerForm" ), mDesignerForm );
  _writeXml( formAnnotationElem, doc, context );
  elem.appendChild( formAnnotationElem );
}

void QgsFormAnnotation::readXml( const QDomElement &itemElem, const QgsReadWriteContext &context )
{
  mDesignerForm = itemElem.attribute( QStringLiteral( "designerForm" ), QString() );
  const QDomElement annotationElem = itemElem.firstChildElement( QStringLiteral( "AnnotationItem" ) );
  if ( !annotationElem.isNull() )
  {
    _readXml( annotationElem, context );
  }
  // upgrade old layer
  if ( !mapLayer() && itemElem.hasAttribute( QStringLiteral( "vectorLayer" ) ) )
  {
    setMapLayer( QgsProject::instance()->mapLayer( itemElem.attribute( QStringLiteral( "vectorLayer" ) ) ) );
  }

  mDesignerWidget.reset( createDesignerWidget( mDesignerForm ) );
  if ( mDesignerWidget && fillSymbol() )
  {
    QgsFillSymbol *newFill = fillSymbol()->clone();
    newFill->setColor( mDesignerWidget->palette().color( QPalette::Window ) );
    setFillSymbol( newFill );
  }
}

void QgsFormAnnotation::setAssociatedFeature( const QgsFeature &feature )
{
  QgsAnnotation::setAssociatedFeature( feature );

  //create new embedded widget
  mDesignerWidget.reset( createDesignerWidget( mDesignerForm ) );
  if ( mDesignerWidget && fillSymbol() )
  {
    QgsFillSymbol *newFill = fillSymbol()->clone();
    newFill->setColor( mDesignerWidget->palette().color( QPalette::Window ) );
    setFillSymbol( newFill );
  }
  emit appearanceChanged();
}



