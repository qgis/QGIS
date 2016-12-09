/***************************************************************************
                         qgsprocessingparameterregistry.h
                         -------------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERREGISTRY_H
#define QGSPROCESSINGPARAMETERREGISTRY_H

#include <QString>
#include <QMap>

class QWidget;
class QgsProcessingParameter;

/**
 * \class QgsProcessingParameterAbstractMetadata
 * \ingroup core
 * Stores metadata about one processing parameter class.
 * \note In C++ you can use QgsProcessingParameterMetadata convenience class.
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterAbstractMetadata
{
  public:

    /**
     * Constructor for parameter abstract metadata. The type parameter must be unique.
     */
    QgsProcessingParameterAbstractMetadata( const QString& type )
        : mType( type )
    {}

    virtual ~QgsProcessingParameterAbstractMetadata() {}

    /**
     * Returns the unique type string which identifies the parameter type.
     */
    QString type() const { return mType; }

    /**
     * Creates a new parameter of this type from an encoded script code line. The name, description and optional flag will have already
     * been parsed when this function is called, so only any extra handling of the remaining definition string needs to be done by the class.
     */
    virtual QgsProcessingParameter* createParameterFromScriptCode( const QString& name, const QString& description, bool isOptional, const QString& definition )
    {
      Q_UNUSED( definition );
      Q_UNUSED( description );
      Q_UNUSED( name );
      Q_UNUSED( isOptional );
      return nullptr;
    }

    virtual QWidget* createWidget() { return nullptr; }

  protected:
    QString mType;
};

//! Function to create a widget for the parameter
typedef QWidget*( *QgsProcessingParameterWidgetFunc )();
//! Function to create a parameter from an encoded script code line
typedef QgsProcessingParameter*( *QgsProcessingParameterFromScriptCodeFunc )( const QString&, const QString&, bool, const QString& );

/**
 * \class QgsProcessingParameterMetadata
 * \ingroup core
 * Convenience class that uses static functions to create parameter metadata.
 * \note Not available in Python bindings
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterMetadata : public QgsProcessingParameterAbstractMetadata
{
  public:

    /**
     * Constructor for QgsProcessingParameterMetadata which accepts static functions for the
     * parameter creation functions.
     */
    QgsProcessingParameterMetadata( const QString& type,
                                    QgsProcessingParameterFromScriptCodeFunc pfCreateFromScriptCode = nullptr,
                                    QgsProcessingParameterWidgetFunc pfWidget = nullptr )
        : QgsProcessingParameterAbstractMetadata( type )
        , mCreateFromScriptCodeFunc( pfCreateFromScriptCode )
        , mWidgetFunc( pfWidget )
    {}

    /**
     * Returns the parameter's widget creation function.
     * @see setWidgetFunction()
     */
    QgsProcessingParameterWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the parameter's widget creation function.
     * @see widgetFunction()
     */
    void setWidgetFunction( QgsProcessingParameterWidgetFunc f ) { mWidgetFunc = f; }

    virtual QgsProcessingParameter* createParameterFromScriptCode( const QString& name, const QString& description, bool isOptional, const QString& definition ) override { return mCreateFromScriptCodeFunc ? mCreateFromScriptCodeFunc( name, description, isOptional, definition ) : nullptr; }
    virtual QWidget* createWidget() override { return mWidgetFunc ? mWidgetFunc() : nullptr; }

  private:
    QgsProcessingParameterFromScriptCodeFunc mCreateFromScriptCodeFunc;
    QgsProcessingParameterWidgetFunc mWidgetFunc;

};

/**
 * \class QgsProcessingParameterRegistry
 * \ingroup core
 * Registry of available processing parameter types.
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsProcessingParameterRegistry
{
  public:

    /**
     * Constructs a new QgsProcessingParameterRegistry including all the default parameter types.
     */
    QgsProcessingParameterRegistry();

    ~QgsProcessingParameterRegistry();

    /**
     * Returns a pointer to the metadata for a parameter type.
     */
    QgsProcessingParameterAbstractMetadata* parameterMetadata( const QString& type ) const;

    /**
     * Adds metadata for a new parameter type to the registry. Ownership of the metadata is transferred.
     * Returns true if the parameter type was successfully registered, or false if the type could not
     * be registered (eg as a result of a duplicate type string).
     */
    bool addParameterType( QgsProcessingParameterAbstractMetadata* metadata );

    /**
     * Creates a new parameter from an encoded script code.
     */
    QgsProcessingParameter* createFromScriptCode( const QString& code ) const;

  private:

    QMap<QString, QgsProcessingParameterAbstractMetadata*> mMetadata;

    QgsProcessingParameterRegistry( const QgsProcessingParameterRegistry& rh );
    QgsProcessingParameterRegistry& operator=( const QgsProcessingParameterRegistry& rh );

    static bool parseScriptCodeParameterOptions( const QString& code, bool& isOptional, QString& name, QString& type, QString& definition );

    static QString createDescription( const QString& name );

};
#endif // QGSPROCESSINGPARAMETERREGISTRY_H


