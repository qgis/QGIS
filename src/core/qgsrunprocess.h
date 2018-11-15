/***************************************************************************
                          qgsrunprocess.h

 A class that runs an external program

                             -------------------
    begin                : Jan 2005
    copyright            : (C) 2005 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRUNPROCESS_H
#define QGSRUNPROCESS_H

#include <QObject>
#include <QProcess>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsMessageOutput;

/**
 * \ingroup core
 * A class that executes an external program/script.
 * It can optionally capture the standard output and error from the
 * process and displays them in a dialog box.
 *
 * On some platforms (e.g. iOS) , the process execution is skipped
 * https://lists.qt-project.org/pipermail/development/2015-July/022205.html
 */
class CORE_EXPORT QgsRunProcess: public QObject SIP_NODEFAULTCTORS
{
    Q_OBJECT

  public:
    // This class deletes itself, so to ensure that it is only created
    // using new, the Named Consturctor Idiom is used, and one needs to
    // use the create() static function to get an instance of this class.

    // The action argument contains string with the command.
    // If capture is true, the standard output and error from the process
    // will be sent to QgsMessageOutput - usually a dialog box.
    static QgsRunProcess *create( const QString &action, bool capture ) SIP_FACTORY
    { return new QgsRunProcess( action, capture ); }

  private:
    QgsRunProcess( const QString &action, bool capture ) SIP_FORCE;
    ~QgsRunProcess() override SIP_FORCE;

#if QT_CONFIG(process)
    // Deletes the instance of the class
    void die();

    QProcess *mProcess = nullptr;
    QgsMessageOutput *mOutput = nullptr;
    QString mCommand;

  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();
#endif // !(QT_CONFIG(process)
};

#endif
