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
#ifndef QGSRENDERERV2WIDGET_H
#define QGSRENDERERV2WIDGET_H

#include <QWidget>
#include <QMenu>
#include <QStackedWidget>
#include "qgssymbol.h"
#include "qgspanelwidget.h"

class QgsVectorLayer;
class QgsStyle;
class QgsFeatureRenderer;
class QgsSymbolSelectorDialog;
class QgsMapCanvas;

/** \ingroup gui
  Base class for renderer settings widgets

WORKFLOW:
- open renderer dialog with some RENDERER  (never null!)
- find out which widget to use
- instantiate it and set in stacked widget
- on any change of renderer type, create some default (dummy?) version and change the stacked widget
- when clicked ok/apply, get the renderer from active widget and clone it for the layer
*/
class GUI_EXPORT QgsRendererWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    QgsRendererWidget( QgsVectorLayer* layer, QgsStyle* style );

    virtual ~QgsRendererWidget() {}

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRenderer* renderer() = 0;

    //! show a dialog with renderer's symbol level settings
    void showSymbolLevelsDialog( QgsFeatureRenderer* r );

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const { return mLayer; }

    /**
     * This method should be called whenever the renderer is actually set on the layer.
     */
    void applyChanges();

  signals:
    /**
     * Emitted when expression context variables on the associated
     * vector layers have been changed. Will request the parent dialog
     * to re-synchronize with the variables.
     */
    void layerVariablesChanged();

  protected:
    QgsVectorLayer* mLayer;
    QgsStyle* mStyle;
    QMenu* contextMenu;
    QAction* mCopyAction;
    QAction* mPasteAction;
    QgsMapCanvas* mMapCanvas;

    /** Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
      and by connecting the slot contextMenuViewCategories(const QPoint&)*/
    virtual QList<QgsSymbol*> selectedSymbols() { return QList<QgsSymbol*>(); }
    virtual void refreshSymbolView() {}

  protected slots:
    void  contextMenuViewCategories( QPoint p );
    /** Change color of selected symbols*/
    void changeSymbolColor();
    /** Change opacity of selected symbols*/
    void changeSymbolTransparency();
    /** Change units mm/map units of selected symbols*/
    void changeSymbolUnit();
    /** Change line widths of selected symbols*/
    void changeSymbolWidth();
    /** Change marker sizes of selected symbols*/
    void changeSymbolSize();
    /** Change marker angles of selected symbols*/
    void changeSymbolAngle();

    virtual void copy() {}
    virtual void paste() {}

  private:
    /**
     * This will be called whenever the renderer is set on a layer.
     * This can be overwritten in subclasses.
     */
    virtual void apply();

};


////////////

#include <QObject>

class QMenu;
class QgsField;
class QgsFields;

#include "ui_widget_set_dd_value.h"
#include "qgssizescalewidget.h"

/** \ingroup gui
Utility classes for "en masse" size definition
*/
class GUI_EXPORT QgsDataDefinedValueDialog : public QDialog, public Ui::QgsDataDefinedValueDialog, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /** Constructor
     * @param symbolList must not be empty
     * @param layer must not be null
     * @param label value label
     */
    QgsDataDefinedValueDialog( const QList<QgsSymbol*>& symbolList, QgsVectorLayer * layer, const QString & label );
    virtual ~QgsDataDefinedValueDialog() {}

    /** Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const { return mLayer; }

  public slots:
    void dataDefinedChanged();

  protected:
    /**
     * Should be called in the constructor of child classes.
     *
     * @note May be missing Python bindings depending on the platform.
     */
    void init( const QString& description ); // needed in children ctor to call virtual

  private:
    QgsDataDefined symbolDataDefined() const;

    virtual QgsDataDefined symbolDataDefined( const QgsSymbol* ) const = 0;
    virtual double value( const QgsSymbol* ) const = 0;
    virtual void setDataDefined( QgsSymbol* symbol, const QgsDataDefined& dd ) = 0;

    QList<QgsSymbol*> mSymbolList;
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mMapCanvas;

    QgsExpressionContext createExpressionContext() const override;
};

/** \ingroup gui
 * \class QgsDataDefinedSizeDialog
 */
class GUI_EXPORT QgsDataDefinedSizeDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedSizeDialog( const QList<QgsSymbol*>& symbolList, QgsVectorLayer * layer )
        : QgsDataDefinedValueDialog( symbolList, layer, tr( "Size" ) )
    {
      init( tr( "Symbol size" ) );
      if ( !symbolList.isEmpty() && symbolList.at( 0 ) && vectorLayer() )
        mDDBtn->setAssistant( tr( "Size Assistant..." ), new QgsSizeScaleWidget( vectorLayer(), static_cast<const QgsMarkerSymbol*>( symbolList.at( 0 ) ) ) );
    }

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbol * symbol ) const override;

    double value( const QgsSymbol * symbol ) const override { return static_cast<const QgsMarkerSymbol*>( symbol )->size(); }

    void setDataDefined( QgsSymbol* symbol, const QgsDataDefined& dd ) override;
};

/** \ingroup gui
 * \class QgsDataDefinedRotationDialog
 */
class GUI_EXPORT QgsDataDefinedRotationDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedRotationDialog( const QList<QgsSymbol*>& symbolList, QgsVectorLayer * layer )
        : QgsDataDefinedValueDialog( symbolList, layer, tr( "Rotation" ) )
    {
      init( tr( "Symbol rotation" ) );
    }

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbol * symbol ) const override;

    double value( const QgsSymbol * symbol ) const override { return static_cast<const QgsMarkerSymbol*>( symbol )->angle(); }

    void setDataDefined( QgsSymbol* symbol, const QgsDataDefined& dd ) override;
};

/** \ingroup gui
 * \class QgsDataDefinedWidthDialog
 */
class GUI_EXPORT QgsDataDefinedWidthDialog : public QgsDataDefinedValueDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedWidthDialog( const QList<QgsSymbol*>& symbolList, QgsVectorLayer * layer )
        : QgsDataDefinedValueDialog( symbolList, layer, tr( "Width" ) )
    {
      init( tr( "Symbol width" ) );
    }

  protected:
    QgsDataDefined symbolDataDefined( const QgsSymbol * symbol ) const override;

    double value( const QgsSymbol * symbol ) const override { return static_cast<const QgsLineSymbol*>( symbol )->width(); }

    void setDataDefined( QgsSymbol* symbol, const QgsDataDefined& dd ) override;
};



#endif // QGSRENDERERV2WIDGET_H
