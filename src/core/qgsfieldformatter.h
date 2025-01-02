/***************************************************************************
  qgsfieldformatter.h - QgsFieldFormatter

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDKIT_H
#define QGSFIELDKIT_H

#include <QString>
#include <QVariant>

#include "qgis_core.h"
#include "qgsvectorlayerref.h"

class QgsVectorLayer;

/**
 * \ingroup core
 * \brief A context for field formatter containing information like the project
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsFieldFormatterContext
{
  public:

    QgsFieldFormatterContext() = default;

    /**
     * Returns the project used in field formatter
     * \see setProject()
     */
    QgsProject *project() const { return mProject; }

    /**
     * Sets the \a project used in field formatter
     * \see project()
     */
    void setProject( QgsProject *project ) { mProject = project; }

  private:
    QgsProject *mProject = nullptr;
};

/**
 * \ingroup core
 * \brief A field formatter helps to handle and display values for a field.
 *
 * It allows for using a shared configuration with the editor widgets
 * for representation of attribute values.
 * Field kits normally have one single instance which is managed by the
 * QgsFieldFormatterRegistry. Custom field formatters should be registered there and
 * field formatters for use within code should normally be obtained from there.
 *
 * This is an abstract base class and will always need to be subclassed.
 *
 */
class CORE_EXPORT QgsFieldFormatter
{
  public:

    QgsFieldFormatter() = default;

    virtual ~QgsFieldFormatter() = default;

    /**
     * Flags for the abilities of the formatter
     *
     * \since QGIS 3.12
     */
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      CanProvideAvailableValues =  1   //!< Can provide possible values
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Returns a unique id for this field formatter.
     * This id will later be used to identify this field formatter in the registry with QgsFieldFormatterRegistry::fieldFormatter().
     *
     * This id matches the id of a QgsEditorWidgetFactory.
     */
    virtual QString id() const = 0;

    /**
     * Create a pretty String representation of the value.
     *
     * \returns By default the string representation of the provided value as implied by the field definition is returned.
     *
     */
    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    /**
     * If the default sort order should be overwritten for this widget, you can transform the value in here.
     *
     * \returns an unmodified value by default.
     *
     */
    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    /**
     * Returns the alignment for a particular field. By default this will consider the field type but can be overwritten if mapped
     * values are represented.
     *
     */
    virtual Qt::AlignmentFlag alignmentFlag( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;

    /**
     * Create a cache for a given field.
     *
     * This will be used in situations where a field is being represented various times in a loop. And will be passed
     * to other methods on QgsFieldKit and QgsEditorWidgetWrapper.
     *
     * For example, the attribute table will create a cache once for each field and then use this
     * cache for representation. The QgsValueRelationFieldFormatter and QgsValueRelationEditorWidget
     * implement this functionality to create a lookuptable once (a QVariantMap / dict) and are
     * make use of a cache if present.
     *
     */
    virtual QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;

    /**
     * Returns a list of weak layer references to other layers required by this formatter
     * for the given \a config.
     * The default implementation returns an empty list.
     *
     * This method should be reimplemented by formatters that handle relations with other layers,
     * (e.g. ValueRelation) and can be used by client code to warn the user about
     * missing required dependencies or to add some resolution logic in order
     * to load the missing dependency.
     * \note not available in Python bindings
     * \since QGIS 3.12
     */
    virtual QList< QgsVectorLayerRef > layerDependencies( const QVariantMap &config ) const SIP_SKIP;

    /**
     * Returns a list of the values that would be possible to select with this widget type
     * On a RelationReference that would be the parents ids or on ValueMap all the configured keys
     * according to the settings in the \a config
     * \since QGIS 3.12
     */
    virtual QVariantList availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const;

    /**
     * Returns the flags
     *
     * \since QGIS 3.12
     */
    Flags flags() const { return mFlags; }

    /**
     * Sets the \a flags
     *
     * \since QGIS 3.12
     */
    void setFlags( const Flags &flags );

  private:
    Flags mFlags;
};

#endif // QGSFIELDKIT_H
