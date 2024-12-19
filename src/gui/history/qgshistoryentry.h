/***************************************************************************
                            qgshistoryentry.h
                            --------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#ifndef QGSHISTORYENTRY_H
#define QGSHISTORYENTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QMap>
#include <QString>
#include <QDateTime>
#include <QVariant>

/**
 * Encapsulates a history entry.
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsHistoryEntry
{
  public:
    /**
     * Constructor for an invalid entry.
     */
    QgsHistoryEntry() = default;

    /**
     * Constructor for QgsHistoryEntry \a entry, with the specified \a providerId and \a timestamp.
     */
    explicit QgsHistoryEntry( const QString &providerId, const QDateTime &timestamp, const QVariantMap &entry );

    /**
     * Constructor for QgsHistoryEntry \a entry.
     *
     * The entry timestamp will be automatically set to the current date/time.
     */
    explicit QgsHistoryEntry( const QVariantMap &entry );

    /**
     * Returns TRUE if the entry is valid.
     *
     * \since QGIS 3.32
     */
    bool isValid() const;

    /**
     * Entry ID.
     *
     * \since QGIS 3.32
     */
    long long id = 0;

    //! Entry timestamp
    QDateTime timestamp;

    //! Associated history provider ID
    QString providerId;

    /**
     * Entry details.
     *
     * Entries details are stored as a free-form map. Interpretation of this map is the responsibility of the
     * associated history provider.
     */
    QVariantMap entry;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    //%MethodCode
    const QString str = QStringLiteral( "<QgsHistoryEntry: %1 %2>" ).arg( sipCpp->providerId, sipCpp->timestamp.toString( Qt::ISODate ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    //%End
#endif
};

Q_DECLARE_METATYPE( QgsHistoryEntry );

#endif // QGSHISTORYENTRY
