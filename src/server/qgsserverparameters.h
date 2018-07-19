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
#include <QColor>
#include "qgsgeometry.h"
#include "qgis_server.h"

class SERVER_EXPORT QgsServerParameterDefinition
{
  public:
    QgsServerParameterDefinition( const QVariant::Type type = QVariant::String,
                                  const QVariant defaultValue = QVariant( "" ) );

    virtual ~QgsServerParameterDefinition() = default;

    QString typeName() const;
    virtual bool isValid() const;

    QString toString() const;
    QStringList toStringList( char delimiter = ',' ) const;
    QList<int> toIntList( bool &ok, char delimiter = ',' ) const;
    QList<double> toDoubleList( bool &ok, char delimiter = ',' ) const;
    QList<QColor> toColorList( bool &ok, char delimiter = ',' ) const;
    QList<QgsGeometry> toGeomList( bool &ok, char delimiter = ',' ) const;
    QgsRectangle toRectangle( bool &ok ) const;
    int toInt( bool &ok ) const;
    double toDouble( bool &ok ) const;
    bool toBool() const;
    QColor toColor( bool &ok ) const;

    static void raiseError( const QString &msg );

    QVariant::Type mType;
    QVariant mValue;
    QVariant mDefaultValue;
};

class SERVER_EXPORT QgsServerParameter : public QgsServerParameterDefinition
{
    Q_GADGET

  public:
    enum Name
    {
      UNKNOWN,
      SERVICE,
      VERSION_SERVICE, // conflict with #define VERSION
      REQUEST,
      MAP,
      FILE_NAME
    };
    Q_ENUM( Name )

    QgsServerParameter( const QgsServerParameter::Name name = QgsServerParameter::UNKNOWN,
                        const QVariant::Type type = QVariant::String,
                        const QVariant defaultValue = QVariant( "" ) );

    void raiseError() const;

    static QString name( const QgsServerParameter::Name name );
    static QgsServerParameter::Name name( const QString &name );

    QgsServerParameter::Name mName;
};

/**
 * QgsServerParameters provides an interface to retrieve and manipulate
 * global parameters received from the client.
 * \since QGIS 3.4
 */
class SERVER_EXPORT QgsServerParameters
{
    Q_GADGET

  public:

    /**
     * Constructor.
     */
    QgsServerParameters();

    /**
     * Constructor.
     */
    QgsServerParameters( const QUrlQuery &query );

    virtual ~QgsServerParameters() = default;

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
     * Removes a parameter.
     * \param name The name of the parameter
     * \since QGIS 3.4
     */
    void remove( QgsServerParameter::Name name );

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

  protected:

    /**
     * Loads a parameter with a specific value. This method should be
     * implemented in subclasses.
     */
    virtual bool loadParameter( const QString &name, const QString &value );

    QMap<QString, QString> mUnmanagedParameters;

  private:
    void save( const QgsServerParameter &parameter );
    QVariant value( QgsServerParameter::Name name ) const;

    QMap<QgsServerParameter::Name, QgsServerParameter> mParameters;
};

#endif
