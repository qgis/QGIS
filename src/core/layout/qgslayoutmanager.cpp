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

#include "qgscompositionconverter.h"
#include "qgslayout.h"
#include "qgslayoutundostack.h"
#include "qgsprintlayout.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgsreport.h"
#include "qgsruntimeprofiler.h"
#include "qgsstyleentityvisitor.h"

#include "moc_qgslayoutmanager.cpp"

QgsLayoutManager::QgsLayoutManager( QgsProject *project )
  : QgsAbstractProjectStoredObjectManager( project )
{
  // proxy base class generic signals to specific layout signals
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeAdded, this, &QgsLayoutManager::layoutAboutToBeAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAdded, this, &QgsLayoutManager::layoutAdded );
  connect( this, &QgsProjectStoredObjectManagerBase::objectRemoved, this, &QgsLayoutManager::layoutRemoved );
  connect( this, &QgsProjectStoredObjectManagerBase::objectAboutToBeRemoved, this, &QgsLayoutManager::layoutAboutToBeRemoved );
}

QgsLayoutManager::~QgsLayoutManager()
{
  clearObjects();
}

bool QgsLayoutManager::addLayout( QgsMasterLayoutInterface *layout )
{
  return addObject( layout );
}

bool QgsLayoutManager::removeLayout( QgsMasterLayoutInterface *layout )
{
  return removeObject( layout );
}

void QgsLayoutManager::clear()
{
  clearObjects();
}

QList<QgsMasterLayoutInterface *> QgsLayoutManager::layouts() const
{
  return mObjects;
}

QList<QgsPrintLayout *> QgsLayoutManager::printLayouts() const
{
  QList<QgsPrintLayout *> result;
  const QList<QgsMasterLayoutInterface *> constLayouts( mObjects );
  result.reserve( constLayouts.size() );
  for ( const auto &layout : constLayouts )
  {
    QgsPrintLayout *_item( dynamic_cast<QgsPrintLayout *>( layout ) );
    if ( _item )
      result.push_back( _item );
  }
  return result;
}

QgsMasterLayoutInterface *QgsLayoutManager::layoutByName( const QString &name ) const
{
  return objectByName( name );
}

bool QgsLayoutManager::readXml( const QDomElement &element, const QDomDocument &doc )
{
  clear();

  QDomElement layoutsElem = element;
  if ( element.tagName() != "Layouts"_L1 )
  {
    layoutsElem = element.firstChildElement( u"Layouts"_s );
  }
  if ( layoutsElem.isNull() )
  {
    // handle legacy projects
    layoutsElem = doc.documentElement();
  }

  //restore each composer
  bool result = true;
  QDomNodeList composerNodes = element.elementsByTagName( u"Composer"_s );
  QgsScopedRuntimeProfile profile( tr( "Loading QGIS 2.x compositions" ), u"projectload"_s );
  for ( int i = 0; i < composerNodes.size(); ++i )
  {
    // This legacy title is the Composer "title" (that can be overridden by the Composition "name")
    QString legacyTitle = composerNodes.at( i ).toElement().attribute( u"title"_s );
    // Convert compositions to layouts
    QDomNodeList compositionNodes = composerNodes.at( i ).toElement().elementsByTagName( u"Composition"_s );
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
          for ( QgsMasterLayoutInterface *layout : std::as_const( mObjects ) )
          {
            if ( l->name() == layout->name() )
            {
              isDuplicateName = true;
              break;
            }
          }
          if ( isDuplicateName )
          {
            l->setName( u"%1 %2"_s.arg( originalName ).arg( id ) );
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

  profile.switchTask( tr( "Creating layouts" ) );

  // restore layouts
  const QDomNodeList layoutNodes = layoutsElem.childNodes();
  for ( int i = 0; i < layoutNodes.size(); ++i )
  {
    if ( layoutNodes.at( i ).nodeName() != "Layout"_L1 )
      continue;

    const QString layoutName = layoutNodes.at( i ).toElement().attribute( u"name"_s );
    QgsScopedRuntimeProfile profile( layoutName, u"projectload"_s );

    auto l = std::make_unique< QgsPrintLayout >( mProject );
    l->undoStack()->blockCommands( true );
    if ( !l->readLayoutXml( layoutNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    l->undoStack()->blockCommands( false );
    if ( !addLayout( l.release() ) )
    {
      result = false;
    }
  }
  //reports
  profile.switchTask( tr( "Creating reports" ) );
  const QDomNodeList reportNodes = element.elementsByTagName( u"Report"_s );
  for ( int i = 0; i < reportNodes.size(); ++i )
  {
    const QString layoutName = reportNodes.at( i ).toElement().attribute( u"name"_s );
    QgsScopedRuntimeProfile profile( layoutName, u"projectload"_s );

    auto r = std::make_unique< QgsReport >( mProject );
    if ( !r->readLayoutXml( reportNodes.at( i ).toElement(), doc, context ) )
    {
      result = false;
      continue;
    }
    if ( !addLayout( r.release() ) )
    {
      result = false;
    }
  }
  return result;
}

QDomElement QgsLayoutManager::writeXml( QDomDocument &doc ) const
{
  QDomElement layoutsElem = doc.createElement( u"Layouts"_s );

  QgsReadWriteContext context;
  context.setPathResolver( mProject->pathResolver() );
  for ( QgsMasterLayoutInterface *l : mObjects )
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
    // cppcheck-suppress returnDanglingLifetime
    return l;
  }
}

QString QgsLayoutManager::generateUniqueTitle( QgsMasterLayoutInterface::Type type ) const
{
  QStringList names;
  names.reserve( mObjects.size() );
  for ( QgsMasterLayoutInterface *l : mObjects )
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

bool QgsLayoutManager::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mObjects.empty() )
    return true;

  // NOTE: if visitEnter returns false it means "don't visit the layouts", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layouts, u"layouts"_s, tr( "Layouts" ) ) ) )
    return true;

  for ( QgsMasterLayoutInterface *l : mObjects )
  {
    if ( !l->layoutAccept( visitor ) )
      return false;
  }

  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::Layouts, u"layouts"_s, tr( "Layouts" ) ) ) )
    return false;

  return true;
}

void QgsLayoutManager::setupObjectConnections( QgsMasterLayoutInterface *layout )
{
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
}
