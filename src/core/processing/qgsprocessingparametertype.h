/***************************************************************************
                         qgsprocessingparametertype.h
                         ------------------------
    begin                : March 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGPARAMETERTYPE_H
#define QGSPROCESSINGPARAMETERTYPE_H

#include "qgsprocessingparameters.h"
#include "qgis.h"
#include "qgis_sip.h"
#include <QObject>

/**
 * Makes metadata of processing parameters available.
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterType
{
  public:

    /**
     * Each parameter type can offer a number of additional flags to finetune its behavior
     * and capabilities.
     */
    enum ParameterFlag
    {
      ExposeToModeler = 1 //!< Is this parameter available in the modeler. Is set to on by default.
    };
    Q_DECLARE_FLAGS( ParameterFlags, ParameterFlag )


    /**
     * Creates a new parameter of this type.
     */
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const = 0 SIP_FACTORY;

    /**
     * Destructor
     */
    virtual ~QgsProcessingParameterType() = default;

    /**
     * A human readable and translatable description for this parameter type.
     * This can be a longer description suitable for tooltips and other elements
     * that give the user context for a given parameter.
     */
    virtual QString description() const = 0;

    /**
     * A human readable and translatable short name for this parameter type.
     * This will be used in comboboxes and list widgets.
     */
    virtual QString name() const = 0;

    // TODO QGIS 4.0 -- make pure virtual

    /**
     * Returns a valid Python import string for importing the corresponding parameter type,
     * e.g. "from qgis.core import QgsProcessingParameterBoolean".
     *
     * \see className()
     * \since QGIS 3.6
     */
    virtual QString pythonImportString() const { return QString(); }

    // TODO QGIS 4.0 -- make pure virtual

    /**
     * Returns the corresponding class name for the parameter type.
     *
     * \see pythonImportString()
     * \since QGIS 3.6
     */
    virtual QString className() const
    {
      return name(); // this is wrong, but it's better than nothing for subclasses which don't implement this method
    }

    /**
     * A static id for this type which will be used for storing this parameter type.
     */
    virtual QString id() const = 0;

    /**
     * Determines if this parameter is available in the modeler.
     * The default implementation returns TRUE.
     */
    virtual ParameterFlags flags() const;

    /**
     * Metadata for this parameter type. Can be used for example to define custom widgets.
     * The default implementation returns an empty map.
     */
    virtual QVariantMap metadata() const;

    /**
     * Returns a list of the Python data types accepted as values for the parameter.
     * E.g. "str", "QgsVectorLayer", "QgsMapLayer", etc.
     *
     * These values should should match the Python types exactly
     * (e.g. "str" not "string", "bool" not "boolean"). Extra explanatory help can
     * be used (which must be translated), eg "str: as comma delimited list of numbers".
     *
     * \see acceptedStringValues()
     */
    virtual QStringList acceptedPythonTypes() const;

    /**
     * Returns a descriptive list of the possible string values acceptable for the parameter.
     *
     * E.g. for a QgsProcessingParameterVectorLayer this may include "Path to a vector layer",
     * for QgsProcessingParameterBoolean "1 for true, 0 for false" etc.
     *
     * Extra explanatory help can be used (which must be translated), eg "a comma delimited list of numbers".
     *
     * \see acceptedPythonTypes()
     * \since QGIS 3.8
     */
    virtual QStringList acceptedStringValues() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingParameterType::ParameterFlags )

#endif // QGSPROCESSINGPARAMETERTYPE_H
