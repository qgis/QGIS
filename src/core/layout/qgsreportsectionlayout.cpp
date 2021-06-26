/***************************************************************************
                             qgsreportsectionlayout.cpp
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

#include "qgsreportsectionlayout.h"
#include "qgslayout.h"

///@cond NOT_STABLE

QgsReportSectionLayout::QgsReportSectionLayout( QgsAbstractReportSection *parent )
  : QgsAbstractReportSection( parent )
{}

QIcon QgsReportSectionLayout::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayout.svg" ) );
}

QgsReportSectionLayout *QgsReportSectionLayout::clone() const
{
  std::unique_ptr< QgsReportSectionLayout > copy = qgis::make_unique< QgsReportSectionLayout >( nullptr );
  copyCommonProperties( copy.get() );

  if ( mBody )
  {
    copy->mBody.reset( mBody->clone() );
  }
  else
    copy->mBody.reset();

  copy->mBodyEnabled = mBodyEnabled;

  return copy.release();
}

bool QgsReportSectionLayout::beginRender()
{
  mExportedBody = false;
  return QgsAbstractReportSection::beginRender();
}

QgsLayout *QgsReportSectionLayout::nextBody( bool &ok )
{
  if ( !mExportedBody && mBody && mBodyEnabled )
  {
    mExportedBody = true;
    ok = true;
    return mBody.get();
  }
  else
  {
    ok = false;
    return nullptr;
  }
}

void QgsReportSectionLayout::reloadSettings()
{
  QgsAbstractReportSection::reloadSettings();
  if ( mBody )
    mBody->reloadSettings();
}

bool QgsReportSectionLayout::writePropertiesToElement( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  if ( mBody )
  {
    QDomElement bodyElement = doc.createElement( QStringLiteral( "body" ) );
    bodyElement.appendChild( mBody->writeXml( doc, context ) );
    element.appendChild( bodyElement );
  }
  element.setAttribute( QStringLiteral( "bodyEnabled" ), mBodyEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  return true;
}

bool QgsReportSectionLayout::readPropertiesFromElement( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  const QDomElement bodyElement = element.firstChildElement( QStringLiteral( "body" ) );
  if ( !bodyElement.isNull() )
  {
    const QDomElement bodyLayoutElem = bodyElement.firstChild().toElement();
    std::unique_ptr< QgsLayout > body = qgis::make_unique< QgsLayout >( project() );
    body->readXml( bodyLayoutElem, doc, context );
    mBody = std::move( body );
  }
  mBodyEnabled = element.attribute( QStringLiteral( "bodyEnabled" ) ).toInt();
  return true;
}

///@endcond

