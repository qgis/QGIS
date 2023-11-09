/***************************************************************************
                             qgsannotationitemwidget.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSANNOTATIONITEMWIDGET_H
#define QGSANNOTATIONITEMWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"

class QgsAnnotationItem;

/**
 * \class QgsAnnotationItemBaseWidget
 * \ingroup gui
 *
 * \brief A base class for property widgets for annotation items.
 *
 * All annotation item widgets should inherit from this base class.
 *
 * \since QGIS 3.22
*/
class GUI_EXPORT QgsAnnotationItemBaseWidget: public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAnnotationItemBaseWidget.
     */
    QgsAnnotationItemBaseWidget( QWidget *parent SIP_TRANSFERTHIS );

    /**
     * Creates a new item matching the settings defined in the widget.
     */
    virtual QgsAnnotationItem *createItem() = 0 SIP_FACTORY;

    /**
     * Updates an existing item to match the settings defined in the widget.
     */
    virtual void updateItem( QgsAnnotationItem *item ) = 0;

    /**
     * Sets the current \a item to show in the widget. If TRUE is returned, \a item
     * was an acceptable type for display in this widget and the widget has been
     * updated to match \a item's properties.
     *
     * If FALSE is returned, then the widget could not be successfully updated
     * to show the properties of \a item.
     */
    bool setItem( QgsAnnotationItem *item );

    /**
     * Sets the \a context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     * \see context()
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

  public slots:

    /**
     * Focuses the default widget for the page.
     */
    virtual void focusDefaultWidget();

  signals:

    /**
     * Emitted when the annotation item definition in the widget is changed by the user.
     */
    void itemChanged();

  protected:

    /**
     * Attempts to update the widget to show the properties
     * for the specified \a item.
     *
     * Subclasses can override this if they support changing items in place.
     *
     * Implementations must return TRUE if the item was accepted and
     * the widget was updated.
     */
    virtual bool setNewItem( QgsAnnotationItem *item );

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};

#endif // QGSANNOTATIONITEMWIDGET_H
