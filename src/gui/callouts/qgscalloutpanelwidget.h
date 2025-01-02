/***************************************************************************
    qgscalloutpanelwidget.h
    ---------------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCALLOUTPANELWIDGET_H
#define QGSCALLOUTPANELWIDGET_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"
#include "ui_qgscalloutpanelwidget.h"

#include <QPointer>

class QgsCallout;
class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsCalloutPanelWidget
 * \brief A panel widget for configuration of callouts.
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCalloutPanelWidget : public QgsPanelWidget, private Ui::QgsCalloutPanelWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsCalloutPanelWidget, with the specified \a parent widget
     *
     * The optional \a layer argument can be set to an associated map layer.
     */
    QgsCalloutPanelWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsMapLayer *layer = nullptr );

    /**
     * Sets the geometry \a type for the objects associated with the callouts.
     *
     * If \a type is Qgis::GeometryType::Unknown then an attempt will be made to
     * deduce the type from the associated map layer.
     *
     * \see geometryType()
    */
    void setGeometryType( Qgis::GeometryType type );

    /**
     * Returns the geometry type for the objects associated with the callouts.
     *
     * If the type is Qgis::GeometryType::Unknown then an attempt will be made to
     * deduce the type from the associated map layer.
     *
     * \see setGeometryType()
    */
    Qgis::GeometryType geometryType() const { return mGeometryType; }

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

    /**
     * Sets the widget state to match the specified \a callout.
     *
     * Ownership is not transferred.
     *
     * \see callout()
     */
    void setCallout( const QgsCallout *callout );

    /**
     * Returns a new callout, respecting the configuration from the widget.
     *
     * \see setCallout()
     */
    QgsCallout *callout() SIP_FACTORY;

  signals:

    //! Emitted when the callout defined by the widget changes
    void calloutChanged();

  private slots:

    void calloutTypeChanged();
    void updateCalloutWidget( const QgsCallout *callout );

  private:
    QPointer<QgsMapLayer> mLayer;
    Qgis::GeometryType mGeometryType = Qgis::GeometryType::Unknown;

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;
};

#endif // QGSCALLOUTPANELWIDGET_H
