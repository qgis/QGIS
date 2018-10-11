/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrunprocess.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRunProcess: QObject /NoDefaultCtors/
{
%Docstring
A class that executes an external program/script.
It can optionally capture the standard output and error from the
process and displays them in a dialog box.

On some platforms (e.g. iOS) , the process execution is skipped
https://lists.qt-project.org/pipermail/development/2015-July/022205.html
%End

%TypeHeaderCode
#include "qgsrunprocess.h"
%End
  public:

    static QgsRunProcess *create( const QString &action, bool capture ) /Factory/;

  private:
    QgsRunProcess( const QString &action, bool capture );
    ~QgsRunProcess();
  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrunprocess.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
