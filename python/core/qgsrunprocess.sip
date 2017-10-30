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
%End

%TypeHeaderCode
#include "qgsrunprocess.h"
%End
  public:

    static QgsRunProcess *create( const QString &action, bool capture ) /Factory/;
%Docstring
 :rtype: QgsRunProcess
%End

  public slots:
    void stdoutAvailable();
    void stderrAvailable();
    void processError( QProcess::ProcessError );
    void processExit( int, QProcess::ExitStatus );
    void dialogGone();

  private:
    QgsRunProcess( const QString &action, bool capture ) ;
    ~QgsRunProcess() ;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrunprocess.h                                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
