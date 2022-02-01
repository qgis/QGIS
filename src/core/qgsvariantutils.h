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
    static QString typeToDisplayString( QVariant::Type type, QVariant::Type subType = QVariant::Type::Invalid );

};

#endif // QGSVARIANTUTILS_H
