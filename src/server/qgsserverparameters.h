/***************************************************************************
                              qgsserverparameters.h
                              ------------------
  begin                : Jun 27, 2018
  copyright            : (C) 2018 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERPARAMETERS_H
#define QGSSERVERPARAMETERS_H

#include <QMap>
#include <QObject>
#include <QMetaEnum>
#include <QUrlQuery>
#include "qgis_server.h"

/**
 * QgsServerParameters provides an interface to retrieve and manipulate
 * global parameters received from the client.
 * \since QGIS 3.4
 */
class SERVER_EXPORT QgsServerParameters
{
    Q_GADGET

  public:
    enum ParameterName
    {
      SERVICE,
      VERSION_SERVICE, // should be VERSION, but there's a conflict with #define VERSION
      REQUEST,
      MAP,
      FILE_NAME
    };
    Q_ENUM( ParameterName )

    struct Parameter
    {
      ParameterName mName;
      QVariant::Type mType;
      QVariant mDefaultValue;
      QVariant mValue;
      bool mDefined;
    };

    /**
     * Constructor.
     */
    QgsServerParameters();

    /**
     * Constructor.
     */
    QgsServerParameters( const QUrlQuery &query );

    /**
     * Loads new parameters.
     * \param query url query
     */
    void load( const QUrlQuery &query );

    /**
     * Removes all parameters.
     */
    void clear();

    /**
     * Adds a parameter.
     * \param key the name of the parameter
     * \param value the value of the parameter
     */
    void add( const QString &key, const QString &value );

    /**
     * Removes a parameter.
     * \param key the name of the parameter
     */
    void remove( const QString &key );

    /**
     * Returns the value of a parameter.
     * \param key the name of the parameter
     */
    QString value( const QString &key ) const;

    /**
     * Returns a url query with underlying parameters.
     */
    QUrlQuery urlQuery() const;

    /**
     * Returns all parameters in a map.
     */
    QMap<QString, QString> toMap() const;

    /**
     * Returns SERVICE parameter as a string or an empty string if not
     * defined.
     * \returns service
     */
    QString service() const;

    /**
     * Returns REQUEST parameter as a string or an empty string if not
     * defined.
     * \returns request
     */
    QString request() const;

    /**
     * Returns MAP parameter as a string or an empty string if not
     * defined.
     * \returns map
     */
    QString map() const;

    /**
     * Returns  FILE_NAME parameter as a string or an empty string if not
     * defined.
     * \returns filename
     */
    QString fileName() const;

    /**
     * Returns VERSION parameter as a string or an empty string if not
     * defined.
     * \returns version
     */
    QString version() const;

  private:
    void save( const Parameter &parameter );
    QVariant value( ParameterName name ) const;

    ParameterName name( const QString &name ) const;
    QString name( ParameterName name ) const;

    void raiseError( ParameterName name ) const;
    void raiseError( const QString &msg ) const;

    QMap<ParameterName, Parameter> mParameters;
    QMap<QString, QString> mUnmanagedParameters;
};

#endif
