/***************************************************************************
                         qgsmeshtimesettings.h
                         ---------------------
    begin                : March 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHTIMESETTINGS_H
#define QGSMESHTIMESETTINGS_H

#include <QDateTime>
#include <QDomDocument>

#include "qgis_core.h"
#include "qgis.h"
#include "qgsreadwritecontext.h"

/**
 * \ingroup core
 *
 * Represents a mesh time settings for mesh datasets
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsMeshTimeSettings
{
  public:
    //! Default constructor for relative time formate and 0 offset
    QgsMeshTimeSettings();
    //! Constructs relative time format settings with defined offset in hours
    QgsMeshTimeSettings( double relativeTimeOffsetHours, const QString &relativeTimeFormat );
    //! Constructs absolute time format settings with defined reference time
    QgsMeshTimeSettings( const QDateTime &absoluteTimeReferenceTime, const QString &absoluteTimeFormat );

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    //! Returns whether to use absolute time format
    bool useAbsoluteTime() const;
    //! Sets use absolute time flag
    void setUseAbsoluteTime( bool useAbsoluteTime );

    //! Returns number of offset hours for relative time formatting
    double relativeTimeOffsetHours() const;
    //! Sets number of offset hours for relative time formatting
    void setRelativeTimeOffsetHours( double relativeTimeOffsetHours );

    //! Returns format used for relative time
    QString relativeTimeFormat() const;
    //! Sets format used for relative time
    void setRelativeTimeFormat( const QString &relativeTimeFormat );

    //! Returns reference time used for absolute time format
    QDateTime absoluteTimeReferenceTime() const;
    //! Sets reference time used for absolute time format
    void setAbsoluteTimeReferenceTime( const QDateTime &absoluteTimeReferenceTime );

    //! Returns format used for absolute time
    QString absoluteTimeFormat() const;
    //! Sets format used for absolute time
    void setAbsoluteTimeFormat( const QString &absoluteTimeFormat );

  private:
    bool mUseAbsoluteTime = false;

    double mRelativeTimeOffsetHours = 0;
    QString mRelativeTimeFormat = QStringLiteral( "d hh:mm:ss" );

    QDateTime mAbsoluteTimeReferenceTime;
    QString mAbsoluteTimeFormat = QStringLiteral( "dd.MM.yyyy hh:mm:ss" );
};

Q_DECLARE_METATYPE( QgsMeshTimeSettings );

#endif // QGSMESHTIMESETTINGS_H
