/***************************************************************************
                         qgsprocessingmodelcomponent.h
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

#ifndef QGSPROCESSINGMODELCOMPONENT_H
#define QGSPROCESSINGMODELCOMPONENT_H

#include "qgis_core.h"
#include "qgis.h"
#include <QPointF>

///@cond NOT_STABLE

/**
 * Represents a component of a model algorithm.
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingModelComponent
{
  public:

    /**
     * Returns the friendly description text for the component.
     * \see setDescription()
     */
    QString description() const;

    /**
     * Sets the friendly \a description text for the component.
     * \see description()
     */
    void setDescription( const QString &description );

    /**
     * Returns the position of the model component within the graphical modeler.
     * \see setPosition()
     */
    QPointF position() const;

    /**
     * Sets the \a position of the model component within the graphical modeler.
     * \see position()
     */
    void setPosition( QPointF position );

  protected:

    //! Only subclasses can be created
    QgsProcessingModelComponent( const QString &description = QString() );

    //! Copies are protected to avoid slicing
    QgsProcessingModelComponent( const QgsProcessingModelComponent &other ) = default;

    //! Copies are protected to avoid slicing
    QgsProcessingModelComponent &operator=( const QgsProcessingModelComponent &other ) = default;

    /**
     * Saves the component properties to a QVariantMap.
     * \see restoreCommonProperties()
     */
    void saveCommonProperties( QVariantMap &map ) const;

    /**
     * Restores the component properties from a QVariantMap.
     * \see saveCommonProperties()
     */
    void restoreCommonProperties( const QVariantMap &map );

  private:

    //! Position of component within model
    QPointF mPosition;

    QString mDescription;

};

///@endcond

#endif // QGSPROCESSINGMODELCOMPONENT_H
