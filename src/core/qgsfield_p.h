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

#define SIP_NO_FILE

#include "qgsfieldconstraints.h"
#include "qgseditorwidgetsetup.h"
#include "qgsdefaultvalue.h"
#include <QString>
#include <QVariant>
#include <QSharedData>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

class QgsFieldPrivate : public QSharedData
{
  public:

    QgsFieldPrivate( const QString &name = QString(),
                     QVariant::Type type = QVariant::Invalid,
                     QVariant::Type subType = QVariant::Invalid,
                     const QString &typeName = QString(),
                     int len = 0,
                     int prec = 0,
                     const QString &comment = QString() )
      : name( name )
      , type( type )
      , subType( subType )
      , typeName( typeName )
      , length( len )
      , precision( prec )
      , comment( comment )
    {
    }

    QgsFieldPrivate( const QgsFieldPrivate &other )
      : QSharedData( other )
      , name( other.name )
      , type( other.type )
      , subType( other.subType )
      , typeName( other.typeName )
      , length( other.length )
      , precision( other.precision )
      , comment( other.comment )
      , alias( other.alias )
      , defaultValueDefinition( other.defaultValueDefinition )
      , constraints( other.constraints )
    {
    }

    ~QgsFieldPrivate() = default;

    bool operator==( const QgsFieldPrivate &other ) const
    {
      return ( ( name == other.name ) && ( type == other.type ) && ( subType == other.subType )
               && ( length == other.length ) && ( precision == other.precision )
               && ( alias == other.alias ) && ( defaultValueDefinition == other.defaultValueDefinition )
               && ( constraints == other.constraints ) );
    }

    //! Name
    QString name;

    //! Variant type
    QVariant::Type type;

    //! If the variant is a collection, its element's type
    QVariant::Type subType;

    //! Type name from provider
    QString typeName;

    //! Length
    int length;

    //! Precision
    int precision;

    //! Comment
    QString comment;

    //! Alias for field name (friendly name shown to users)
    QString alias;

    //! Default value
    QgsDefaultValue defaultValueDefinition;

    //! Field constraints
    QgsFieldConstraints constraints;

    QgsEditorWidgetSetup editorWidgetSetup;
};

/// @endcond

#endif
