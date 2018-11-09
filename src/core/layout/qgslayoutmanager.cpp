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
#include "qgsreadwritecontext.h"

QgsLayoutManager::QgsLayoutManager( QgsProject *project )
  : QObject( project )
  , mProject( project )
{

}

QgsLayoutManager::~QgsLayoutManager()
{
  clear();
}

bool QgsLayoutManager::addLayout( QgsMasterLayoutInterface *layout )
{
  if ( !layout || mLayouts.contains( layout ) )
    return false;

  // check for duplicate name
  for ( QgsMasterLayoutInterface *l : qgis::as_const( mLayouts ) )
  {
    if ( l->name() == layout->name() )
    {
      delete layout;
      return false;
    }
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
  const QList< QgsMasterLayoutInterface * > layouts = mLayouts;
  for ( QgsMasterLayoutInterface *l : layouts )
  {
    removeLayout( l );
  }
}

QList<QgsMasterLayoutInterface *> QgsLayoutManager::layouts() const
{
  return mLayouts;
}

QList<QgsPrintLayout *> QgsLayoutManager::printLayouts() const
{
  QList<QgsPrintLayout *> result;
  const QList<QgsMasterLayoutInterface *> _layouts( mLayouts );
  result.reserve( _layouts.size() );
  for ( const auto &layout : _layouts )
  {
    QgsPrintLayout *_item( dynamic_cast<QgsPrintLayout *>( layout ) );
    if ( _item )
      result.push_back( _item );
  }
  return result;
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

        // some 2.x projects could end in a state where they had duplicated layout names. This is strictly forbidden in 3.x
        // so check for duplicate name in layouts already added
        int id = 2;
        bool isDuplicateName = false;
        QString originalName = l->name();
        do
        {
          isDuplicateName = false;
          for ( QgsMasterLayoutInterface *layout : qgis::as_const( mLayouts ) )
          {
            if ( l->name() == layout->name() )
            {
              isDuplicateName = true;
              break;
            }
          }
          if ( isDuplicateName )
          {
            l->setName( QStringLiteral( "%1 %2" ).arg( originalName ).arg( id ) );
            id++;
          }
        }
        while ( isDuplicateName );

        bool added = addLayout( l.release() );
        result = added && result;
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

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );
  for ( QgsMasterLayoutInterface *l : mLayouts )
  {
    QDomElement layoutElem = l->writeLayoutXml( doc, context );
    layoutsElem.appendChild( layoutElem );
  }
  return layoutsElem;
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
  if ( !addLayout( newLayout.release() ) )
  {
    return nullptr;
  }
  else
  {
    return l;
  }
}

QString QgsLayoutManager::generateUniqueTitle( QgsMasterLayoutInterface::Type type ) const
{
  QStringList names;
  names.reserve( mLayouts.size() );
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

