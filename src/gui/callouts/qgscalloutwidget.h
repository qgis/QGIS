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
#include "qgscallout.h"
#include <QWidget>
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsCalloutWidget
 * Base class for widgets which allow control over the properties of callouts.
 * \since QGIS 3.10
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

    /**
     * Sets the \a callout to show in the widget. Ownership is not transferred.
     * \see callout()
     */
    virtual void setCallout( QgsCallout *callout ) = 0;

    /**
     * Returns the callout defined by the current settings in the widget. Ownership is not transferred,
     * and the caller should clone the returned value.
     * \see setCallout()
     */
    virtual QgsCallout *callout() = 0;

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     */
    const QgsVectorLayer *vectorLayer() const { return mVectorLayer; }

    /**
     * Sets the geometry \a type of the features to customize the widget accordingly.
     */
    virtual void setGeometryType( QgsWkbTypes::GeometryType type ) = 0;

  protected:

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsCallout::Property key );

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

  private slots:
    void updateDataDefinedProperty();

    void createAuxiliaryField();

  private:
    QgsSymbolWidgetContext mContext;
};


///////////

#include "ui_widget_simplelinecallout.h"

class QgsSimpleLineCallout;

#ifndef SIP_RUN
///@cond PRIVATE

class GUI_EXPORT QgsSimpleLineCalloutWidget : public QgsCalloutWidget, private Ui::WidgetSimpleLineCallout
{
    Q_OBJECT

  public:

    QgsSimpleLineCalloutWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsCalloutWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSimpleLineCalloutWidget( vl ); }

    void setCallout( QgsCallout *callout ) override;

    QgsCallout *callout() override;

    void setGeometryType( QgsWkbTypes::GeometryType type ) override;

  private slots:

    void minimumLengthChanged();
    void minimumLengthUnitWidgetChanged();
    void offsetFromAnchorUnitWidgetChanged();
    void offsetFromAnchorChanged();
    void offsetFromLabelUnitWidgetChanged();
    void offsetFromLabelChanged();
    void lineSymbolChanged();
    void mAnchorPointComboBox_currentIndexChanged( int index );
    void drawToAllPartsToggled( bool active );

  private:
    std::unique_ptr< QgsSimpleLineCallout > mCallout;

};

class GUI_EXPORT QgsManhattanLineCalloutWidget : public QgsSimpleLineCalloutWidget
{
    Q_OBJECT

  public:

    QgsManhattanLineCalloutWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsCalloutWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsManhattanLineCalloutWidget( vl ); }

};

#endif
///@endcond

#endif // QGSCALLOUTWIDGET_H
