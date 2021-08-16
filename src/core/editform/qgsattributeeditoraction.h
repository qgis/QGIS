/***************************************************************************
  qgsattributeeditoraction.h - QgsAttributeEditorAction

 ---------------------
 begin                : 14.8.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORACTION_H
#define QGSATTRIBUTEEDITORACTION_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"
#include "qgsaction.h"

/**
 * \ingroup core
 * \brief This element will load a layer action onto the form.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAttributeEditorAction : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new element which can display a layer action.
     *
     * \param action       The action
     * \param parent       The parent (used as container)
    */
    QgsAttributeEditorAction( const QgsAction &action, QgsAttributeEditorElement *parent );

    /**
     * Creates a new element which can display a layer action, this constructor allows
     * to create a QgsAttributeEditorAction when actions are not yet loaded.
     *
     * \param uuid         The action unique identifier (UUID).
     * \param layerId      The ID of the layer the action belongs to.
     * \param parent       The parent (used as container).
    */
    QgsAttributeEditorAction( const QUuid &uuid, const QString &layerId, QgsAttributeEditorElement *parent );

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * Returns the (possibly lazy loaded) action.
     */
    const QgsAction &action() const;

    /**
     * Set the action to \a newAction.
     */
    void setAction( const QgsAction &newAction );

    /**
     * Returns the action's unique identifier (UUID).
     */
    const QString actionId() const;

    /**
     * Returns the action short title (if defined) or the action name as a fallback for display.
     */
    const QString actionDisplayName() const;

  private:

    // Lazy loaded
    mutable QgsAction mAction;
    QUuid mUuid;
    QString mLayerId;

    // QgsAttributeEditorElement interface
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;

};

#endif // QGSATTRIBUTEEDITORACTION_H
