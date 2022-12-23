/***************************************************************************
    qgssymbolselectordialog.h
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

#ifndef QGSSYMBOLSELECTORDIALOG_H
#define QGSSYMBOLSELECTORDIALOG_H

#include <QDialog>
#include "qgis_sip.h"

#include "ui_qgssymbolselectordialogbase.h"

#include "qgspanelwidget.h"
#include "qgssymbolwidgetcontext.h"
#include "qgsproperty.h"
#include "qgshelp.h"

#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QPointer>
#include "qgis_gui.h"

class QgsStyle;
class QgsSymbol;
class QgsSymbolLayer;
class QgsVectorLayer;

class QMenu;
class QWidget;

class SymbolLayerItem;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsMarkerSymbolLayer;
class QgsLineSymbolLayer;

class QgsMapCanvas;

#ifndef SIP_RUN
/// @cond PRIVATE

class DataDefinedRestorer: public QObject
{
    Q_OBJECT
  public:
    DataDefinedRestorer( QgsSymbol *symbol, const QgsSymbolLayer *symbolLayer );

  public slots:
    void restore();

  private:
    QgsMarkerSymbol *mMarker = nullptr;
    const QgsMarkerSymbolLayer *mMarkerSymbolLayer = nullptr;
    double mSize;
    double mAngle;
    QPointF mMarkerOffset;
    QgsProperty mDDSize;
    QgsProperty mDDAngle;

    QgsLineSymbol *mLine = nullptr;
    const QgsLineSymbolLayer *mLineSymbolLayer = nullptr;
    double mWidth;
    double mLineOffset;
    QgsProperty mDDWidth;

    void save();
};
///@endcond
#endif

class QgsSymbolSelectorDialog;

/**
 * \ingroup gui
 * \brief Symbol selector widget that can be used to select and build a symbol
 */
class GUI_EXPORT QgsSymbolSelectorWidget: public QgsPanelWidget, private Ui::QgsSymbolSelectorDialogBase
{
    Q_OBJECT
    /// To allow for non API break access from the dialog.
    friend class QgsSymbolSelectorDialog;

  public:

    // TODO QGIS 4.0 - transfer ownership of symbol to widget!

    /**
     * Symbol selector widget that can be used to select and build a symbol
     * \param symbol The symbol to load into the widget as a start point.
     * \param style The style used by the widget.
     * \param vl The vector layer for the symbol.
     * \param parent
     * \note The ownership of the symbol is not transferred and must exist for the lifetime of the widget.
     */
    QgsSymbolSelectorWidget( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu *advancedMenu();

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     * \since QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     * \since QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the symbol that is currently active in the widget. Can be NULLPTR.
     * \returns The active symbol.
     */
    QgsSymbol *symbol() { return mSymbol; }

    // TODO QGIS 4.0 - transfer ownership of symbol to widget!

    /**
     * Loads the given symbol into the widget.
     * \param symbol The symbol to load.
     * \param parent The parent symbol layer item. If the parent parameter is null, the whole symbol and model will be reset.
     * \note The ownership of the symbol is not transferred and must exist for the lifetime of the widget.
     */
    void loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent = nullptr ) SIP_SKIP;

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
     * Add a symbol layer to the bottom of the stack.
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
     * Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
     * \since QGIS 2.14
     */
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
     * Update the preview of the whole symbol in the interface.
     */
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();

    /**
     * alters tree and sets proper widget when Layer Type is changed
     * \note: The layer is received from the LayerPropertiesWidget
     */
    void changeLayer( QgsSymbolLayer *layer );

  signals:

    /**
     * Emitted when a symbol is modified in the widget.
     */
    void symbolModified();

  private slots:

    /**
     * Called when project data (such as project colors, image cache) has changed. Updates
     * the symbol preview to take changes into account.
     */
    void projectDataChanged();

    /**
     * Called when layers are about to be removed from the project.
     */
    void layersAboutToBeRemoved( const QList<QgsMapLayer *> &layers );

  private:

    /**
     * Reload the current symbol in the view.
     */
    void reloadSymbol();

    /**
     * Update the state of the UI based on the currently set symbol layer.
     */
    void updateUi();

    /**
     * Update the lock button states based on the current symbol layer.
     */
    void updateLockButton();

    SymbolLayerItem *currentLayerItem();

    /**
     * The current symbol layer that is active in the interface.
     * \returns The active symbol layer.
     */
    QgsSymbolLayer *currentLayer();

    /**
     * Move the current active layer by a set offset in the list.
     * \param offset The offset to move the layer by
     */
    void moveLayerByOffset( int offset );

    /**
     * Set the properties widget for the active symbol layer.
     * \param widget The widget to set to configure the active symbol layer.
     */
    void setWidget( QWidget *widget );

    QgsStyle *mStyle = nullptr;
    QgsSymbol *mSymbol = nullptr;
    QMenu *mAdvancedMenu = nullptr;
    QPointer< QgsVectorLayer > mVectorLayer;

    QStandardItemModel *mSymbolLayersModel = nullptr;
    QWidget *mPresentWidget = nullptr;

    std::unique_ptr<DataDefinedRestorer> mDataDefineRestorer;
    QgsSymbolWidgetContext mContext;
    QgsFeature mPreviewFeature;
    QgsExpressionContext mPreviewExpressionContext;
    bool mBlockModified = false;

};

/**
 * \ingroup gui
 * \class QgsSymbolSelectorDialog
 */
class GUI_EXPORT QgsSymbolSelectorDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSymbolSelectorDialog.
     *
     * \param symbol The symbol
     * \param style The style
     * \param vl Associated vector layer
     * \param parent Parent widget
     * \param embedded TRUE to embed in renderer properties dialog, FALSE otherwise
     */
    QgsSymbolSelectorDialog( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr, bool embedded = false );

    //! Returns menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu *advancedMenu();

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     * \since QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     * \since QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the symbol that is currently active in the widget. Can be NULLPTR.
     * \returns The active symbol.
     */
    QgsSymbol *symbol();

    /**
     * Loads the given symbol into the widget.
     * \param symbol The symbol to load.
     * \param parent The parent symbol layer item. If the parent parameter is null, the whole symbol and model will be reset.
     */
    void loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent = nullptr ) SIP_SKIP;

    /**
     * Returns a reference to the dialog's button box.
     * \since QGIS 3.10
     */
    QDialogButtonBox *buttonBox() const;

  public slots:

    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void lockLayer();

    /**
     * Duplicates the current symbol layer and places the duplicated layer above the current symbol layer
     * \since QGIS 2.14
     */
    void duplicateLayer();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

    //! Slot to update tree when a new symbol from style
    void symbolChanged();

    /**
     * alters tree and sets proper widget when Layer Type is changed
     * \note: The layer is received from the LayerPropertiesWidget
     */
    void changeLayer( QgsSymbolLayer *layer );

  protected:

    // Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent *e ) override;

  private slots:

    void showHelp();

  signals:

    void symbolModified();

  private:

    void reloadSymbol();

    void updateUi();

    void updateLockButton();

    SymbolLayerItem *currentLayerItem();

    QgsSymbolLayer *currentLayer();

    void moveLayerByOffset( int offset );

    void setWidget( QWidget *widget );

    QgsSymbolSelectorWidget *mSelectorWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QgsSymbolWidgetContext mContext;

};

#endif
