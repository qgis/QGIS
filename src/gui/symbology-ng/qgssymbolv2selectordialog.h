/***************************************************************************
    qgssymbolv2selectordialog.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
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

#include <QStandardItemModel>

class QgsStyleV2;
class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorLayer;

class QMenu;
class QWidget;

class SymbolLayerItem;

class GUI_EXPORT QgsSymbolV2SelectorDialog : public QDialog, private Ui::QgsSymbolV2SelectorDialogBase
{
    Q_OBJECT

  public:
    QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent = NULL, bool embedded = false );

    //! return menu for "advanced" button - create it if doesn't exist and show the advanced button
    QMenu* advancedMenu();

  protected:
    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol();
    void loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent );

    void populateLayerTypes( QgsSymbolV2* symbol );

    void updateUi();

    void updateLockButton();

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
};

#endif
