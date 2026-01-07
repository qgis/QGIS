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
  return QgsApplication::getThemeIcon( u"/mIconLayout.svg"_s );
}

QgsReportSectionLayout *QgsReportSectionLayout::clone() const
{
  auto copy = std::make_unique< QgsReportSectionLayout >( nullptr );
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
    QDomElement bodyElement = doc.createElement( u"body"_s );
    bodyElement.appendChild( mBody->writeXml( doc, context ) );
    element.appendChild( bodyElement );
  }
  element.setAttribute( u"bodyEnabled"_s, mBodyEnabled ? u"1"_s : u"0"_s );
  return true;
}

bool QgsReportSectionLayout::readPropertiesFromElement( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  const QDomElement bodyElement = element.firstChildElement( u"body"_s );
  if ( !bodyElement.isNull() )
  {
    const QDomElement bodyLayoutElem = bodyElement.firstChild().toElement();
    auto body = std::make_unique< QgsLayout >( project() );
    body->readXml( bodyLayoutElem, doc, context );
    mBody = std::move( body );
  }
  mBodyEnabled = element.attribute( u"bodyEnabled"_s ).toInt();
  return true;
}

///@endcond

