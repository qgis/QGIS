class QgsPythonRunner
{
%TypeHeaderCode
#include <qgspythonrunner.h>
%End
  public:

    /** returns true if the runner has an instance
        (and thus is able to run commands) */
    static bool isValid();

    /** execute a python statement */
    static bool run( QString command, QString messageOnError = QString() );

    /** assign an instance of python runner so that run() can be used.
      This method should be called during app initialization.
      Takes ownership of the object, deletes previous instance. */
    static void setInstance( QgsPythonRunner* runner /Transfer/ );

  protected:
    /** protected constructor: can be instantiated only from children */
    QgsPythonRunner();
    virtual ~QgsPythonRunner();

    virtual bool runCommand( QString command, QString messageOnError = QString() ) = 0;
};
