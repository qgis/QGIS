/***************************************************************************
                              qgsftptransaction.h
                              -------------------
  begin                : May 16, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFtp>

/** A class for synchronous ftp access (using QFtp in background)
 *
 * @deprecated because of QFtp removal in Qt5.
 */
class QgsFtpTransaction: public QObject
{
    Q_OBJECT
  public:
    Q_DECL_DEPRECATED QgsFtpTransaction();
    ~QgsFtpTransaction();

    /** Transfers the file with the given Url and stores it into ba
       @param ftpUrl url of the file to access
       @param pointer to buffer to store file contents
       @return 0 in case of success*/
    Q_DECL_DEPRECATED int get( const QString& ftpUrl, QByteArray& ba );

  public slots:
    void setFinishedFlag( bool error );

  private:
    QFtp* mFtp;
    bool mRequestFinished;
    bool mErrorFlag;
};
