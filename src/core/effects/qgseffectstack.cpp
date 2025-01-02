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
#include "qgsrendercontext.h"
#include "qgsapplication.h"
#include "qgspainting.h"
#include <QPicture>

QgsEffectStack::QgsEffectStack( const QgsEffectStack &other )
  : QgsPaintEffect( other )
{
  //deep copy
  for ( int i = 0; i < other.count(); ++i )
  {
    appendEffect( other.effect( i )->clone() );
  }
}

QgsEffectStack::QgsEffectStack( QgsEffectStack &&other )
  : QgsPaintEffect( other )
{
  std::swap( mEffectList, other.mEffectList );
}

QgsEffectStack::QgsEffectStack( const QgsPaintEffect &effect )
{
  appendEffect( effect.clone() );
}

QgsEffectStack::~QgsEffectStack()
{
  clearStack();
}

QgsEffectStack &QgsEffectStack::operator=( const QgsEffectStack &rhs )
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

QgsEffectStack &QgsEffectStack::operator=( QgsEffectStack &&other )
{
  std::swap( mEffectList, other.mEffectList );
  mEnabled = other.enabled();
  return *this;
}

QgsPaintEffect *QgsEffectStack::create( const QVariantMap &map )
{
  QgsEffectStack *effect = new QgsEffectStack();
  effect->readProperties( map );
  return effect;
}

void QgsEffectStack::draw( QgsRenderContext &context )
{
  QPainter *destPainter = context.painter();

  //first, we build up a list of rendered effects
  //we do this moving backwards through the stack, so that each effect's results
  //becomes the source of the previous effect
  QPicture *sourcePic = new QPicture( *source() );
  QPicture *currentPic = sourcePic;
  QList< QPicture * > results;
  for ( int i = mEffectList.count() - 1; i >= 0; --i )
  {
    QgsPaintEffect *effect = mEffectList.at( i );
    if ( !effect->enabled() )
    {
      continue;
    }

    QPicture *pic = nullptr;
    if ( effect->type() == QLatin1String( "drawSource" ) )
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
    if ( mEffectList.at( i )->drawMode() != QgsPaintEffect::Render )
    {
      currentPic = resultPic;
    }
  }
  delete sourcePic;
  sourcePic = nullptr;

  context.setPainter( destPainter );
  //then, we render all the results in the opposite order
  for ( int i = 0; i < mEffectList.count(); ++i )
  {
    if ( !mEffectList[i]->enabled() )
    {
      continue;
    }

    QPicture *pic = results.takeLast();
    if ( mEffectList.at( i )->drawMode() != QgsPaintEffect::Modifier )
    {
      QgsPainting::drawPicture( context.painter(), QPointF( 0, 0 ), *pic );
    }
    delete pic;
  }
}

QgsEffectStack *QgsEffectStack::clone() const
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

  QDomElement effectElement = doc.createElement( QStringLiteral( "effect" ) );
  effectElement.setAttribute( QStringLiteral( "type" ), type() );
  effectElement.setAttribute( QStringLiteral( "enabled" ), mEnabled );

  bool ok = true;
  for ( QgsPaintEffect *effect : mEffectList )
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

  mEnabled = ( element.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );

  clearStack();

  //restore all child effects
  const QDomNodeList childNodes = element.childNodes();
  for ( int i = 0; i < childNodes.size(); ++i )
  {
    const QDomElement childElement = childNodes.at( i ).toElement();
    QgsPaintEffect *effect = QgsApplication::paintEffectRegistry()->createEffect( childElement );
    if ( effect )
      mEffectList << effect;
  }
  return true;
}

QVariantMap QgsEffectStack::properties() const
{
  QVariantMap props;
  return props;
}

void QgsEffectStack::readProperties( const QVariantMap &props )
{
  Q_UNUSED( props )
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

  delete mEffectList.at( index );
  mEffectList[index] = effect;
  return true;
}

QgsPaintEffect *QgsEffectStack::takeEffect( const int index )
{
  if ( index < 0 || index >= mEffectList.count() )
    return nullptr;

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
    return nullptr;
  }
}
