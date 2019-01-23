/***************************************************************************
    qgspythonrunner.h
    ---------------------
    begin                : May 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPYTHONRUNNER_H
#define QGSPYTHONRUNNER_H

#include <QString>
#include "qgis_sip.h"

#include "qgis_core.h"

/**
 * \ingroup core
  Utility class for running Python commands from various parts of QGIS.
  There is no direct Python support in the core library, so it is expected
  that application with Python support creates a subclass that implements
  pure virtual function(s) during the initialization. The static methods
  will then work as expected.

  Added in QGIS v?
 */
class CORE_EXPORT QgsPythonRunner
{
  public:

    /**
     * Returns true if the runner has an instance
        (and thus is able to run commands) */
    static bool isValid();

    //! Execute a Python statement
    static bool run( const QString &command, const QString &messageOnError = QString() );

    //! Eval a Python statement
    static bool eval( const QString &command, QString &result SIP_OUT );

    /**
     * Assign an instance of Python runner so that run() can be used.
      This method should be called during app initialization.
      Takes ownership of the object, deletes previous instance. */
    static void setInstance( QgsPythonRunner *runner SIP_TRANSFER );

  protected:
    //! Protected constructor: can be instantiated only from children
    QgsPythonRunner() = default;
    virtual ~QgsPythonRunner() = default;

    virtual bool runCommand( QString command, QString messageOnError = QString() ) = 0;

    virtual bool evalCommand( QString command, QString &result ) = 0;

    static QgsPythonRunner *sInstance;
};

#endif // QGSPYTHONRUNNER_H
