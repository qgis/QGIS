/***************************************************************************
                        qgsserverstreamingdevice.h
  -------------------------------------------------------------------
Date                 : 25 May 2015
Copyright            : (C) 2015 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERSTREAMINGDEVICE_H
#define QGSSERVERSTREAMINGDEVICE_H

#include <QIODevice>

class QgsRequestHandler;

class QgsServerStreamingDevice: public QIODevice
{
    Q_OBJECT

  public:
    QgsServerStreamingDevice( const QString& formatName, QgsRequestHandler* rh, QObject* parent = nullptr );
    ~QgsServerStreamingDevice();

    bool isSequential() const override { return false; }

    bool open( OpenMode mode ) override;
    void close() override;

  protected:
    QString mFormatName;
    QgsRequestHandler* mRequestHandler;

    QgsServerStreamingDevice(); //default constructor forbidden

    qint64 writeData( const char * data, qint64 maxSize ) override;
    qint64 readData( char * data, qint64 maxSize ) override;
};

#endif // QGSSERVERSTREAMINGDEVICE_H
