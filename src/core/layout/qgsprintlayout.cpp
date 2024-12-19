/***************************************************************************
                              qgsprintlayout.cpp
                             -------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"
#include "qgsreadwritecontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsstyleentityvisitor.h"

QgsPrintLayout::QgsPrintLayout( QgsProject *project )
  : QgsLayout( project )
  , mAtlas( new QgsLayoutAtlas( this ) )
{
}

QgsPrintLayout *QgsPrintLayout::clone() const
{
  QDomDocument currentDoc;

  const QgsReadWriteContext context;
  const QDomElement elem = writeXml( currentDoc, context );
  currentDoc.appendChild( elem );

  std::unique_ptr< QgsPrintLayout > newLayout = std::make_unique< QgsPrintLayout >( project() );
  bool ok = false;
  newLayout->loadFromTemplate( currentDoc, context, true, &ok );
  if ( !ok )
  {
    return nullptr;
  }

  return newLayout.release();
}

QgsProject *QgsPrintLayout::layoutProject() const
{
  return project();
}

QIcon QgsPrintLayout::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconLayout.svg" ) );
}

QgsLayoutAtlas *QgsPrintLayout::atlas()
{
  return mAtlas;
}

void QgsPrintLayout::setName( const QString &name )
{
  mName = name;
  emit nameChanged( name );
  layoutProject()->setDirty( true );
}

QDomElement QgsPrintLayout::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement layoutElem = QgsLayout::writeXml( document, context );
  layoutElem.setAttribute( QStringLiteral( "name" ), mName );
  mAtlas->writeXml( layoutElem, document, context );
  return layoutElem;
}

bool QgsPrintLayout::readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  if ( !QgsLayout::readXml( layoutElement, document, context ) )
    return false;

  const QDomElement atlasElem = layoutElement.firstChildElement( QStringLiteral( "Atlas" ) );
  mAtlas->readXml( atlasElem, document, context );

  setName( layoutElement.attribute( QStringLiteral( "name" ) ) );

  return true;
}

QDomElement QgsPrintLayout::writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  return writeXml( document, context );
}

bool QgsPrintLayout::readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  return readXml( layoutElement, document, context );
}

QgsExpressionContext QgsPrintLayout::createExpressionContext() const
{
  QgsExpressionContext context = QgsLayout::createExpressionContext();

  if ( mAtlas->enabled() )
  {
    context.appendScope( QgsExpressionContextUtils::atlasScope( mAtlas ) );
  }

  return context;
}

void QgsPrintLayout::updateSettings()
{
  reloadSettings();
}

bool QgsPrintLayout::layoutAccept( QgsStyleEntityVisitorInterface *visitor ) const
{
  // NOTE: if visitEnter returns false it means "don't visit the layout", not "abort all further visitations"
  if ( !visitor->visitEnter( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::PrintLayout, QStringLiteral( "layout" ), mName ) ) )
    return true;

  if ( !accept( visitor ) )
    return false;
  if ( !visitor->visitExit( QgsStyleEntityVisitorInterface::Node( QgsStyleEntityVisitorInterface::NodeType::PrintLayout, QStringLiteral( "layout" ), mName ) ) )
    return false;
  return true;
}

QgsMasterLayoutInterface::Type QgsPrintLayout::layoutType() const
{
  return QgsMasterLayoutInterface::PrintLayout;
}
