/***************************************************************************
                                  qgsdatetimeutilsils.h
                              ---------------------
    begin                : October 2026
    copyright            : (C) 2026 by Vincent Cloarec
    email                : vcoarec@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMEUTILSUTILS_H
#define QGSDATETIMEUTILSUTILS_H

#include "qgis_core.h"
#include "qgsreadwritecontext.h"

#include <QDomElement>
#include <QStringList>

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class QgsDateTimeUtils
 * \brief Utilities for handling date/time.
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsDateTimeUtils
{
  public:
    /**
     * Encodes a \a datetime to a new DOM element.
     * \param dateTime datetime to encode
     * \param doc DOM document
     * \param tagName name of the DOM element
     * \param context read/write context used for encoding
     * \returns element containing encoded datetime
     */
    static QDomElement writeDateTimeToXml( const QDateTime &dateTime, const QString &tagName, QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Decodes a DOM element to a datetime.
     * \param element DOM element to decode
     * \param context read/write context used for decoding
     * \returns decoded datetime
     */
    static QDateTime readDateTimeToXml( const QDomElement &element, const QgsReadWriteContext &context );
};

#endif //QGSDATETIMEUTILSUTILS_H
