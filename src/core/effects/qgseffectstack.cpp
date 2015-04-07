/***************************************************************************
                            qgseffectstack.cpp
                             -------------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
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

#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include <QPicture>

QgsEffectStack::QgsEffectStack()
    : QgsPaintEffect()
{

}

QgsEffectStack::QgsEffectStack( const QgsEffectStack &other )
    : QgsPaintEffect( other )
{
  //deep copy
  for ( int i = 0; i < other.count(); ++i )
  {
    appendEffect( other.effect( i )->clone() );
  }
}

QgsEffectStack::QgsEffectStack( const QgsPaintEffect &effect )
    : QgsPaintEffect()
{
  appendEffect( effect.clone() );
}

QgsEffectStack::~QgsEffectStack()
{
  clearStack();
}

QgsEffectStack &QgsEffectStack::operator=( const QgsEffectStack & rhs )
{
  if ( &rhs == this )
    return *this;

  //deep copy
  clearStack();
  for ( int i = 0; i < rhs.count(); ++i )
  {
    appendEffect( rhs.effect( i )->clone() );
  }
  mEnabled = rhs.enabled();
  return *this;
}

QgsPaintEffect *QgsEffectStack::create( const QgsStringMap &map )
{
  QgsEffectStack* effect = new QgsEffectStack();
  effect->readProperties( map );
  return effect;
}

void QgsEffectStack::draw( QgsRenderContext &context )
{
  QPainter* destPainter = context.painter();

  //first, we build up a list of rendered effects
  //we do this moving backwards through the stack, so that each effect's results
  //becomes the source of the previous effect
  QPicture* sourcePic = new QPicture( *source() );
  QPicture* currentPic = sourcePic;
  QList< QPicture* > results;
  for ( int i = mEffectList.count() - 1; i >= 0; --i )
  {
    QgsPaintEffect* effect = mEffectList[i];
    if ( !effect->enabled() )
    {
      continue;
    }

    QPicture* pic;
    if ( effect->type() == "drawSource" )
    {
      //draw source is always the original source, regardless of previous effect results
      pic = sourcePic;
    }
    else
    {
      pic = currentPic;
    }

    QPicture *resultPic = new QPicture();
    QPainter p( resultPic );
    context.setPainter( &p );
    //effect stack has it's own handling of the QPicture DPI issue, so
    //we disable QgsPaintEffect's internal workaround
    effect->requiresQPainterDpiFix = false;
    effect->render( *pic, context );
    effect->requiresQPainterDpiFix = true;
    p.end();

    results << resultPic;
    if ( mEffectList[i]->drawMode() != QgsPaintEffect::Render )
    {
      currentPic = resultPic;
    }
  }
  delete sourcePic;
  sourcePic = 0;

  context.setPainter( destPainter );
  //then, we render all the results in the opposite order
  for ( int i = 0; i < mEffectList.count(); ++i )
  {
    if ( !mEffectList[i]->enabled() )
    {
      continue;
    }

    QPicture* pic = results.takeLast();
    if ( mEffectList[i]->drawMode() != QgsPaintEffect::Modifier )
    {
      context.painter()->save();
      fixQPictureDpi( context.painter() );
      context.painter()->drawPicture( 0, 0, *pic );
      context.painter()->restore();

    }
    delete pic;
  }
}

QgsPaintEffect *QgsEffectStack::clone() const
{
  return new QgsEffectStack( *this );
}

bool QgsEffectStack::saveProperties( QDomDocument &doc, QDomElement &element ) const
{
  //effect stack needs to save all child effects
  if ( element.isNull() )
  {
    return false;
  }

  QDomElement effectElement = doc.createElement( "effect" );
  effectElement.setAttribute( QString( "type" ), type() );
  effectElement.setAttribute( QString( "enabled" ), mEnabled );

  bool ok = true;
  foreach ( QgsPaintEffect* effect, mEffectList )
  {
    if ( effect )
      ok = ok && effect->saveProperties( doc, effectElement );
  }

  element.appendChild( effectElement );
  return ok;
}

bool QgsEffectStack::readProperties( const QDomElement &element )
{
  if ( element.isNull() )
  {
    return false;
  }

  mEnabled = ( element.attribute( "enabled", "0" ) != "0" );

  clearStack();

  //restore all child effects
  QDomNodeList childNodes = element.childNodes();
  for ( int i = 0; i < childNodes.size(); ++i )
  {
    QDomElement childElement = childNodes.at( i ).toElement();
    QgsPaintEffect* effect = QgsPaintEffectRegistry::instance()->createEffect( childElement );
    if ( effect )
      mEffectList << effect;
  }
  return true;
}

QgsStringMap QgsEffectStack::properties() const
{
  QgsStringMap props;
  return props;
}

void QgsEffectStack::readProperties( const QgsStringMap &props )
{
  Q_UNUSED( props );
}

void QgsEffectStack::clearStack()
{
  qDeleteAll( mEffectList );
  mEffectList.clear();
}

void QgsEffectStack::appendEffect( QgsPaintEffect *effect )
{
  mEffectList.append( effect );
}

bool QgsEffectStack::insertEffect( const int index, QgsPaintEffect *effect )
{
  if ( index < 0 || index > mEffectList.count() )
    return false;
  if ( !effect )
    return false;

  mEffectList.insert( index, effect );
  return true;
}

bool QgsEffectStack::changeEffect( const int index, QgsPaintEffect *effect )
{
  if ( index < 0 || index >= mEffectList.count() )
    return false;
  if ( !effect )
    return false;

  delete mEffectList[index];
  mEffectList[index] = effect;
  return true;
}

QgsPaintEffect *QgsEffectStack::takeEffect( const int index )
{
  if ( index < 0 || index >= mEffectList.count() )
    return NULL;

  return mEffectList.takeAt( index );
}

QList<QgsPaintEffect *> *QgsEffectStack::effectList()
{
  return &mEffectList;
}

QgsPaintEffect *QgsEffectStack::effect( int index ) const
{
  if ( index >= 0 && index < mEffectList.count() )
  {
    return mEffectList.at( index );
  }
  else
  {
    return NULL;
  }
}
