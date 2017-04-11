/***************************************************************************
                             qgslayermetadata.cpp
                             --------------------
    begin                : April 2017
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

#include "qgslayermetadata.h"

QString QgsLayerMetadata::identifier() const
{
  return mIdentifier;
}

void QgsLayerMetadata::setIdentifier( const QString &identifier )
{
  mIdentifier = identifier;
}

QString QgsLayerMetadata::parentIdentifier() const
{
  return mParentIdentifier;
}

void QgsLayerMetadata::setParentIdentifier( const QString &parentIdentifier )
{
  mParentIdentifier = parentIdentifier;
}

QString QgsLayerMetadata::type() const
{
  return mType;
}

void QgsLayerMetadata::setType( const QString &type )
{
  mType = type;
}

QString QgsLayerMetadata::title() const
{
  return mTitle;
}

void QgsLayerMetadata::setTitle( const QString &title )
{
  mTitle = title;
}

QString QgsLayerMetadata::abstract() const
{
  return mAbstract;
}

void QgsLayerMetadata::setAbstract( const QString &abstract )
{
  mAbstract = abstract;
}

QString QgsLayerMetadata::fees() const
{
  return mFees;
}

void QgsLayerMetadata::setFees( const QString &fees )
{
  mFees = fees;
}

QList<QgsLayerMetadata::Constraint> QgsLayerMetadata::constraints() const
{
  return mConstraints;
}

void QgsLayerMetadata::setConstraints( const QList<Constraint> &constraints )
{
  mConstraints = constraints;
}

QStringList QgsLayerMetadata::rights() const
{
  return mRights;
}

void QgsLayerMetadata::setRights( const QStringList &rights )
{
  mRights = rights;
}

QString QgsLayerMetadata::encoding() const
{
  return mEncoding;
}

void QgsLayerMetadata::setEncoding( const QString &encoding )
{
  mEncoding = encoding;
}

QgsCoordinateReferenceSystem QgsLayerMetadata::crs() const
{
  return mCrs;
}

void QgsLayerMetadata::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

QMap<QString, QStringList> QgsLayerMetadata::keywords() const
{
  return mKeywords;
}

void QgsLayerMetadata::setKeywords( const QMap<QString, QStringList> &keywords )
{
  mKeywords = keywords;
}

void QgsLayerMetadata::addKeywords( const QString &vocabulary, const QStringList &keywords )
{
  mKeywords.insert( vocabulary, keywords );
}

QStringList QgsLayerMetadata::keywordVocabularies() const
{
  return mKeywords.keys();
}

QStringList QgsLayerMetadata::keywords( const QString &vocabulary ) const
{
  return mKeywords.value( vocabulary );
}

QList<QgsLayerMetadata::Contact> QgsLayerMetadata::contacts() const
{
  return mContacts;
}

void QgsLayerMetadata::setContacts( const QList<Contact> &contacts )
{
  mContacts = contacts;
}

void QgsLayerMetadata::addContact( const QgsLayerMetadata::Contact &contact )
{
  mContacts << contact;
}

QList<QgsLayerMetadata::Link> QgsLayerMetadata::links() const
{
  return mLinks;
}

void QgsLayerMetadata::setLinks( const QList<QgsLayerMetadata::Link> &links )
{
  mLinks = links;
}

void QgsLayerMetadata::addLink( const QgsLayerMetadata::Link &link )
{
  mLinks << link;
}

QString QgsLayerMetadata::language() const
{
  return mLanguage;
}

void QgsLayerMetadata::setLanguage( const QString &language )
{
  mLanguage = language;
}
