/***************************************************************************
  qgsattributeeditorcontainer.h - QgsAttributeEditorElement

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORCONTAINER_H
#define QGSATTRIBUTEEDITORCONTAINER_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"
#include "qgsoptionalexpression.h"

/**
 * \ingroup core
 * \brief This is a container for attribute editors, used to group them visually in the
 * attribute form if it is set to the drag and drop designer.
 */
class CORE_EXPORT QgsAttributeEditorContainer : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new attribute editor container
     *
     * \param name   The name to show as title
     * \param parent The parent. May be another container.
     * \param backgroundColor The optional background color of the container.
     */
    QgsAttributeEditorContainer( const QString &name, QgsAttributeEditorElement *parent, const QColor &backgroundColor = QColor() )
      : QgsAttributeEditorElement( Qgis::AttributeEditorType::Container, name, parent )
      , mBackgroundColor( backgroundColor )
    {}

    ~QgsAttributeEditorContainer() override;

    /**
     * Add a child element to this container. This may be another container, a field or a relation.
     *
     * \param element The element to add as child
     */
    virtual void addChildElement( QgsAttributeEditorElement *element SIP_TRANSFER );

    /**
     * Sets the container type.
     *
     * \see type()
     * \since QGIS 3.32
     */
    void setType( Qgis::AttributeEditorContainerType type ) { mType = type; }

    /**
     * Returns the container type.
     *
     * \see setType()
     * \since QGIS 3.32
     */
    Qgis::AttributeEditorContainerType type() const { return mType; }

    /**
     * Determines if this container is rendered as collapsible group box or tab in a tabwidget
     *
     * \param isGroupBox If TRUE, this will be a group box
     * \deprecated QGIS 3.40. Use setType() instead.
     */
    Q_DECL_DEPRECATED virtual void setIsGroupBox( bool isGroupBox ) SIP_DEPRECATED;

    /**
     * Returns if this container is going to be a group box
     *
     * \returns TRUE if it will be a group box, FALSE if it will be a tab
     *
     * \deprecated QGIS 3.40. Use type() instead.
     */
    Q_DECL_DEPRECATED virtual bool isGroupBox() const SIP_DEPRECATED;

    /**
     * For group box containers returns TRUE if this group box is collapsed.
     *
     * \returns TRUE if the group box is collapsed, FALSE otherwise.
     * \see collapsed()
     * \see setCollapsed()
     * \since QGIS 3.26
     */
    bool collapsed() const { return mCollapsed; };

    /**
     * For group box containers sets if this group box is \a collapsed.
     *
     * \see collapsed()
     * \see setCollapsed()
     * \since QGIS 3.26
     */
    void setCollapsed( bool collapsed ) { mCollapsed = collapsed; };

    /**
     * Gets a list of the children elements of this container
     *
     * \returns A list of elements
     */
    QList<QgsAttributeEditorElement *> children() const { return mChildren; }

    /**
     * Traverses the element tree to find any element of the specified type
     *
     * \param type The type which should be searched
     *
     * \returns A list of elements of the type which has been searched for
     */
    virtual QList<QgsAttributeEditorElement *> findElements( Qgis::AttributeEditorType type ) const;

    /**
     * Clear all children from this container.
     */
    void clear();

    /**
     * Change the name of this container.
     */
    void setName( const QString &name );

    /**
     * Gets the number of columns in this group.
     *
     * \see setColumnCount()
     */
    int columnCount() const;

    /**
     * Set the number of columns in this group.
     *
     * \see columnCount()
     */
    void setColumnCount( int columnCount );

    /**
     * Creates a deep copy of this element. To be implemented by subclasses.
     *
     */
    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * The visibility expression is used in the attribute form to
     * show or hide this container based on an expression incorporating
     * the field value controlled by editor widgets.
     *
     */
    QgsOptionalExpression visibilityExpression() const;

    /**
     * The visibility expression is used in the attribute form to
     * show or hide this container based on an expression incorporating
     * the field value controlled by editor widgets.
     *
     */
    void setVisibilityExpression( const QgsOptionalExpression &visibilityExpression );

    /**
     * The collapsed expression is used in the attribute form to
     * set the collapsed status of the group box container container based on an expression incorporating
     * the field value controlled by editor widgets. This property is ignored if the container is not
     * a group box.
     *
     * \note Not available in Python bindings
     * \see setCollapsedExpression()
     * \since QGIS 3.26
     */
    QgsOptionalExpression collapsedExpression() const SIP_SKIP;

    /**
     * The collapsed expression is used in the attribute form to
     * set the collapsed status of the group box of this container based on an expression incorporating
     * the field value controlled by editor widgets. This property is ignored if the container is not
     * a group box.
     *
     * \note Not available in Python bindings
     * \see collapsedExpression()
     * \since QGIS 3.26
     */
    void setCollapsedExpression( const QgsOptionalExpression &collapsedExpression ) SIP_SKIP;

    /**
     * Returns the background color of the container.
     *
     * \see setBackgroundColor()
     * \since QGIS 3.8
     */
    QColor backgroundColor() const;

    /**
     * Sets the background color to \a backgroundColor.
     *
     * \see backgroundColor()
     */
    void setBackgroundColor( const QColor &backgroundColor );

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element,  const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;

    Qgis::AttributeEditorContainerType mType = Qgis::AttributeEditorContainerType::GroupBox;
    QList<QgsAttributeEditorElement *> mChildren;
    int mColumnCount = 1;
    QgsOptionalExpression mVisibilityExpression;
    QColor mBackgroundColor;
    bool mCollapsed = false;
    QgsOptionalExpression mCollapsedExpression;
};


#endif // QGSATTRIBUTEEDITORCONTAINER_H
