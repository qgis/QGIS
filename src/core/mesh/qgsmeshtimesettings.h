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
 * \brief Represents a mesh time settings for mesh datasets
 *
 * \since QGIS 3.8
 */
class CORE_EXPORT QgsMeshTimeSettings
{
  public:

    /**
     *  Time units used to display time
     *  \since QGIS 3.12
    */
    enum TimeUnit
    {
      //! second unit
      seconds = 0,
      //! minute unit
      minutes,
      //! hour unit
      hours,
      //! day unit
      days
    };

    QgsMeshTimeSettings();

    //! Writes configuration to a new DOM element
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Reads configuration from the given DOM element
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    //! Returns format used for relative time
    QString relativeTimeFormat() const;
    //! Sets format used for relative time
    void setRelativeTimeFormat( const QString &relativeTimeFormat );

    //! Returns format used for absolute time
    QString absoluteTimeFormat() const;
    //! Sets format used for absolute time
    void setAbsoluteTimeFormat( const QString &absoluteTimeFormat );

  private:

    QString mRelativeTimeFormat = QStringLiteral( "d hh:mm:ss" );
    QString mAbsoluteTimeFormat = QStringLiteral( "dd.MM.yyyy hh:mm:ss" );
};

Q_DECLARE_METATYPE( QgsMeshTimeSettings );

#endif // QGSMESHTIMESETTINGS_H
