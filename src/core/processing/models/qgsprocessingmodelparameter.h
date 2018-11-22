/***************************************************************************
                         qgsprocessingmodelparameter.h
                         -----------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSPROCESSINGMODELPARAMETER_H
#define QGSPROCESSINGMODELPARAMETER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingmodelcomponent.h"

///@cond NOT_STABLE


/**
 * Represents an input parameter used by the model.
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingModelParameter : public QgsProcessingModelComponent
{
  public:

    /**
     * Constructor for QgsProcessingModelParameter. The parameter name should match one of the
     * parameters from the parent model.
     */
    QgsProcessingModelParameter( const QString &parameterName = QString() );

    /**
     * Returns the associated parameter name. The parameter name should match one of the
     * parameters from the parent model.
     * \see parameterName()
     */
    QString parameterName() const { return mParameterName; }

    /**
     * Sets the associated parameter name. The parameter name should match one of the
     * parameters from the parent model.
     * \see parameterName()
     */
    void setParameterName( const QString &name ) { mParameterName = name; }

    /**
     * Saves this parameter to a QVariant.
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Loads this parameter from a QVariantMap.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map );

  private:

    QString mParameterName;

};

///@endcond

#endif // QGSPROCESSINGMODELPARAMETER_H
