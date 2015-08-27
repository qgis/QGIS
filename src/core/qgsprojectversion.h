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

/** \ingroup core
 * A class to describe the version of a project.
 * Used in places where you need to check if the current version
 * of QGIS is greater than the one used to create a project file.
 */

class CORE_EXPORT QgsProjectVersion
{

  public:

    QgsProjectVersion() : mMajor( 0 ), mMinor( 0 ), mSub( 0 ) {}
    ~QgsProjectVersion() {}
    QgsProjectVersion( int major, int minor, int sub, QString name = "" );
    QgsProjectVersion( QString string );
    int majorVersion() { return mMajor;}
    int minorVersion() { return mMinor;}
    int subVersion()   { return mSub;}
    QString text();

    /** Boolean equal operator
    */
    bool operator==( const QgsProjectVersion &other );

    /** Boolean >= operator
    */
    bool operator>=( const QgsProjectVersion &other );

    /** Boolean > operator
    */
    bool operator>( const QgsProjectVersion &other );

  private:
    int mMajor;
    int mMinor;
    int mSub;
    QString mName;
};

#endif // QGSPROJECTVERSION_H
