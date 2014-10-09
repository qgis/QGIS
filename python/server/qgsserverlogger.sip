/***************************************************************************
                              qgsserverlogger.h
                              -----------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/**Writes message log into server logfile*/
class QgsServerLogger: public QObject
{
%TypeHeaderCode
    #include "qgsserverlogger.h"
%End

  public:
    static QgsServerLogger* instance();

    int logLevel();

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );

};
