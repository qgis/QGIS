#ifndef QGSPROCESSINGPARAMETERTYPE_H
#define QGSPROCESSINGPARAMETERTYPE_H

#include "qgsprocessingparameters.h"
#include "qgis.h"
#include "qgis_sip.h"
#include <QObject>

/**
 * Makes metadata of processing parameters available.
 *
 * \since QGIS 3.2
 * \ingroup core
 */
class CORE_EXPORT QgsProcessingParameterType
{
  public:

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

    /**
     * A static id for this type which will be used for storing this parameter type.
     */
    virtual QString id() const = 0;

    /**
     * Determines if this parameter is available in the modeler.
     * The default implementation returns true.
     */
    virtual bool exposeToModeler() const;

    /**
     * Metadata for this parameter type. Can be used for example to define custom widgets.
     * The default implementation returns an empty map.
     */
    virtual QVariantMap metadata() const;
};

#endif // QGSPROCESSINGPARAMETERTYPE_H
