/***************************************************************************
    qgsvariantutils.h
    ------------------
    Date                 : January 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVARIANTUTILS_H
#define QGSVARIANTUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

/**
 * \ingroup core
 * \class QgsVariantUtils
 *
 * \brief Contains utility functions for working with QVariants and QVariant types.
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsVariantUtils
{
  public:

    /**
     * Returns a user-friendly translated string representing a QVariant \a type.
     *
     * The optional \a subType can be used to specify the type of variant list or map values.
     */
    static QString typeToDisplayString( QMetaType::Type type, QMetaType::Type subType = QMetaType::Type::UnknownType );

    /**
     * Returns a user-friendly translated string representing a QVariant \a type.
     *
     * The optional \a subType can be used to specify the type of variant list or map values.
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED static QString typeToDisplayString( QVariant::Type type, QVariant::Type subType = QVariant::Type::Invalid ) SIP_DEPRECATED;

    /**
     * Returns TRUE if the specified \a variant should be considered a NULL value.
     *
     * This method is more rigorous vs QVariant::isNull(), which will return
     * FALSE on newer Qt versions for tests like `QVariant( QDateTime() ).isNull()`.
     *
     * \since QGIS 3.28
     */
    static bool isNull( const QVariant &variant, bool silenceNullWarnings SIP_PYARGREMOVE = false );

    /**
     * Returns TRUE if the specified \a metaType is a numeric type.
     * \since QGIS 3.40
     */
    static bool isNumericType( QMetaType::Type metaType );

    /**
     * Converts a QVariant::Type to a QMetaType::Type.
     * \see metaTypeToVariantType()
     * \since QGIS 3.36
     */
    static QMetaType::Type variantTypeToMetaType( QVariant::Type variantType ) SIP_SKIP;

    /**
     * Converts a QMetaType::Type to a QVariant::Type.
     *
     * \warning Not all QMetaType::Type values can be represented as a QVariant::Type.
     * If no conversion is possible, QVariant::UserType will be returned. Some conversions
     * are lossy, in that the QVariant::Type cannot represent the full range of values
     * possible in QMetaType::Type. In these cases the returned type will be an "expanded"
     * type capable of storing the full range of values possible in the original type.
     *
     * \see variantTypeToMetaType()
     * \since QGIS 3.36
     */
    static QVariant::Type metaTypeToVariantType( QMetaType::Type metaType ) SIP_SKIP;


    // TODO QGIS 4 remove this method

    /**
     * Helper method to properly create a null QVariant from a \a metaType
     * Returns the created QVariant
     */
    static QVariant createNullVariant( QMetaType::Type metaType ) SIP_SKIP;

};

#endif // QGSVARIANTUTILS_H
