/***************************************************************************
  qgsvectorlayerutils.h
  ---------------------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERUTILS_H
#define QGSVECTORLAYERUTILS_H

#include "qgsvectorlayer.h"

/** \ingroup core
 * \class QgsVectorLayerUtils
 * \brief Contains utility methods for working with QgsVectorLayers.
 *
 * \note Added in version 3.0
 */

class CORE_EXPORT QgsVectorLayerUtils
{
  public:

    /**
     * Returns true if the specified value already exists within a field. This method can be used to test for uniqueness
     * of values inside a layer's attributes. An optional list of ignored feature IDs can be provided, if so, any features
     * with IDs within this list are ignored when testing for existance of the value.
     */
    static bool valueExists( const QgsVectorLayer* layer, int fieldIndex, const QVariant& value, const QgsFeatureIds& ignoreIds = QgsFeatureIds() );

    /**
     * Tests an attribute value to check whether it passes all constraints which are present on the corresponding field.
     * Returns true if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
     * If the strength or origin parameter is set then only constraints with a matching strength/origin will be checked.
     */
    static bool validateAttribute( const QgsVectorLayer* layer, const QgsFeature& feature, int attributeIndex, QStringList& errors,
                                   QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthNotSet,
                                   QgsFieldConstraints::ConstraintOrigin origin = QgsFieldConstraints::ConstraintOriginNotSet );

};

#endif // QGSVECTORLAYERUTILS_H
