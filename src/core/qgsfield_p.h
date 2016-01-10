/***************************************************************************
               qgsfield_p.h
               ------------
               Date                 : May 2015
               Copyright            : (C) 2015 by Nyall Dawson
               email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELD_PRIVATE_H
#define QGSFIELD_PRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QString>
#include <QVariant>
#include <QSharedData>
#include "qgsfield.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

class QgsFieldPrivate : public QSharedData
{
  public:

    QgsFieldPrivate( const QString& name = QString(),
                     QVariant::Type type = QVariant::Invalid,
                     const QString& typeName = QString(),
                     int len = 0,
                     int prec = 0,
                     const QString& comment = QString() )
        : name( name )
        , type( type )
        , typeName( typeName )
        , length( len )
        , precision( prec )
        , comment( comment )
    {
    }

    QgsFieldPrivate( const QgsFieldPrivate& other )
        : QSharedData( other )
        , name( other.name )
        , type( other.type )
        , typeName( other.typeName )
        , length( other.length )
        , precision( other.precision )
        , comment( other.comment )
    {
    }

    ~QgsFieldPrivate() {}

    bool operator==( const QgsFieldPrivate& other ) const
    {
      return (( name == other.name ) && ( type == other.type )
              && ( length == other.length ) && ( precision == other.precision ) );
    }

    //! Name
    QString name;

    //! Variant type
    QVariant::Type type;

    //! Type name from provider
    QString typeName;

    //! Length
    int length;

    //! Precision
    int precision;

    //! Comment
    QString comment;
};


/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

class CORE_EXPORT QgsFieldsPrivate : public QSharedData
{
  public:

    QgsFieldsPrivate()
    {
    }

    QgsFieldsPrivate( const QgsFieldsPrivate& other )
        : QSharedData( other )
        , fields( other.fields )
        , nameToIndex( other.nameToIndex )
    {
    }

    ~QgsFieldsPrivate() {}

    //! internal storage of the container
    QVector<QgsFields::Field> fields;

    //! map for quick resolution of name to index
    QHash<QString, int> nameToIndex;

};

/// @endcond

#endif
