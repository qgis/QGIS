/***************************************************************************
  qgsmaplayerstylemanager.cpp
  --------------------------------------
  Date                 : January 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerstylemanager.h"

#include "qgslogger.h"
#include "qgsmaplayer.h"

#include <QDomElement>
#include <QTemporaryFile>
#include <QTextStream>

QgsMapLayerStyleManager::QgsMapLayerStyleManager( QgsMapLayer* layer )
    : mLayer( layer )
{
  reset();
}

void QgsMapLayerStyleManager::reset()
{
  mStyles.insert( QString(), QgsMapLayerStyle() ); // insert entry for the default current style
  mCurrentStyle.clear();
}

void QgsMapLayerStyleManager::readXml( const QDomElement& mgrElement )
{
  mCurrentStyle = mgrElement.attribute( "current" );

  mStyles.clear();
  QDomElement ch = mgrElement.firstChildElement( "map-layer-style" );
  while ( !ch.isNull() )
  {
    QString name = ch.attribute( "name" );
    QgsMapLayerStyle style;
    style.readXml( ch );
    mStyles.insert( name, style );

    ch = ch.nextSiblingElement( "map-layer-style" );
  }
}

void QgsMapLayerStyleManager::writeXml( QDomElement& mgrElement ) const
{
  QDomDocument doc = mgrElement.ownerDocument();
  mgrElement.setAttribute( "current", mCurrentStyle );

  foreach ( const QString& name, styles() )
  {
    QDomElement ch = doc.createElement( "map-layer-style" );
    ch.setAttribute( "name", name );
    mStyles[name].writeXml( ch );
    mgrElement.appendChild( ch );
  }
}

QStringList QgsMapLayerStyleManager::styles() const
{
  return mStyles.keys();
}

QgsMapLayerStyle QgsMapLayerStyleManager::style( const QString& name ) const
{
  if ( name == mCurrentStyle )
  {
    // current style's entry is always kept invalid - get the style data from layer's properties
    QgsMapLayerStyle curr;
    curr.readFromLayer( mLayer );
    return curr;
  }

  return mStyles.value( name );
}

bool QgsMapLayerStyleManager::addStyle( const QString& name, const QgsMapLayerStyle& style )
{
  if ( mStyles.contains( name ) )
    return false;
  if ( !style.isValid() )
    return false;

  mStyles.insert( name, style );
  return true;
}

bool QgsMapLayerStyleManager::addStyleFromLayer( const QString& name )
{
  QgsMapLayerStyle style;
  style.readFromLayer( mLayer );
  return addStyle( name, style );
}

bool QgsMapLayerStyleManager::removeStyle( const QString& name )
{
  if ( !mStyles.contains( name ) )
    return false;
  if ( mStyles.count() == 1 )
    return false; // cannot remove the last one

  // change to a different style if this one is the current
  if ( mCurrentStyle == name )
  {
    QStringList keys = mStyles.keys();
    QString newCurrent = keys[0];
    if ( newCurrent == name )
      newCurrent = keys[1]; // there must be at least one more
    setCurrentStyle( newCurrent );
  }

  mStyles.remove( name );
  return true;
}

bool QgsMapLayerStyleManager::renameStyle( const QString& name, const QString& newName )
{
  if ( !mStyles.contains( name ) || mStyles.contains( newName ) )
    return false;

  if ( name == mCurrentStyle )
    mCurrentStyle = newName;

  mStyles[newName] = mStyles[name];
  mStyles.remove( name );
  return true;
}

QString QgsMapLayerStyleManager::currentStyle() const
{
  return mCurrentStyle;
}

bool QgsMapLayerStyleManager::setCurrentStyle( const QString& name )
{
  if ( !mStyles.contains( name ) )
    return false;

  if ( mCurrentStyle == name )
    return true; // nothing to do

  mStyles[mCurrentStyle].readFromLayer( mLayer ); // sync before unloading it
  mCurrentStyle = name;
  mStyles[mCurrentStyle].writeToLayer( mLayer );
  mStyles[mCurrentStyle].clear(); // current style does not keep any stored data
  mLayer->triggerRepaint();
  return true;
}


// -----

QgsMapLayerStyle::QgsMapLayerStyle()
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

QString QgsMapLayerStyle::dump() const
{
  return mXmlData;
}

void QgsMapLayerStyle::readFromLayer( QgsMapLayer* layer )
{
  QString errorMsg;
  QDomDocument doc;
  layer->exportNamedStyle( doc, errorMsg );
  if ( !errorMsg.isEmpty() )
  {
    QgsDebugMsg( "Failed to export style from layer: " + errorMsg );
    return;
  }

  mXmlData.clear();
  QTextStream stream( &mXmlData );
  doc.save( stream, 0 );
}

void QgsMapLayerStyle::writeToLayer( QgsMapLayer* layer ) const
{
  // QgsMapLayer does not have a importNamedStyle() method - working it around like this
  QTemporaryFile f;
  f.open();
  f.write( mXmlData );
  f.flush();

  bool res;
  QString status = layer->loadNamedStyle( f.fileName(), res );
  if ( !res )
    QgsDebugMsg( "Failed to import style to layer: " + status );
}

void QgsMapLayerStyle::readXml( const QDomElement& styleElement )
{
  mXmlData.clear();
  QTextStream stream( &mXmlData );
  styleElement.firstChildElement().save( stream, 0 );
}

void QgsMapLayerStyle::writeXml( QDomElement& styleElement ) const
{
  // the currently selected style has no content stored here (layer has all the information inside)
  if ( !isValid() )
    return;

  QDomDocument docX;
  docX.setContent( mXmlData );
  styleElement.appendChild( docX.documentElement() );
}
