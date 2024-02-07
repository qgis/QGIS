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
#include <QSizeF>
#include <QColor>

class QgsProcessingModelComment;

///@cond NOT_STABLE

/**
 * \brief Represents a component of a model algorithm.
 * \ingroup core
 */
class CORE_EXPORT QgsProcessingModelComponent
{
  public:

    virtual ~QgsProcessingModelComponent() = default;

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
     * Returns the position of the model component within the model designer.
     * \see setPosition()
     */
    QPointF position() const;

    /**
     * Sets the \a position of the model component within the model designer.
     * \see position()
     */
    void setPosition( QPointF position );

    /**
     * Returns the size of the model component within the model designer.
     * \see setSize()
     * \since QGIS 3.14
     */
    QSizeF size() const;

    /**
     * Sets the \a size of the model component within the model designer.
     * \see size()
     * \since QGIS 3.14
     */
    void setSize( QSizeF size );

    /**
     * Returns the color of the model component within the model designer.
     *
     * An invalid color indicates that the default color for the component should be used.
     *
     * \see setColor()
     * \since QGIS 3.14
     */
    QColor color() const;

    /**
     * Sets the \a color of the model component within the model designer. An invalid \a color
     * indicates that the default color for the component should be used.
     *
     * \see color()
     * \since QGIS 3.14
     */
    void setColor( const QColor &color );

    /**
     * Returns TRUE if the link points for the specified \a edge should be shown collapsed or not.
     * \see setLinksCollapsed()
     */
    bool linksCollapsed( Qt::Edge edge ) const;

    /**
     * Sets whether the link points for the specified \a edge for this component should be shown collapsed
     * in the model designer.
     * \see linksCollapsed()
     */
    void setLinksCollapsed( Qt::Edge edge, bool collapsed );

    /**
     * Returns the comment attached to this component (may be NULLPTR)
     * \see setComment()
     */
    SIP_SKIP virtual const QgsProcessingModelComment *comment() const { return nullptr; }

    /**
     * Returns the comment attached to this component (may be NULLPTR)
     * \see setComment()
     */
    virtual QgsProcessingModelComment *comment() { return nullptr; }

    /**
     * Sets the \a comment attached to this component.
     * \see comment()
     */
    virtual void setComment( const QgsProcessingModelComment &comment );

    /**
     * Clones the component.
     *
     * Ownership is transferred to the caller.
     */
    virtual QgsProcessingModelComponent *clone() const = 0 SIP_FACTORY;

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

    /**
     * Copies all non-specific definition properties from the \a other component definition.
     *
     * This includes properties like the size and position of the component, but not properties
     * like the specific algorithm or input details.
     *
     * \since QGIS 3.14
     */
    void copyNonDefinitionProperties( const QgsProcessingModelComponent &other );

  private:

    static constexpr double DEFAULT_COMPONENT_WIDTH = 200;
    static constexpr double DEFAULT_COMPONENT_HEIGHT = 30;

    //! Position of component within model
    QPointF mPosition;

    QString mDescription;

    QSizeF mSize = QSizeF( DEFAULT_COMPONENT_WIDTH, DEFAULT_COMPONENT_HEIGHT );
    QColor mColor;

    bool mTopEdgeLinksCollapsed = true;
    bool mBottomEdgeLinksCollapsed = true;

};

///@endcond

#endif // QGSPROCESSINGMODELCOMPONENT_H
