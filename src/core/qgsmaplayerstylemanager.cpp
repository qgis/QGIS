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
#include "qgsmaplayerstyle.h"
#include "qgsmaplayer.h"

#include "qgslogger.h"

#include <QDomElement>
#include <QTextStream>

QgsMapLayerStyleManager::QgsMapLayerStyleManager( QgsMapLayer *layer )
  : mLayer( layer )

{
  reset();
}

QString QgsMapLayerStyleManager::defaultStyleName()
{
  return tr( "default" );
}


void QgsMapLayerStyleManager::reset()
{
  mStyles.insert( defaultStyleName(), QgsMapLayerStyle() ); // insert entry for the default current style
  mCurrentStyle = defaultStyleName();
}

void QgsMapLayerStyleManager::readXml( const QDomElement &mgrElement )
{
  mCurrentStyle = mgrElement.attribute( QStringLiteral( "current" ) );
  if ( mCurrentStyle.isEmpty() )
  {
    // For old project made with QGIS 2, we migrate to "default".
    mCurrentStyle = defaultStyleName();
  }

  mStyles.clear();
  QDomElement ch = mgrElement.firstChildElement( QStringLiteral( "map-layer-style" ) );
  while ( !ch.isNull() )
  {
    QString name = ch.attribute( QStringLiteral( "name" ) );
    if ( name.isEmpty() )
    {
      // For old project made with QGIS 2, we migrate to "default".
      name = defaultStyleName();
    }
    QgsMapLayerStyle style;
    style.readXml( ch );
    mStyles.insert( name, style );

    ch = ch.nextSiblingElement( QStringLiteral( "map-layer-style" ) );
  }
}

void QgsMapLayerStyleManager::writeXml( QDomElement &mgrElement ) const
{
  QDomDocument doc = mgrElement.ownerDocument();
  mgrElement.setAttribute( QStringLiteral( "current" ), mCurrentStyle );

  const auto constStyles = styles();
  for ( const QString &name : constStyles )
  {
    QDomElement ch = doc.createElement( QStringLiteral( "map-layer-style" ) );
    ch.setAttribute( QStringLiteral( "name" ), name );
    mStyles[name].writeXml( ch );
    mgrElement.appendChild( ch );
  }
}

QStringList QgsMapLayerStyleManager::styles() const
{
  return mStyles.keys();
}

QMap<QString, QgsMapLayerStyle> QgsMapLayerStyleManager::mapLayerStyles() const
{
  return mStyles;
}

QgsMapLayerStyle QgsMapLayerStyleManager::style( const QString &name ) const
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

bool QgsMapLayerStyleManager::addStyle( const QString &name, const QgsMapLayerStyle &style )
{
  if ( mStyles.contains( name ) )
    return false;
  if ( !style.isValid() )
    return false;

  mStyles.insert( name, style );
  emit styleAdded( name );
  return true;
}

bool QgsMapLayerStyleManager::addStyleFromLayer( const QString &name )
{
  QgsMapLayerStyle style;
  style.readFromLayer( mLayer );
  return addStyle( name, style );
}

bool QgsMapLayerStyleManager::removeStyle( const QString &name )
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
  emit styleRemoved( name );
  return true;
}

bool QgsMapLayerStyleManager::renameStyle( const QString &name, const QString &newName )
{
  if ( !mStyles.contains( name ) || mStyles.contains( newName ) )
    return false;

  if ( name == mCurrentStyle )
    mCurrentStyle = newName;

  mStyles[newName] = mStyles[name];
  mStyles.remove( name );
  emit styleRenamed( name, newName );
  return true;
}

QString QgsMapLayerStyleManager::currentStyle() const
{
  return mCurrentStyle;
}

bool QgsMapLayerStyleManager::setCurrentStyle( const QString &name )
{
  if ( !mStyles.contains( name ) )
    return false;

  if ( mCurrentStyle == name )
    return true; // nothing to do

  mStyles[mCurrentStyle].readFromLayer( mLayer ); // sync before unloading it
  mCurrentStyle = name;
  mStyles[mCurrentStyle].writeToLayer( mLayer );
  mStyles[mCurrentStyle].clear(); // current style does not keep any stored data
  emit currentStyleChanged( mCurrentStyle );

  mLayer->triggerRepaint();
  return true;
}

bool QgsMapLayerStyleManager::setOverrideStyle( const QString &styleDef )
{
  if ( mOverriddenOriginalStyle )
    return false; // cannot override the style more than once!

  mLayer->blockSignals( true );
  if ( mStyles.contains( styleDef ) )
  {
    mOverriddenOriginalStyle = new QgsMapLayerStyle;
    mOverriddenOriginalStyle->readFromLayer( mLayer );

    // apply style name
    mStyles[styleDef].writeToLayer( mLayer );
  }
  else if ( styleDef.startsWith( '<' ) )
  {
    mOverriddenOriginalStyle = new QgsMapLayerStyle;
    mOverriddenOriginalStyle->readFromLayer( mLayer );

    // apply style XML
    const QgsMapLayerStyle overrideStyle( styleDef );
    overrideStyle.writeToLayer( mLayer );
  }
  mLayer->blockSignals( false );

  return true;
}

bool QgsMapLayerStyleManager::restoreOverrideStyle()
{
  if ( !mOverriddenOriginalStyle )
    return false;

  mLayer->blockSignals( true );
  mOverriddenOriginalStyle->writeToLayer( mLayer );
  mLayer->blockSignals( false );

  delete mOverriddenOriginalStyle;
  mOverriddenOriginalStyle = nullptr;
  return true;
}

bool QgsMapLayerStyleManager::isDefault( const QString &styleName )
{
  return styleName == defaultStyleName();
}

void QgsMapLayerStyleManager::copyStylesFrom( QgsMapLayerStyleManager *other )
{
  const QStringList styleNames = other->mStyles.keys();

  for ( const QString &styleName : styleNames )
  {
    mStyles.remove( styleName );
    addStyle( styleName, other->style( styleName ) );
  }
}
