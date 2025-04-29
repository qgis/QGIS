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
 * \brief Makes metadata of processing parameters available.
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingParameterType
{
  public:

    /**
     * Creates a new parameter of this type.
     */
    virtual QgsProcessingParameterDefinition *create( const QString &name ) const = 0 SIP_FACTORY;

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
    virtual Qgis::ProcessingParameterTypeFlags flags() const;

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

    /**
     * Returns a list of compatible Processing parameter types for inputs
     * for this parameter type.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingParameterDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingParameterDefinition::type()
     * values which can be used as input values for the parameter.
     *
     * \see acceptedOutputTypes()
     * \see acceptedDataTypes()
     * \since QGIS 3.44
     */
    virtual QStringList acceptedParameterTypes() const = 0;

    /**
     * Returns a list of compatible Processing output types for inputs
     * for this parameter type.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingOutputDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingOutputDefinition::type()
     * values which can be used as values for the parameter.
     *
     * \see acceptedParameterTypes()
     * \see acceptedDataTypes()
     * \since QGIS 3.44
     */
    virtual QStringList acceptedOutputTypes() const = 0;

    /**
     * Returns a list of compatible Processing data types for inputs
     * for this parameter type for the specified \a parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds
     * to the various available values from QgsProcessing::SourceType.
     *
     * Subclasses should return a list of all QgsProcessing::SourceType
     * values which can be used as values for the parameter.
     *
     * \see acceptedParameterTypes()
     * \see acceptedOutputTypes()
     * \since QGIS 3.44
     */
    virtual QList<int> acceptedDataTypes( const QgsProcessingParameterDefinition *parameter ) const;

};

#endif // QGSPROCESSINGPARAMETERTYPE_H
