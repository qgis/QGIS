/***************************************************************************
                         qgsprocessingmodelcomment.h
                         --------------------------
    begin                : February 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSPROCESSINGMODELCOMMENT_H
#define QGSPROCESSINGMODELCOMMENT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingparameters.h"

///@cond NOT_STABLE

/**
 * \brief Represents a comment in a model.
 * \ingroup core
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingModelComment : public QgsProcessingModelComponent
{
  public:

    /**
     * Constructor for QgsProcessingModelComment with the specified \a description.
     */
    QgsProcessingModelComment( const QString &description = QString() );

    QgsProcessingModelComment *clone() const override SIP_FACTORY;

    /**
     * Saves this comment to a QVariant.
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Loads this comment from a QVariantMap.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map );
};

///@endcond

#endif // QGSPROCESSINGMODELCOMMENT_H
