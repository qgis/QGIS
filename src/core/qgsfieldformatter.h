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

class QgsVectorLayer;

/**
 * \ingroup core
 * A field formatter helps to handle and display values for a field.
 *
 * It allows for using a shared configuration with the editor widgets
 * for representation of attribute values.
 * Field kits normally have one single instance which is managed by the
 * QgsFieldFormatterRegistry. Custom field formatters should be registered there and
 * field formatters for use within code should normally be obtained from there.
 *
 * This is an abstract base class and will always need to be subclassed.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFieldFormatter
{
  public:
    QgsFieldFormatter();

    virtual ~QgsFieldFormatter() = default;

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
     * \since QGIS 3.0
     */
    virtual QString representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    /**
     * If the default sort order should be overwritten for this widget, you can transform the value in here.
     *
     * \returns an unmodified value by default.
     *
     * \since QGIS 3.0
     */
    virtual QVariant sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const;

    /**
     * Returns the alignment for a particular field. By default this will consider the field type but can be overwritten if mapped
     * values are represented.
     *
     * \since QGIS 3.0
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
     * \since QGIS 3.0
     */
    virtual QVariant createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const;
};

#endif // QGSFIELDKIT_H
