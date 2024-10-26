/***************************************************************************
                             qgsreport.cpp
                             --------------------
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

#include "qgsreport.h"
#include "moc_qgsreport.cpp"
#include "qgslayout.h"

///@cond NOT_STABLE

QgsReport::QgsReport( QgsProject *project )
  : QgsAbstractReportSection( nullptr )
  , mProject( project )
{}

QIcon QgsReport::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconReport.svg" ) );
}

QgsReport *QgsReport::clone() const
{
  std::unique_ptr< QgsReport > copy = std::make_unique< QgsReport >( mProject );
  copyCommonProperties( copy.get() );
  return copy.release();
}

void QgsReport::setName( const QString &name )
{
  mName = name;
  emit nameChanged( mName );
}

QDomElement QgsReport::writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Report" ) );
  writeXml( element, document, context );
  element.setAttribute( QStringLiteral( "name" ), mName );
  return element;
}

bool QgsReport::readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context )
{
  const QDomNodeList sectionList = layoutElement.elementsByTagName( QStringLiteral( "Section" ) );
  if ( sectionList.count() > 0 )
  {
    readXml( sectionList.at( 0 ).toElement(), document, context );
  }
  setName( layoutElement.attribute( QStringLiteral( "name" ) ) );
  return true;
}

void QgsReport::updateSettings()
{
  reloadSettings();
}

bool QgsReport::layoutAccept( QgsStyleEntityVisitorInterface *visitor ) const
{
  return accept( visitor );
}

QgsMasterLayoutInterface::Type QgsReport::layoutType() const
{
  return QgsMasterLayoutInterface::Report;
}

///@endcond
