/***************************************************************************
                    qgsannotationitemcommonpropertieswidget.h
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
#ifndef QGSANNOTATIONITEMCOMMONPROPERTIESWIDGET_H
#define QGSANNOTATIONITEMCOMMONPROPERTIESWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "ui_qgsannotationcommonpropertieswidgetbase.h"
#include "qgssymbolwidgetcontext.h"

class QgsAnnotationItem;

/**
 * \class QgsAnnotationItemCommonPropertiesWidget
 * \ingroup gui
 *
 * \brief A widget for configuring common properties for QgsAnnotationItems
 *
 * \since QGIS 3.22
*/
class GUI_EXPORT QgsAnnotationItemCommonPropertiesWidget: public QWidget, private Ui::QgsAnnotationCommonPropertiesWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAnnotationItemCommonPropertiesWidget.
     */
    QgsAnnotationItemCommonPropertiesWidget( QWidget *parent SIP_TRANSFERTHIS );

    /**
     * Sets the \a item whose properties should be shown in the widget.
     */
    void setItem( QgsAnnotationItem *item );

    /**
     * Updates an \a item, setting the properties defined in the widget.
     */
    void updateItem( QgsAnnotationItem *item );

    /**
     * Sets the \a context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     * \see context()
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

  signals:

    /**
     * Emitted when the annotation item definition in the widget is changed by the user.
     */
    void itemChanged();

  private:

    bool mBlockChangedSignal = false;

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};

#endif // QGSANNOTATIONITEMCOMMONPROPERTIESWIDGET_H
