#ifndef QGSPYTHONRUNNER_H
#define QGSPYTHONRUNNER_H

#include <QString>

/**
  Utility class for running python commands from various parts of QGIS.
  There is no direct python support in the core library, so it is expected
  that application with python support creates a subclass that implements
  pure virtual function(s) during the initialization. The static methods
  will then work as expected.

  Added in QGIS v?
 */
class CORE_EXPORT QgsPythonRunner
{
public:

    /** returns true if the runner has an instance
        (and thus is able to run commands) */
    static bool isValid();

    /** execute a python statement */
    static bool run( QString command, QString messageOnError = QString() );

    /** assign an instance of python runner so that run() can be used.
      This method should be called during app initialization.
      Takes ownership of the object, deletes previous instance. */
    static void setInstance( QgsPythonRunner* runner );

protected:
    /** protected constructor: can be instantiated only from children */
    QgsPythonRunner();
    virtual ~QgsPythonRunner();

    virtual bool runCommand( QString command, QString messageOnError = QString() ) = 0;

    static QgsPythonRunner* mInstance;
};

#endif // QGSPYTHONRUNNER_H
