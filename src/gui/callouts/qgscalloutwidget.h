/***************************************************************************
    qgscalloutwidget.h
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCALLOUTWIDGET_H
#define QGSCALLOUTWIDGET_H

#include "qgspropertyoverridebutton.h"
#include "qgis_sip.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"
#include <QWidget>
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMapCanvas;
class QgsCallout;

/**
 * \ingroup gui
 * \class QgsCalloutWidget
 */
class GUI_EXPORT QgsCalloutWidget : public QWidget, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCalloutWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsCalloutWidget( QWidget *parent SIP_TRANSFERTHIS, QgsVectorLayer *vl = nullptr )
      : QWidget( parent )
      , mVectorLayer( vl )
    {}

    virtual void setCallout( QgsCallout *callout ) = 0;
    virtual QgsCallout *callout() = 0;

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     * \since QGIS 3.0
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     * \since QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     * \since QGIS 2.12
     */
    const QgsVectorLayer *vectorLayer() const { return mVectorLayer; }

  protected:

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     * \since QGIS 3.0
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsVectorLayer *mVectorLayer = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;

  signals:

    /**
     * Should be emitted whenever configuration changes happened on this symbol layer configuration.
     * If the subsymbol is changed, symbolChanged() should be emitted instead.
     */
    void changed();

  protected slots:
    void updateDataDefinedProperty();

  private slots:
    void createAuxiliaryField();

  private:
    QgsSymbolWidgetContext mContext;
};

#endif // QGSCALLOUTWIDGET_H
