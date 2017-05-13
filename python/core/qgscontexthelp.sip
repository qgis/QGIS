/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscontexthelp.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsContextHelp : QObject
{
%Docstring
 Provides a context based help browser for a dialog.

 The help text is stored in SQLite and accessed by a context identifier
 unique to each dialog. This is a singleton class which invokes the help
 viewer using QProcess and ensures that only one viewer is open.
 The viewer will be terminated if open when the main application quits.
%End

%TypeHeaderCode
#include "qgscontexthelp.h"
%End
  public:
    static void run( const QString &context );

  private:
    QgsContextHelp() ;
  private:
    ~QgsContextHelp() ;
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscontexthelp.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
