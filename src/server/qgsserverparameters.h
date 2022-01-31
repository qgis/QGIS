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
#include <QMetaEnum>
#include <QUrlQuery>
#include <QColor>
#include "qgsgeometry.h"
#include "qgis_server.h"

/**
 * \ingroup server
 * \class QgsServerParameterDefinition
 * \brief Definition of a parameter with basic conversion methods
 * \since QGIS 3.4
 */
class SERVER_EXPORT QgsServerParameterDefinition
{
  public:

    /**
     * Constructor for QgsServerParameterDefinition.
     * \param type The type of the parameter
     * \param defaultValue The default value of the parameter
     */
    QgsServerParameterDefinition( const QVariant::Type type = QVariant::String,
                                  const QVariant defaultValue = QVariant( "" ) );

    /**
     * Default destructor for QgsServerParameterDefinition.
     */
    virtual ~QgsServerParameterDefinition() = default;

    /**
     * Returns the type of the parameter as a string.
     */
    QString typeName() const;

    /**
     * Returns TRUE if the parameter is valid, FALSE otherwise.
     */
    virtual bool isValid() const;

    /**
     * Converts the parameter into a string. If \a defaultValue is true
     * and current value is empty, then the default value is returned.
     */
    QString toString( bool defaultValue = false ) const;

    /**
     * Converts the parameter into a list of strings
     * \param delimiter The character used for delimiting
     * \param skipEmptyParts To use QString::SkipEmptyParts for splitting
     * \returns A list of strings
     */
    QStringList toStringList( char delimiter = ',', bool skipEmptyParts = true ) const;

    /**
     * Converts the parameter into a list of integers.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \param delimiter The character used for delimiting
     * \returns A list of integers
     */
    QList<int> toIntList( bool &ok, char delimiter = ',' ) const;

    /**
     * Converts the parameter into a list of doubles.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \param delimiter The character used for delimiting
     * \returns A list of doubles
     */
    QList<double> toDoubleList( bool &ok, char delimiter = ',' ) const;

    /**
     * Converts the parameter into a list of colors.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \param delimiter The character used for delimiting
     * \returns A list of colors
     */
    QList<QColor> toColorList( bool &ok, char delimiter = ',' ) const;

    /**
     * Converts the parameter into a list of geometries.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \param delimiter The character used for delimiting
     * \returns A list of geometries
     */
    QList<QgsGeometry> toGeomList( bool &ok, char delimiter = ',' ) const;

    /**
     * Converts the parameter into a list of OGC filters.
     * \returns A list of strings
     * \since QGIS 3.24
     */
    QStringList toOgcFilterList() const;

    /**
     * Converts the parameter into a list of QGIS expressions.
     * \returns A list of strings
     * \since QGIS 3.24
     */
    QStringList toExpressionList() const;

    /**
     * Converts the parameter into a rectangle.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \returns A rectangle
     */
    QgsRectangle toRectangle( bool &ok ) const;

    /**
     * Converts the parameter into an integer.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \returns An integer
     */
    int toInt( bool &ok ) const;

    /**
     * Converts the parameter into a double.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \returns A double
     */
    double toDouble( bool &ok ) const;

    /**
     * Converts the parameter into a boolean.
     * \returns A boolean
     */
    bool toBool() const;

    /**
     * Converts the parameter into a color.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \returns A color
     */
    QColor toColor( bool &ok ) const;

    /**
     * Converts the parameter into an url.
     * \param ok TRUE if there's no error during the conversion, FALSE otherwise
     * \returns An url
     * \since QGIS 3.4
     */
    QUrl toUrl( bool &ok ) const;

    /**
     * Loads the data associated to the parameter converted into an url.
     * \param ok TRUE if there's no error during the load, FALSE otherwise
     * \returns The content loaded
     * \since QGIS 3.4
     */
    QString loadUrl( bool &ok ) const;

    /**
     * Raises an exception in case of an invalid parameters.
     * \param msg The message describing the exception
     * \throws QgsBadRequestException Invalid parameter exception
     */
    static void raiseError( const QString &msg );

    QVariant::Type mType;
    QVariant mValue;
    QVariant mDefaultValue;
};

/**
 * \ingroup server
 * \class QgsServerParameter
 * \brief Parameter common to all services (WMS, WFS, ...)
 * \since QGIS 3.4
 */
class SERVER_EXPORT QgsServerParameter : public QgsServerParameterDefinition
{
    Q_GADGET

  public:
    //! Parameter's name common to all services
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

    /**
     * Constructor for QgsServerParameter.
     * \param name The name of the parameter
     * \param type The type of the parameter
     * \param defaultValue The default value to use if not defined
     */
    QgsServerParameter( const QgsServerParameter::Name name = QgsServerParameter::UNKNOWN,
                        const QVariant::Type type = QVariant::String,
                        const QVariant defaultValue = QVariant( "" ) );

    /**
     * Raises an error in case of an invalid conversion.
     * \throws QgsBadRequestException Invalid parameter exception
     */
    void raiseError() const;

    /**
     * Converts a parameter's name into its string representation.
     */
    static QString name( const QgsServerParameter::Name name );

    /**
     * Converts a string into a parameter's name (UNKNOWN in case of an
     * invalid string).
     */
    static QgsServerParameter::Name name( const QString &name );

    QgsServerParameter::Name mName;
};

/**
 * \ingroup server
 * \class QgsServerParameters
 * \brief QgsServerParameters provides an interface to retrieve and manipulate global parameters received from the client.
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
    virtual QString request() const;

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
    virtual QString version() const;

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
    QUrlQuery mUrlQuery;
};

#endif
