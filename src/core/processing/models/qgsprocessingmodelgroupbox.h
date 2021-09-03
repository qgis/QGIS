/***************************************************************************
                         qgsprocessingmodelgroupbox.h
                         --------------------------
    begin                : March 2020
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

#ifndef QGSPROCESSINGMODELGROUPBOX_H
#define QGSPROCESSINGMODELGROUPBOX_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingparameters.h"

///@cond NOT_STABLE

/**
 * \brief Represents a group box in a model.
 * \ingroup core
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingModelGroupBox : public QgsProcessingModelComponent
{
  public:

    /**
     * Constructor for QgsProcessingModelGroupBox with the specified \a description.
     */
    QgsProcessingModelGroupBox( const QString &description = QString() );

    QgsProcessingModelGroupBox *clone() const override SIP_FACTORY;

    /**
     * Saves this group box to a QVariant.
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Loads this group box from a QVariantMap.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map, bool ignoreUuid = false );

    /**
     * Returns the unique ID associated with this group box.
     */
    QString uuid() const;

  private:

    QString mUuid;
};

///@endcond

#endif // QGSPROCESSINGMODELGROUPBOX_H
