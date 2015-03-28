/***************************************************************************
                            qgspainteffectregistry.cpp
                             ------------------------
    begin                : January 2015
    copyright            : (C) 2015 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspainteffectregistry.h"
#include "qgsblureffect.h"
#include "qgsshadoweffect.h"
#include "qgseffectstack.h"
#include "qgsgloweffect.h"
#include "qgstransformeffect.h"
#include "qgscoloreffect.h"

QgsPaintEffectAbstractMetadata::QgsPaintEffectAbstractMetadata( const QString& name, const QString& visibleName )
    : mName( name )
    , mVisibleName( visibleName )
{

}

QgsPaintEffectRegistry::QgsPaintEffectRegistry()
{
  //init registry with known effects
  addEffectType( new QgsPaintEffectMetadata( "blur", QObject::tr( "Blur" ),
                 QgsBlurEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "dropShadow", QObject::tr( "Drop Shadow" ),
                 QgsDropShadowEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "innerShadow", QObject::tr( "Inner Shadow" ),
                 QgsInnerShadowEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "effectStack", QObject::tr( "Stack" ),
                 QgsEffectStack::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "outerGlow", QObject::tr( "Outer Glow" ),
                 QgsOuterGlowEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "innerGlow", QObject::tr( "Inner Glow" ),
                 QgsInnerGlowEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "drawSource", QObject::tr( "Source" ),
                 QgsDrawSourceEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "transform", QObject::tr( "Transform" ),
                 QgsTransformEffect::create, NULL ) );
  addEffectType( new QgsPaintEffectMetadata( "color", QObject::tr( "Colorise" ),
                 QgsColorEffect::create, NULL ) );
}

QgsPaintEffectRegistry::~QgsPaintEffectRegistry()
{
  foreach ( QString name, mMetadata.keys() )
  {
    delete mMetadata[name];
  }
  mMetadata.clear();
}

QgsPaintEffectAbstractMetadata *QgsPaintEffectRegistry::effectMetadata( const QString &name ) const
{
  if ( mMetadata.contains( name ) )
    return mMetadata.value( name );
  else
    return NULL;
}

bool QgsPaintEffectRegistry::addEffectType( QgsPaintEffectAbstractMetadata *metadata )
{
  if ( metadata == NULL || mMetadata.contains( metadata->name() ) )
    return false;

  mMetadata[metadata->name()] = metadata;
  return true;
}

QgsPaintEffect *QgsPaintEffectRegistry::createEffect( const QString &name, const QgsStringMap &properties ) const
{
  if ( !mMetadata.contains( name ) )
    return NULL;

  QgsPaintEffect* effect = mMetadata[name]->createPaintEffect( properties );
  return effect;
}

QgsPaintEffect *QgsPaintEffectRegistry::createEffect( const QDomElement &element ) const
{
  if ( element.isNull() )
  {
    return NULL;
  }

  QString type = element.attribute( QString( "type" ) );

  QgsPaintEffect* effect = instance()->createEffect( type );
  if ( !effect )
    return NULL;

  effect->readProperties( element );
  return effect;
}

QStringList QgsPaintEffectRegistry::effects() const
{
  QStringList lst;
  QMap<QString, QgsPaintEffectAbstractMetadata*>::ConstIterator it = mMetadata.begin();
  for ( ; it != mMetadata.end(); ++it )
  {
    lst.append( it.key() );
  }
  return lst;
}
