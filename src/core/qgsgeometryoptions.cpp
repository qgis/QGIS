/***************************************************************************
                          qgsgeometryoptions.cpp
                             -------------------
    begin                : Aug 23, 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometryoptions.h"

#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsxmlutils.h"

#include "moc_qgsgeometryoptions.cpp"

const QgsSettingsEntryString *QgsGeometryOptions::settingsGeometryValidationDefaultChecks = new QgsSettingsEntryString( u"default_checks"_s, QgsSettingsTree::sTreeGeometryValidation, QString() );

QgsGeometryOptions::QgsGeometryOptions()
{
  mGeometryChecks = settingsGeometryValidationDefaultChecks->value().split( ',' ) ;
}

bool QgsGeometryOptions::removeDuplicateNodes() const
{
  return mRemoveDuplicateNodes;
}

void QgsGeometryOptions::setRemoveDuplicateNodes( bool value )
{
  mRemoveDuplicateNodes = value;
  emit removeDuplicateNodesChanged();
}

double QgsGeometryOptions::geometryPrecision() const
{
  return mGeometryPrecision;
}

void QgsGeometryOptions::setGeometryPrecision( double value )
{
  mGeometryPrecision = value;
  emit geometryPrecisionChanged();
}

bool QgsGeometryOptions::isActive() const
{
  return mGeometryPrecision != 0.0 || mRemoveDuplicateNodes;
}

void QgsGeometryOptions::apply( QgsGeometry &geometry ) const
{
  if ( mGeometryPrecision != 0.0 )
    geometry = geometry.snappedToGrid( mGeometryPrecision, mGeometryPrecision );

  if ( mRemoveDuplicateNodes )
    geometry.removeDuplicateNodes( 4 * std::numeric_limits<double>::epsilon(), true );
}

QStringList QgsGeometryOptions::geometryChecks() const
{
  return mGeometryChecks;
}

void QgsGeometryOptions::setGeometryChecks( const QStringList &geometryChecks )
{
  mGeometryChecks = geometryChecks;
  emit geometryChecksChanged();
}

QVariantMap QgsGeometryOptions::checkConfiguration( const QString &checkId ) const
{
  return mCheckConfiguration.value( checkId ).toMap();
}

void QgsGeometryOptions::setCheckConfiguration( const QString &checkId, const QVariantMap &checkConfiguration )
{
  mCheckConfiguration[checkId] = checkConfiguration;
  emit checkConfigurationChanged();
}

void QgsGeometryOptions::writeXml( QDomNode &node ) const
{
  QDomDocument doc = node.ownerDocument();
  QDomElement geometryOptionsElement = doc.createElement( u"geometryOptions"_s );
  node.appendChild( geometryOptionsElement );

  geometryOptionsElement.setAttribute( u"removeDuplicateNodes"_s, mRemoveDuplicateNodes ? 1 : 0 );
  geometryOptionsElement.setAttribute( u"geometryPrecision"_s, mGeometryPrecision );

  QDomElement activeCheckListElement = QgsXmlUtils::writeVariant( mGeometryChecks, doc );
  activeCheckListElement.setTagName( u"activeChecks"_s );
  geometryOptionsElement.appendChild( activeCheckListElement );
  QDomElement checkConfigurationElement = QgsXmlUtils::writeVariant( mCheckConfiguration, doc );
  checkConfigurationElement.setTagName( u"checkConfiguration"_s );
  geometryOptionsElement.appendChild( checkConfigurationElement );
}

void QgsGeometryOptions::readXml( const QDomNode &node )
{
  const QDomElement geometryOptionsElement = node.toElement();
  setGeometryPrecision( geometryOptionsElement.attribute( u"geometryPrecision"_s,  u"0.0"_s ).toDouble() );
  setRemoveDuplicateNodes( geometryOptionsElement.attribute( u"removeDuplicateNodes"_s,  u"0"_s ).toInt() == 1 );

  const QDomElement activeChecksElem = node.namedItem( u"activeChecks"_s ).toElement();
  const QVariant activeChecks = QgsXmlUtils::readVariant( activeChecksElem );
  setGeometryChecks( activeChecks.toStringList() );

  const QDomElement checkConfigurationElem = node.namedItem( u"checkConfiguration"_s ).toElement();
  const QVariant checkConfiguration = QgsXmlUtils::readVariant( checkConfigurationElem );
  mCheckConfiguration = checkConfiguration.toMap();
}
