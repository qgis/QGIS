/***************************************************************************
    qgspostgreslistener.h  -  Listen to postgres NOTIFY
                             -------------------
    begin                : Sept 11, 2017
    copyright            : (C) 2017 by Vincent Mora
    email                : vincent dor mora at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESLISTENER_H
#define QGSPOSTGRESLISTENER_H

#include <memory>

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class QgsPostgresConn;


/**
 * \class QgsPostgresListener
 * \brief Launch a thread to listen on postgres notifications on the "qgis" channel, the notify signal is emitted on postgres notify.
 *
 */

class QgsPostgresListener : public QThread
{
    Q_OBJECT

  public:

    /**
     * create an instance if possible and starts the associated thread
     * /returns NULLPTR on error
     */
    static std::unique_ptr< QgsPostgresListener > create( const QString &connString );

    ~QgsPostgresListener() override;

    void run() override;

  signals:
    void notify( QString message );

  private:
    volatile bool mStop = false;

    QgsPostgresConn *mConn = nullptr;

    QgsPostgresListener( const QString &connString );

    Q_DISABLE_COPY( QgsPostgresListener )

};

#endif // QGSPOSTGRESLISTENER_H
