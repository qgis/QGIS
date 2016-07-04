/***************************************************************************
    qgssymbolv2selectordialog.h
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

#ifndef QGSSYMBOLV2SELECTORDIALOG_H
#define QGSSYMBOLV2SELECTORDIALOG_H

#include <QDialog>

#include "ui_qgssymbolv2selectordialogbase.h"

#include "qgsdatadefined.h"
#include "qgspanelwidget.h"

#include <QStandardItemModel>
#include <QScopedPointer>
#include <QDialogButtonBox>

class QgsStyleV2;
class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorLayer;

class QMenu;
class QWidget;

class SymbolLayerItem;
class QgsMarkerSymbolV2;
class QgsLineSymbolV2;
class QgsMarkerSymbolLayerV2;
class QgsLineSymbolLayerV2;

class QgsMapCanvas;

/// @cond PRIVATE

class DataDefinedRestorer: public QObject
{
    Q_OBJECT
  public:
    DataDefinedRestorer( QgsSymbolV2* symbol, const QgsSymbolLayerV2* symbolLayer );

  public slots:
    void restore();

  private:
    QgsMarkerSymbolV2* mMarker;
    const QgsMarkerSymbolLayerV2* mMarkerSymbolLayer;
    double mSize;
    double mAngle;
    QPointF mMarkerOffset;
    QgsDataDefined mDDSize;
    QgsDataDefined mDDAngle;

    QgsLineSymbolV2* mLine;
    const QgsLineSymbolLayerV2* mLineSymbolLayer;
    double mWidth;
    double mLineOffset;
    QgsDataDefined mDDWidth;

    void save();
};
///@endcond

class QgsSymbolV2SelectorDialog;

/** \ingroup gui
 * Symbol selector widget that cna be used to select and build a symbol
 */
class GUI_EXPORT QgsSymbolV2SelectorWidget: public QgsPanelWidget, private Ui::QgsSymbolV2SelectorDialogBase
{
    Q_OBJECT
    /// Too allow for non API break access from the dialog.
    friend class QgsSymbolV2SelectorDialog;

  public:
    /**
       * Symbol selector widget that can be used to select and build a symbol
       * @param symbol The symbol to load into the widget as a start point.
       * @param style The style used by the widget.
       * @param vl The vector layer for the symbol.
       * @param parent
       */
    QgsSymbolV2SelectorWidget( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = nullptr );
    ~QgsSymbolV2SelectorWidget();

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is transferred to the dialog.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context );

    /** Returns the expression context used for the dialog, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the dialog.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const { return mPresetExpressionContext.data(); }

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * @brief Return the symbol that is currently active in the widget. Can be null.
     * @return The active symbol.
     */
    QgsSymbolV2* symbol() { return mSymbol; }

  protected:

    /**
     * Reload the current symbol in the view.
     */
    void loadSymbol();
    //! @note not available in python bindings

    /**
     * Load the given symbol into the widget..
     * @param symbol The symbol to load.
     * @param parent The parent symbol layer item.
     */
    void loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent );

    /**
     * Update the state of the UI based on the currently set symbol layer.
     */
    void updateUi();

    /**
     * Update the lock button states based on the current symbol layer.
     */
    void updateLockButton();

    //! @note not available in python bindings
    SymbolLayerItem* currentLayerItem();

    /**
     * The current symbol layer that is active in the interface.
     * @return The active symbol layer.
     */
    QgsSymbolLayerV2* currentLayer();

    /**
     * Move the current active layer by a set offset in the list.
     * @param offset The offset to move the layer by
     */
    void moveLayerByOffset( int offset );

    /**
     * Set the properties widget for the active symbol layer.
     * @param widget The widget to set to configure the active symbol layer.
     */
    void setWidget( QWidget* widget );

  signals:
    /**
     * Emiited when a symbol is modified in the widget.
     */
    void symbolModified();

  public slots:
    /**
     * Move the active symbol layer down.
     */
    void moveLayerDown();

    /**
     * Move the active symbol layer up.
     */
    void moveLayerUp();

    /**
     * Add a symobl layer to the bottom of the stack.
     */
    void addLayer();

    /**
     * Remove the current active symbol layer.
     */
    void removeLayer();

    /**
     * Lock the current active symbol layer.
     */
    void lockLayer();

    /**
     * Save the current active symbol layer into the users saved styles.
     */
    Q_DECL_DEPRECATED void saveSymbol();

    //! Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
    //! @note added in QGIS 2.14
    void duplicateLayer();

    /**
     * Called when the layer changes in the widget. Updates the active properties for
     * active symbol layer.
     */
    void layerChanged();

    /**
     * Update the single symbol layer preview in the widget.
     */
    void updateLayerPreview();

    /**
     * Update the preivew of the whole symbol in the iterface.
     */
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();
    //! alters tree and sets proper widget when Layer Type is changed
    //! @note: The layer is received from the LayerPropertiesWidget
    void changeLayer( QgsSymbolLayerV2* layer );


  protected: // data
    QgsStyleV2* mStyle;
    QgsSymbolV2* mSymbol;
    QMenu* mAdvancedMenu;
    const QgsVectorLayer* mVectorLayer;

    QStandardItemModel* model;
    QWidget *mPresentWidget;

  private:
    QScopedPointer<DataDefinedRestorer> mDataDefineRestorer;
    QScopedPointer< QgsExpressionContext > mPresetExpressionContext;

    QgsMapCanvas* mMapCanvas;
};

/** \ingroup gui
 * \class QgsSymbolV2SelectorDialog
 */
class GUI_EXPORT QgsSymbolV2SelectorDialog : public QDialog
{
    Q_OBJECT

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = nullptr, bool embedded = false );
    ~QgsSymbolV2SelectorDialog();

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is transferred to the dialog.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context );

    /** Returns the expression context used for the dialog, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the dialog.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const;

    /** Sets the map canvas associated with the dialog. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /**
     * @brief Return the symbol that is currently active in the widget. Can be null.
     * @return The active symbol.
     */
    QgsSymbolV2* symbol();

  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * e ) override;

    void loadSymbol();
    //! @note not available in python bindings
    void loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent );

    void updateUi();

    void updateLockButton();

    //! @note not available in python bindings
    SymbolLayerItem* currentLayerItem();
    QgsSymbolLayerV2* currentLayer();

    void moveLayerByOffset( int offset );

    void setWidget( QWidget* widget );

  signals:
    void symbolModified();

  public slots:
    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void lockLayer();

    Q_DECL_DEPRECATED void saveSymbol();

    //! Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
    //! @note added in QGIS 2.14
    void duplicateLayer();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();
    //! alters tree and sets proper widget when Layer Type is changed
    //! @note: The layer is received from the LayerPropertiesWidget
    void changeLayer( QgsSymbolLayerV2* layer );

  private:
    QgsSymbolV2SelectorWidget* mSelectorWidget;
    QDialogButtonBox* mButtonBox;
};

#endif
