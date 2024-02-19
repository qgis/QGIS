/***************************************************************************
                          qgsexception.h
                             -------------------
  begin                : August 31, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXCEPTION_H
#define QGSEXCEPTION_H

#define SIP_NO_CREATION

#define SIP_NO_FILE

#include <QString>

#include "qgis_core.h"



/**
 * \ingroup core
 * \brief Defines a QGIS exception class.
 */
class CORE_EXPORT QgsException
{
  public:

    /**
     * Constructor for QgsException, with the specified error \a message.
     */
    QgsException( const QString &message )
      : mWhat( message )
    {}

    // cppcheck-suppress internalAstError
    virtual ~QgsException() throw() = default;

    //! \note not available in Python bindings
    QString what() const throw()
    {
      return mWhat;
    }

  private:

    //! Description of exception
    QString mWhat;

};


/**
 * \ingroup core
 * \brief Custom exception class for Coordinate Reference System related exceptions.
 */
class CORE_EXPORT QgsCsException : public QgsException
{
  public:

    /**
     * Constructor for QgsCsException, with the specified error \a message.
     */
    QgsCsException( const QString &message ) : QgsException( message ) {}

};

/**
 * \class QgsProcessingException
 * \ingroup core
 * \brief Custom exception class for processing related exceptions.
 */
class CORE_EXPORT QgsProcessingException : public QgsException
{
  public:

    /**
     * Constructor for QgsProcessingException, with the specified error \a message.
     */
    QgsProcessingException( const QString &message ) : QgsException( message ) {}

};


/**
 * \class QgsProviderConnectionException
 * \ingroup core
 * \brief Custom exception class for provider connection related exceptions.
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsProviderConnectionException: public QgsException
{
  public:

    /**
     * Constructor for QgsProviderConnectionException, with the specified error \a message.
     */
    QgsProviderConnectionException( const QString &message ) : QgsException( message ) {}

};

/**
 * \class QgsNotSupportedException
 * \ingroup core
 * \brief Custom exception class which is raised when an operation is not supported.
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsNotSupportedException : public QgsException
{
  public:

    /**
     * Constructor for QgsNotSupportedException, with the specified error \a message.
     */
    QgsNotSupportedException( const QString &message ) : QgsException( message ) {}
};

/**
 * \class QgsSettingsException
 * \ingroup core
 * \brief Custom exception class for settings related exceptions.
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsException : public QgsException
{
  public:

    /**
     * Constructor for QgsProcessingException, with the specified error \a message.
     */
    QgsSettingsException( const QString &message ) : QgsException( message ) {}

};

#endif
