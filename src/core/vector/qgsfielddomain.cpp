/***************************************************************************
    qgsfielddomain.h
    ------------------
    Date                 : January 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfielddomain.h"
#include <memory>

//
// QgsFieldDomain
//

QgsFieldDomain::QgsFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType )
  : mName( name )
  , mDescription( description )
  , mFieldType( fieldType )
{

}

QgsFieldDomain::~QgsFieldDomain() = default;

//
// QgsCodedValue
//
bool QgsCodedValue::operator==( const QgsCodedValue &other ) const
{
  return other.mCode == mCode && other.mValue == mValue;
}

bool QgsCodedValue::operator!=( const QgsCodedValue &other ) const
{
  return !( *this == other );
}

//
// QgsCodedFieldDomain
//

QgsCodedFieldDomain::QgsCodedFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType, const QList<QgsCodedValue> &values )
  : QgsFieldDomain( name, description, fieldType )
  , mValues( values )
{

}

Qgis::FieldDomainType QgsCodedFieldDomain::type() const
{
  return Qgis::FieldDomainType::Coded;
}

QString QgsCodedFieldDomain::typeName() const
{
  return QObject::tr( "Coded Values" );
}

QgsCodedFieldDomain *QgsCodedFieldDomain::clone() const
{
  std::unique_ptr< QgsCodedFieldDomain > res = std::make_unique< QgsCodedFieldDomain >( mName, mDescription, mFieldType, mValues );
  res->mSplitPolicy = mSplitPolicy;
  res->mMergePolicy = mMergePolicy;
  return res.release();
}

//
// QgsRangeFieldDomain
//

QgsRangeFieldDomain::QgsRangeFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType, const QVariant &minimum, bool minimumIsInclusive, const QVariant &maximum, bool maximumIsInclusive )
  : QgsFieldDomain( name, description, fieldType )
  , mMin( minimum )
  , mMax( maximum )
  , mMinIsInclusive( minimumIsInclusive )
  , mMaxIsInclusive( maximumIsInclusive )
{

}

Qgis::FieldDomainType QgsRangeFieldDomain::type() const
{
  return Qgis::FieldDomainType::Range;
}

QString QgsRangeFieldDomain::typeName() const
{
  return QObject::tr( "Range" );
}

QgsRangeFieldDomain *QgsRangeFieldDomain::clone() const
{
  std::unique_ptr< QgsRangeFieldDomain > res = std::make_unique< QgsRangeFieldDomain >( mName, mDescription, mFieldType, mMin, mMinIsInclusive, mMax, mMaxIsInclusive );
  res->mSplitPolicy = mSplitPolicy;
  res->mMergePolicy = mMergePolicy;
  return res.release();
}


//
// QgsGlobFieldDomain
//

QgsGlobFieldDomain::QgsGlobFieldDomain( const QString &name, const QString &description, QVariant::Type fieldType, const QString &glob )
  : QgsFieldDomain( name, description, fieldType )
  , mGlob( glob )
{

}

Qgis::FieldDomainType QgsGlobFieldDomain::type() const
{
  return Qgis::FieldDomainType::Glob;
}

QString QgsGlobFieldDomain::typeName() const
{
  return QObject::tr( "Glob" );
}

QgsGlobFieldDomain *QgsGlobFieldDomain::clone() const
{
  std::unique_ptr< QgsGlobFieldDomain > res = std::make_unique< QgsGlobFieldDomain >( mName, mDescription, mFieldType, mGlob );
  res->mSplitPolicy = mSplitPolicy;
  res->mMergePolicy = mMergePolicy;
  return res.release();
}
