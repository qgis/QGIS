/***************************************************************************
                          qgsnmeaconnection.h  -  description
                          -------------------
    begin                : November 30th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNMEACONNECTION_H
#define QGSNMEACONNECTION_H

#include "qgis_core.h"
#include "qgsgpsconnection.h"

/**
 * \ingroup core
 * Evaluates NMEA sentences coming from a GPS device
 * \since QGIS 1.4
 */
class CORE_EXPORT QgsNmeaConnection: public QgsGpsConnection
{
    Q_OBJECT
  public:

    /**
     * \brief Constructs a QgsNmeaConnection with given \a device.
     */
    QgsNmeaConnection( QIODevice *device );

  protected slots:
    //! Parse available data source content
    void parseData() override;

  protected:
    //! Store data from the device before it is processed
    QString mStringBuffer;
    //! Splits mStringBuffer into sentences and calls libnmea
    void processStringBuffer();
    //handle the different sentence type
    //! process GGA sentence
    void processGgaSentence( const char *data, int len );
    //! process RMC sentence
    void processRmcSentence( const char *data, int len );
    //! process GSV sentence
    void processGsvSentence( const char *data, int len );
    //! process VTG sentence
    void processVtgSentence( const char *data, int len );
    //! process GSA sentence
    void processGsaSentence( const char *data, int len );
};

#endif // QGSNMEACONNECTION_H
