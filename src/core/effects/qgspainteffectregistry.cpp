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
                 QgsBlurEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "dropShadow", QObject::tr( "Drop Shadow" ),
                 QgsDropShadowEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "innerShadow", QObject::tr( "Inner Shadow" ),
                 QgsInnerShadowEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "effectStack", QObject::tr( "Stack" ),
                 QgsEffectStack::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "outerGlow", QObject::tr( "Outer Glow" ),
                 QgsOuterGlowEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "innerGlow", QObject::tr( "Inner Glow" ),
                 QgsInnerGlowEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "drawSource", QObject::tr( "Source" ),
                 QgsDrawSourceEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "transform", QObject::tr( "Transform" ),
                 QgsTransformEffect::create, nullptr ) );
  addEffectType( new QgsPaintEffectMetadata( "color", QObject::tr( "Colorise" ),
                 QgsColorEffect::create, nullptr ) );
}

QgsPaintEffectRegistry::~QgsPaintEffectRegistry()
{
  qDeleteAll( mMetadata );
}

QgsPaintEffectRegistry* QgsPaintEffectRegistry::instance()
{
  static QgsPaintEffectRegistry sInstance;
  return &sInstance;
}

QgsPaintEffectAbstractMetadata *QgsPaintEffectRegistry::effectMetadata( const QString &name ) const
{
  if ( mMetadata.contains( name ) )
    return mMetadata.value( name );
  else
    return nullptr;
}

bool QgsPaintEffectRegistry::addEffectType( QgsPaintEffectAbstractMetadata *metadata )
{
  if ( !metadata || mMetadata.contains( metadata->name() ) )
    return false;

  mMetadata[metadata->name()] = metadata;
  return true;
}

QgsPaintEffect *QgsPaintEffectRegistry::createEffect( const QString &name, const QgsStringMap &properties ) const
{
  if ( !mMetadata.contains( name ) )
    return nullptr;

  QgsPaintEffect* effect = mMetadata[name]->createPaintEffect( properties );
  return effect;
}

QgsPaintEffect *QgsPaintEffectRegistry::createEffect( const QDomElement &element ) const
{
  if ( element.isNull() )
  {
    return nullptr;
  }

  QString type = element.attribute( QString( "type" ) );

  QgsPaintEffect* effect = instance()->createEffect( type );
  if ( !effect )
    return nullptr;

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

QgsPaintEffect* QgsPaintEffectRegistry::defaultStack()
{
  //NOTE - also remember to update isDefaultStack below if making changes to this list
  QgsEffectStack* stack = new QgsEffectStack();
  QgsDropShadowEffect* dropShadow = new QgsDropShadowEffect();
  dropShadow->setEnabled( false );
  stack->appendEffect( dropShadow );
  QgsOuterGlowEffect* outerGlow = new QgsOuterGlowEffect();
  outerGlow->setEnabled( false );
  stack->appendEffect( outerGlow );
  stack->appendEffect( new QgsDrawSourceEffect() );
  QgsInnerShadowEffect* innerShadow = new QgsInnerShadowEffect();
  innerShadow->setEnabled( false );
  stack->appendEffect( innerShadow );
  QgsInnerGlowEffect* innerGlow = new QgsInnerGlowEffect();
  innerGlow->setEnabled( false );
  stack->appendEffect( innerGlow );
  return stack;
}

bool QgsPaintEffectRegistry::isDefaultStack( QgsPaintEffect* effect )
{
  QgsEffectStack* effectStack = dynamic_cast< QgsEffectStack* >( effect );
  if ( !effectStack )
    return false;

  if ( effectStack->count() != 5 )
    return false;

  for ( int i = 0; i < 5; ++i )
  {
    //only the third effect should be enabled
    if ( effectStack->effect( i )->enabled() != ( i == 2 ) )
      return false;
  }

  if ( !dynamic_cast< QgsDropShadowEffect* >( effectStack->effect( 0 ) ) )
    return false;
  if ( !dynamic_cast< QgsOuterGlowEffect* >( effectStack->effect( 1 ) ) )
    return false;
  if ( !dynamic_cast< QgsDrawSourceEffect* >( effectStack->effect( 2 ) ) )
    return false;
  if ( !dynamic_cast< QgsInnerShadowEffect* >( effectStack->effect( 3 ) ) )
    return false;
  if ( !dynamic_cast< QgsInnerGlowEffect* >( effectStack->effect( 4 ) ) )
    return false;

  //we don't go as far as to check the individual effect's properties
  return true;
}
