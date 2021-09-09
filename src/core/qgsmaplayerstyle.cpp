/***************************************************************************
  qgsmaplayersty.cpp
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerstyle.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsreadwritecontext.h"
#include "qgsmaplayer.h"


#include "qgslogger.h"

#include <QDomElement>
#include <QTextStream>


QgsMapLayerStyle::QgsMapLayerStyle( const QString &xmlData )
  : mXmlData( xmlData )
{
}

bool QgsMapLayerStyle::isValid() const
{
  return !mXmlData.isEmpty();
}

void QgsMapLayerStyle::clear()
{
  mXmlData.clear();
}

QString QgsMapLayerStyle::xmlData() const
{
  return mXmlData;
}

void QgsMapLayerStyle::readFromLayer( QgsMapLayer *layer )
{
  QString errorMsg;
  QDomDocument doc;
  const QgsReadWriteContext context;
  layer->exportNamedStyle( doc, errorMsg, context );
  if ( !errorMsg.isEmpty() )
  {
    QgsDebugMsg( "Failed to export style from layer: " + errorMsg );
    return;
  }

  mXmlData.clear();
  QTextStream stream( &mXmlData );
  doc.documentElement().save( stream, 0 );
}

void QgsMapLayerStyle::writeToLayer( QgsMapLayer *layer ) const
{
  if ( !isValid() )
  {
    return;
  }

  QDomDocument doc( QStringLiteral( "qgis" ) );
  if ( !doc.setContent( mXmlData ) )
  {
    QgsDebugMsg( QStringLiteral( "Failed to parse XML of previously stored XML data - this should not happen!" ) );
    return;
  }

  QString errorMsg;
  if ( !layer->importNamedStyle( doc, errorMsg ) )
  {
    QgsDebugMsg( "Failed to import style to layer: " + errorMsg );
  }
}

void QgsMapLayerStyle::readXml( const QDomElement &styleElement )
{
  mXmlData.clear();
  QTextStream stream( &mXmlData );
  styleElement.firstChildElement().save( stream, 0 );
}

void QgsMapLayerStyle::writeXml( QDomElement &styleElement ) const
{
  // the currently selected style has no content stored here (layer has all the information inside)
  if ( !isValid() )
    return;

  QDomDocument docX;
  docX.setContent( mXmlData );
  styleElement.appendChild( docX.documentElement() );
}

QgsMapLayerStyleOverride::~QgsMapLayerStyleOverride()
{
  if ( mLayer && mStyleOverridden )
    mLayer->styleManager()->restoreOverrideStyle();
}

void QgsMapLayerStyleOverride::setOverrideStyle( const QString &style )
{
  if ( mLayer )
  {
    if ( mStyleOverridden )
      mLayer->styleManager()->restoreOverrideStyle();

    mLayer->styleManager()->setOverrideStyle( style );
    mStyleOverridden = true;
  }
}
