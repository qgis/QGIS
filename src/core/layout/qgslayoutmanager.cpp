/***************************************************************************
    qgslayoutmanager.cpp
    --------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmanager.h"
#include "qgslayout.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgslayoutundostack.h"
#include "qgsprintlayout.h"
#include "qgsreport.h"
#include "qgscompositionconverter.h"

QgsLayoutManager::QgsLayoutManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsLayoutManager::~QgsLayoutManager()
{
  clear();
}

bool QgsLayoutManager::addComposition( QgsComposition *composition )
{
  if ( !composition )
    return false;

  // check for duplicate name
  Q_FOREACH ( QgsComposition *c, mCompositions )
  {
    if ( c->name() == composition->name() )
      return false;
  }

  mCompositions << composition;
  mProject->setDirty( true );
  return true;
}

bool QgsLayoutManager::addLayout( QgsMasterLayoutInterface *layout )
{
  if ( !layout )
    return false;

  // check for duplicate name
  for ( QgsMasterLayoutInterface *l : qgis::as_const( mLayouts ) )
  {
    if ( l->name() == layout->name() )
      return false;
  }

  // ugly, but unavoidable for interfaces...
  if ( QgsPrintLayout *l = dynamic_cast< QgsPrintLayout * >( layout ) )
  {
    connect( l, &QgsPrintLayout::nameChanged, this, [this, l]( const QString & newName )
    {
      emit layoutRenamed( l, newName );
    } );
  }
  else if ( QgsReport *r = dynamic_cast< QgsReport * >( layout ) )
  {
    connect( r, &QgsReport::nameChanged, this, [this, r]( const QString & newName )
    {
      emit layoutRenamed( r, newName );
    } );
  }

  emit layoutAboutToBeAdded( layout->name() );
  mLayouts << layout;
  emit layoutAdded( layout->name() );
  mProject->setDirty( true );
  return true;
}

bool QgsLayoutManager::removeComposition( QgsComposition *composition )
{
  if ( !composition )
    return false;

  if ( !mCompositions.contains( composition ) )
    return false;

  mCompositions.removeAll( composition );
  delete composition;
  mProject->setDirty( true );
  return true;
}

bool QgsLayoutManager::removeLayout( QgsMasterLayoutInterface *layout )
{
  if ( !layout )
    return false;

  if ( !mLayouts.contains( layout ) )
    return false;

  QString name = layout->name();
  emit layoutAboutToBeRemoved( name );
  mLayouts.removeAll( layout );
  delete layout;
  emit layoutRemoved( name );
  mProject->setDirty( true );
  return true;
}

void QgsLayoutManager::clear()
{
  Q_FOREACH ( QgsComposition *c, mCompositions )
  {
    removeComposition( c );
  }
  const QList< QgsMasterLayoutInterface * > layouts = mLayouts;
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    removeLayout( l );
  }
}

QList<QgsComposition *> QgsLayoutManager::compositions() const
{
  return mCompositions;
}

QList<QgsMasterLayoutInterface *> QgsLayoutManager::layouts() const
{
  return mLayouts;
}

QgsComposition *QgsLayoutManager::compositionByName( const QString &name ) const
{
  Q_FOREACH ( QgsComposition *c, mCompositions )
  {
    if ( c->name() == name )
      return c;
  }
  return nullptr;
}

QgsMasterLayoutInterface *QgsLayoutManager::layoutByName( const QString &name ) const
{
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    if ( l->name() == name )
      return l;
  }
  return nullptr;
}

bool QgsLayoutManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  clear();

  QDomElement layoutsElem = element;
  if ( element.tagName() != QStringLiteral( "Layouts" ) )
  {
    layoutsElem = element.firstChildElement( QStringLiteral( "Layouts" ) );
  }
  if ( layoutsElem.isNull() )
  {
    // handle legacy projects
    layoutsElem = doc.documentElement();
  }

  //restore each composer
  bool result = true;
  QDomNodeList composerNodes = element.elementsByTagName( QStringLiteral( "Composer" ) );
  for ( int i = 0; i < composerNodes.size(); ++i )
  {
    // This legacy title is the Composer "title" (that can be overridden by the Composition "name")
    QString legacyTitle = composerNodes.at( i ).toElement().attribute( QStringLiteral( "title" ) );
    // Convert compositions to layouts
    QDomNodeList compositionNodes = composerNodes.at( i ).toElement().elementsByTagName( QStringLiteral( "Composition" ) );
    for ( int j = 0; j < compositionNodes.size(); ++j )
    {
      std::unique_ptr< QgsPrintLayout > l( QgsCompositionConverter::createLayoutFromCompositionXml( compositionNodes.at( j ).toElement(), mProject ) );
      if ( l )
      {
        if ( l->name().isEmpty() )
          l->setName( legacyTitle );
        result = result && addLayout( l.release() );
      }
    }
  }

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );

  // restore layouts
  const QDomNodeList layoutNodes = layoutsElem.childNodes();
  for ( int i = 0; i < layoutNodes.size(); ++i )
  {
    if ( layoutNodes.at( i ).nodeName() != QStringLiteral( "Layout" ) )
      continue;

    std::unique_ptr< QgsPrintLayout > l = qgis::make_unique< QgsPrintLayout >( mProject );
    l->undoStack()->blockCommands( true );
    if ( !l->readLayoutXml( layoutNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    l->undoStack()->blockCommands( false );
    if ( addLayout( l.get() ) )
    {
      ( void )l.release(); // ownership was transferred successfully
    }
    else
    {
      result = false;
    }
  }
  //reports
  const QDomNodeList reportNodes = element.elementsByTagName( QStringLiteral( "Report" ) );
  for ( int i = 0; i < reportNodes.size(); ++i )
  {
    std::unique_ptr< QgsReport > r = qgis::make_unique< QgsReport >( mProject );
    if ( !r->readLayoutXml( reportNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    if ( addLayout( r.get() ) )
    {
      ( void )r.release(); // ownership was transferred successfully
    }
    else
    {
      result = false;
    }
  }
  return result;
}

QDomElement QgsLayoutManager::writeXml( QDomDocument &doc ) const
{
  QDomElement layoutsElem = doc.createElement( QStringLiteral( "Layouts" ) );
  Q_FOREACH ( QgsComposition *c, mCompositions )
  {
    QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );

    layoutsElem.appendChild( composerElem );

    c->writeXml( composerElem, doc );
    c->atlasComposition().writeXml( composerElem, doc );
  }

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    QDomElement layoutElem = l->writeLayoutXml( doc, context );
    layoutsElem.appendChild( layoutElem );
  }
  return layoutsElem;
}

bool QgsLayoutManager::saveAsTemplate( const QString &name, QDomDocument &doc ) const
{
  QgsComposition *c = compositionByName( name );
  if ( !c )
    return false;

  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c->writeXml( composerElem, doc );
  c->atlasComposition().writeXml( composerElem, doc );
  return true;
}

QgsMasterLayoutInterface *QgsLayoutManager::duplicateLayout( const QgsMasterLayoutInterface *layout, const QString &newName )
{
  if ( !layout )
    return nullptr;

  std::unique_ptr< QgsMasterLayoutInterface > newLayout( layout->clone() );
  if ( !newLayout )
  {
    return nullptr;
  }

  newLayout->setName( newName );
  QgsMasterLayoutInterface *l = newLayout.get();
  if ( !addLayout( l ) )
  {
    return nullptr;
  }
  else
  {
    ( void )newLayout.release(); //ownership was transferred successfully
    return l;
  }
}

QString QgsLayoutManager::generateUniqueTitle( QgsMasterLayoutInterface::Type type ) const
{
  QStringList names;
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    names << l->name();
  }
  QString name;
  int id = 1;
  while ( name.isEmpty() || names.contains( name ) )
  {
    switch ( type )
    {
      case QgsMasterLayoutInterface::PrintLayout:
        name = tr( "Layout %1" ).arg( id );
        break;
      case QgsMasterLayoutInterface::Report:
        name = tr( "Report %1" ).arg( id );
        break;
    }
    id++;
  }
  return name;
}

QgsComposition *QgsLayoutManager::createCompositionFromXml( const QDomElement &element, const QDomDocument &doc ) const
{
  QDomNodeList compositionNodeList = element.elementsByTagName( QStringLiteral( "Composition" ) );

  if ( compositionNodeList.size() > 0 )
  {
    std::unique_ptr< QgsComposition > c( new QgsComposition( mProject ) );
    QDomElement compositionElem = compositionNodeList.at( 0 ).toElement();
    if ( !c->readXml( compositionElem, doc ) )
    {
      return nullptr;
    }

    // read atlas parameters - must be done before adding items
    QDomElement atlasElem = element.firstChildElement( QStringLiteral( "Atlas" ) );
    c->atlasComposition().readXml( atlasElem, doc );

    //read and restore all the items
    c->addItemsFromXml( element, doc );

    //make sure z values are consistent
    c->refreshZList();
    return c.release();
  }
  else
  {
    return nullptr;
  }
}
