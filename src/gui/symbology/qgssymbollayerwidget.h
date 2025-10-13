/***************************************************************************
 qgssymbollayerwidget.h - symbol layer widgets

 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERWIDGET_H
#define QGSSYMBOLLAYERWIDGET_H

#include "qgspropertyoverridebutton.h"
#include "qgis_sip.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"

#include <QWidget>
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMarkerSymbol;
class QgsLineSymbol;

/**
 * \ingroup gui
 * \class QgsSymbolLayerWidget
 * \brief Abstract base class for widgets used to configure QgsSymbolLayer classes.
 */
class GUI_EXPORT QgsSymbolLayerWidget : public QWidget, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSymbolLayerWidget( QWidget *parent SIP_TRANSFERTHIS, QgsVectorLayer *vl = nullptr )
      : QWidget( parent )
      , mVectorLayer( vl )
    {}

    virtual void setSymbolLayer( QgsSymbolLayer *layer ) = 0;
    virtual QgsSymbolLayer *symbolLayer() = 0;

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

  protected:
    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsVectorLayer *mVectorLayer = nullptr;

  signals:

    /**
     * Should be emitted whenever configuration changes happened on this symbol layer configuration.
     * If the subsymbol is changed, symbolChanged() should be emitted instead.
     */
    void changed();

    /**
     * Should be emitted whenever the sub symbol changed on this symbol layer configuration.
     * Normally changed() should be preferred.
     *
     * \see changed()
     */
    void symbolChanged();

  protected slots:
    void updateDataDefinedProperty();

  private slots:
    void createAuxiliaryField();

  private:
    QgsSymbolWidgetContext mContext;
};

#endif
