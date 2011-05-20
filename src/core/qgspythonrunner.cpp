#include "qgspythonrunner.h"
#include "qgslogger.h"

QgsPythonRunner* QgsPythonRunner::mInstance = NULL;

///////////////////////////
// static methods

bool QgsPythonRunner::isValid()
{
  return mInstance != NULL;
}

bool QgsPythonRunner::run( QString command, QString messageOnError )
{
  if ( mInstance )
  {
    return mInstance->runCommand( command, messageOnError );
  }
  else
  {
    QgsDebugMsg("Unable to run Python command: runner not available!");
    return false;
  }
}

void QgsPythonRunner::setInstance( QgsPythonRunner* runner )
{
  delete mInstance;
  mInstance = runner;
}

///////////////////////////
// non-static methods

QgsPythonRunner::QgsPythonRunner()
{
}

QgsPythonRunner::~QgsPythonRunner()
{

}
