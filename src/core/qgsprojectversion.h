/***************************************************************************
                          qgsprojectfile.h  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTVERSION_H
#define QGSPROJECTVERSION_H

#include <QString>
#include <QStringList>
#include "qgis_core.h"

/**
 * \ingroup core
 * A class to describe the version of a project.
 * Used in places where you need to check if the current version
 * of QGIS is greater than the one used to create a project file.
 */
class CORE_EXPORT QgsProjectVersion
{

  public:

    /**
     * Creates a new NULL version
     */
    QgsProjectVersion() = default;

    /**
     * Constructor for QgsProjectVersion, with the specified \a major, \a minor and \a sub version numbers.
     */
    QgsProjectVersion( int major, int minor, int sub, const QString &name = QString() );

    /**
     * Constructor for QgsProjectVersion, which parses the version number from a \a string.
     */
    QgsProjectVersion( const QString &string );

    /**
     * Returns the major version number.
     */
    int majorVersion() const { return mMajor;}

    /**
     * Returns the minor version number.
     */
    int minorVersion() const { return mMinor;}

    /**
     * Returns the sub version number.
     */
    int subVersion() const { return mSub;}

    /**
     * Returns a string representation of the version.
     */
    QString text() const;

    /**
     * Returns TRUE if this is a NULL project version.
     */
    bool isNull() const;

    /**
     * Boolean equal operator
     */
    bool operator==( const QgsProjectVersion &other ) const;

    /**
     * Boolean not equal operator
     */
    bool operator!=( const QgsProjectVersion &other ) const;

    /**
     * Boolean >= operator
     */
    bool operator>=( const QgsProjectVersion &other ) const;

    /**
     * Boolean > operator
     */
    bool operator>( const QgsProjectVersion &other ) const;

  private:
    int mMajor = 0;
    int mMinor = 0;
    int mSub = 0;
    QString mName;
};

#endif // QGSPROJECTVERSION_H
