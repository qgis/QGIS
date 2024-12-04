/***************************************************************************
    qgsrendererwidget.h
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
#ifndef QGSRENDERERWIDGET_H
#define QGSRENDERERWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QStackedWidget>
#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsrenderer.h"
#include "qgsexpressioncontextgenerator.h"

class QgsDataDefinedSizeLegend;
class QgsDataDefinedSizeLegendWidget;
class QgsVectorLayer;
class QgsStyle;
class QgsMapCanvas;
class QgsMarkerSymbol;
class QgsLegendSymbolItem;
class QgsPropertyOverrideButton;

/**
 * \ingroup gui
 * \brief Base class for renderer settings widgets.
 *
 * WORKFLOW:
 *
 * - open renderer dialog with some RENDERER  (never null!)
 * - find out which widget to use
 * - instantiate it and set in stacked widget
 * - on any change of renderer type, create some default (dummy?) version and change the stacked widget
 * - when clicked OK/Apply, get the renderer from active widget and clone it for the layer
*/
class GUI_EXPORT QgsRendererWidget : public QgsPanelWidget, public QgsExpressionContextGenerator
{
    Q_OBJECT
  public:
    QgsRendererWidget( QgsVectorLayer *layer, QgsStyle *style );
    QgsExpressionContext createExpressionContext() const override;

    //! Returns pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRenderer *renderer() = 0;

    /**
     * Show a dialog with renderer's symbol level settings.
     */
    void showSymbolLevelsDialog( QgsFeatureRenderer *r );

    /**
     * Sets the context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the renderer widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     */
    const QgsVectorLayer *vectorLayer() const { return mLayer; }

    /**
     * This method should be called whenever the renderer is actually set on the layer.
     */
    void applyChanges();

    void setDockMode( bool dockMode ) override;

    /**
     * Disables symbol level modification on the widget.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.20
     */
    virtual void disableSymbolLevels() SIP_SKIP;

  signals:

    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

    /**
     * Emitted when the symbol levels settings have been changed.
     *
     * \deprecated QGIS 3.20. No longer emitted.
     */
    Q_DECL_DEPRECATED void symbolLevelsChanged() SIP_DEPRECATED;

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QgsStyle *mStyle = nullptr;
    QMenu *contextMenu = nullptr;
    QAction *mCopyAction = nullptr;
    QAction *mPasteAction = nullptr;

    /**
     * Copy symbol action.
     * \since QGIS 3.10
     */
    QAction *mCopySymbolAction = nullptr;

    /**
     * Paste symbol action.
     * \since QGIS 3.10
     */
    QAction *mPasteSymbolAction = nullptr;

    //! Context in which widget is shown
    QgsSymbolWidgetContext mContext;

    /**
     * Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
     * and by connecting the slot contextMenuViewCategories(const QPoint&).
    */
    virtual QList<QgsSymbol *> selectedSymbols() { return QList<QgsSymbol *>(); }
    virtual void refreshSymbolView() {}

    /**
     * Creates widget to setup data-defined size legend.
     * Returns newly created panel - may be NULLPTR if it could not be opened. Ownership is transferred to the caller.
     */
    QgsDataDefinedSizeLegendWidget *createDataDefinedSizeLegendWidget( const QgsMarkerSymbol *symbol, const QgsDataDefinedSizeLegend *ddsLegend ) SIP_FACTORY;

    /**
     * Sets the symbol levels for the renderer defined in the widget.
     *
     * The \a levels argument defines the updated list of symbols with rendering passes set.
     *
     * The \a enabled arguments specifies if symbol levels should be enabled for the renderer.
     *
     * \since QGIS 3.20
     */
    virtual void setSymbolLevels( const QList<QgsLegendSymbolItem> &levels, bool enabled );

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsFeatureRenderer::Property key );

  protected slots:
    void contextMenuViewCategories( QPoint p );
    //! Change color of selected symbols
    void changeSymbolColor();
    //! Change opacity of selected symbols
    void changeSymbolOpacity();
    //! Change units mm/map units of selected symbols
    void changeSymbolUnit();
    //! Change line widths of selected symbols
    void changeSymbolWidth();
    //! Change marker sizes of selected symbols
    void changeSymbolSize();
    //! Change marker angles of selected symbols
    void changeSymbolAngle();


    virtual void copy() {}
    virtual void paste() {}

    /**
      * Pastes the clipboard symbol over selected items.
      *
      * \since QGIS 3.10
     */
    virtual void pasteSymbolToSelection();

  private slots:

    void copySymbol();
    void updateDataDefinedProperty();

  private:
    /**
     * This will be called whenever the renderer is set on a layer.
     * This can be overwritten in subclasses.
     */
    virtual void apply() SIP_FORCE;
};


////////////

#include <QObject>

class QMenu;
class QgsField;
class QgsFields;

#include "ui_widget_set_dd_value.h"
#include "qgis_gui.h"


/**
 * \ingroup gui
 * \brief Utility classes for "en masse" size definition.
 */
class GUI_EXPORT QgsDataDefinedValueDialog : public QDialog, public Ui::QgsDataDefinedValueBaseDialog, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param symbolList must not be empty
     * \param layer must not be NULLPTR
     * \param label value label
     */
    QgsDataDefinedValueDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer, const QString &label );

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     */
    const QgsVectorLayer *vectorLayer() const { return mLayer; }

  public slots:
    void dataDefinedChanged();

  protected:
    /**
     * Should be called in the constructor of child classes.
     *
     * \note May be missing Python bindings depending on the platform.
     */
    void init( int propertyKey ); // needed in children ctor to call virtual

  private:
    QgsProperty symbolDataDefined() const SIP_FORCE;

    virtual QgsProperty symbolDataDefined( const QgsSymbol * ) const = 0 SIP_FORCE;
    virtual double value( const QgsSymbol * ) const = 0 SIP_FORCE;
    virtual void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd ) = 0 SIP_FORCE;

    QList<QgsSymbol *> mSymbolList;
    QgsVectorLayer *mLayer = nullptr;

    QgsSymbolWidgetContext mContext;

    QgsExpressionContext createExpressionContext() const override;
};

/**
 * \ingroup gui
 * \class QgsDataDefinedSizeDialog
 */
class GUI_EXPORT QgsDataDefinedSizeDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedSizeDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const override;

    double value( const QgsSymbol *symbol ) const override;

    void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd ) override;

  private:
    std::shared_ptr<QgsMarkerSymbol> mAssistantSymbol;
};

/**
 * \ingroup gui
 * \class QgsDataDefinedRotationDialog
 */
class GUI_EXPORT QgsDataDefinedRotationDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedRotationDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const override;

    double value( const QgsSymbol *symbol ) const override;

    void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd ) override;
};

/**
 * \ingroup gui
 * \class QgsDataDefinedWidthDialog
 */
class GUI_EXPORT QgsDataDefinedWidthDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedWidthDialog( const QList<QgsSymbol *> &symbolList, QgsVectorLayer *layer );

  protected:
    QgsProperty symbolDataDefined( const QgsSymbol *symbol ) const override;

    double value( const QgsSymbol *symbol ) const override;

    void setDataDefined( QgsSymbol *symbol, const QgsProperty &dd ) override;
};


#endif // QGSRENDERERWIDGET_H
