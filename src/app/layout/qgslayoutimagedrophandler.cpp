/***************************************************************************
    qgslayoutimagedrophandler.cpp
    ------------------------------
    begin                : November 2019
    copyright            : (C) 2019 by nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutimagedrophandler.h"
#include "qgslayoutdesignerinterface.h"
#include "qgslayout.h"
#include "qgslayoutview.h"
#include "qgslayoutitempicture.h"

#include <QImageReader>
#include <QFileInfo>
#include <QMimeData>

QgsLayoutImageDropHandler::QgsLayoutImageDropHandler( QObject *parent )
  : QgsLayoutCustomDropHandler( parent )
{

}

bool QgsLayoutImageDropHandler::handleFileDrop( QgsLayoutDesignerInterface *iface, QPointF point, const QString &file )
{
  const QFileInfo fi( file );
  bool matched = false;
  bool svg = false;
  if ( fi.suffix().compare( "svg", Qt::CaseInsensitive ) == 0 )
  {
    matched = true;
    svg = true;
  }
  else
  {
    const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for ( const QByteArray &format : formats )
    {
      if ( fi.suffix().compare( format, Qt::CaseInsensitive ) == 0 )
      {
        matched = true;
        break;
      }
    }
  }

  if ( !matched )
    return false;

  if ( !iface->layout() )
    return false;

  std::unique_ptr< QgsLayoutItemPicture > item = std::make_unique< QgsLayoutItemPicture >( iface->layout() );

  const QgsLayoutPoint layoutPoint = iface->layout()->convertFromLayoutUnits( point, iface->layout()->units() );

  item->setPicturePath( file, svg ? QgsLayoutItemPicture::FormatSVG : QgsLayoutItemPicture::FormatRaster );

  // force a resize to the image's actual size
  item->setResizeMode( QgsLayoutItemPicture::FrameToImageSize );
  // and then move back to standard freeform image sizing
  item->setResizeMode( QgsLayoutItemPicture::Zoom );

  // we want the drop location to be the center of the placed item, because drag thumbnails are usually centered on the mouse cursor
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptMove( layoutPoint );

  // reset to standard top-left reference point location
  item->setReferencePoint( QgsLayoutItem::UpperLeft );

  // and auto select new item for convenience
  QList< QgsLayoutItem * > newSelection;
  newSelection << item.get();
  iface->layout()->addLayoutItem( item.release() );
  iface->layout()->deselectAll();
  iface->selectItems( newSelection );

  return true;
}

bool QgsLayoutImageDropHandler::handlePaste( QgsLayoutDesignerInterface *iface, QPointF pastePoint, const QMimeData *data, QList<QgsLayoutItem *> &pastedItems )
{
  if ( !data->hasImage() )
    return false;

  const QgsLayoutPoint layoutPoint = iface->layout()->convertFromLayoutUnits( pastePoint, iface->layout()->units() );
  std::unique_ptr< QgsLayoutItemPicture > item = std::make_unique< QgsLayoutItemPicture >( iface->layout() );

  const QByteArray imageData = data->data( QStringLiteral( "application/x-qt-image" ) );
  if ( imageData.isEmpty() )
    return false;

  const QByteArray encoded = imageData.toBase64();

  QString path( encoded );
  path.prepend( QLatin1String( "base64:" ) );

  item->setPicturePath( path, QgsLayoutItemPicture::FormatRaster );

  // force a resize to the image's actual size
  item->setResizeMode( QgsLayoutItemPicture::FrameToImageSize );
  // and then move back to standard freeform image sizing
  item->setResizeMode( QgsLayoutItemPicture::Zoom );

  // we want the drop location to be the center of the placed item, because drag thumbnails are usually centered on the mouse cursor
  item->setReferencePoint( QgsLayoutItem::Middle );
  item->attemptMove( layoutPoint );

  // reset to standard top-left reference point location
  item->setReferencePoint( QgsLayoutItem::UpperLeft );

  pastedItems << item.get();
  iface->layout()->addLayoutItem( item.release() );

  return true;
}
