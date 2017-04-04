/***************************************************************************
                         qgsprocessingprovider.cpp
                         --------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#include "qgsprocessingprovider.h"
#include "qgsapplication.h"
#include "qgsvectorfilewriter.h"

QgsProcessingProvider::QgsProcessingProvider( QObject *parent SIP_TRANSFERTHIS )
  : QObject( parent )
{}


QgsProcessingProvider::~QgsProcessingProvider()
{
  qDeleteAll( mAlgorithms );
}

QIcon QgsProcessingProvider::icon() const
{
  return QgsApplication::getThemeIcon( "/processingAlgorithm.svg" );
}

QString QgsProcessingProvider::svgIconPath() const
{
  return QgsApplication::iconPath( "processingAlgorithm.svg" );
}

void QgsProcessingProvider::refreshAlgorithms()
{
  qDeleteAll( mAlgorithms );
  mAlgorithms.clear();
  loadAlgorithms();

  QMap< QString, QgsProcessingAlgorithm * >::const_iterator it = mAlgorithms.constBegin();
  for ( ; it != mAlgorithms.constEnd(); ++it )
  {
    it.value()->setProvider( this );
  }
  emit algorithmsLoaded();
}

QList<QgsProcessingAlgorithm *> QgsProcessingProvider::algorithms() const
{
  return mAlgorithms.values();
}

QgsProcessingAlgorithm *QgsProcessingProvider::algorithm( const QString &name ) const
{
  return mAlgorithms.value( name );
}

bool QgsProcessingProvider::addAlgorithm( QgsProcessingAlgorithm *algorithm )
{
  if ( !algorithm )
    return false;

  if ( mAlgorithms.contains( algorithm->name() ) )
    return false;

  mAlgorithms.insert( algorithm->name(), algorithm );
  return true;
}

QStringList QgsProcessingProvider::supportedOutputVectorLayerExtensions() const
{
  return QgsVectorFileWriter::supportedFormatExtensions();
}

