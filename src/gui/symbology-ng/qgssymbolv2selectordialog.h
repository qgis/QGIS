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

#include <QStandardItemModel>
#include <QScopedPointer>

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

class GUI_EXPORT QgsSymbolV2SelectorDialog : public QDialog, private Ui::QgsSymbolV2SelectorDialogBase
{
    Q_OBJECT

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = 0, bool embedded = false );

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

  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event ) override;

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

    void saveSymbol();
    void lockLayer();

    void layerChanged();

    void updateLayerPreview();
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
};

#endif
