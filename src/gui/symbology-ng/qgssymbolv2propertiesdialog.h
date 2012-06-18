/***************************************************************************
    qgssymbolv2propertiesdialog.h
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

#ifndef QGSSYMBOLV2PROPERTIESDIALOG_H
#define QGSSYMBOLV2PROPERTIESDIALOG_H

#include "ui_qgssymbolv2propertiesdialogbase.h"

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsSymbolLayerV2Widget;
class QgsVectorLayer;

class SymbolLayerItem;

#include <QMap>
#include <QStandardItemModel>


class GUI_EXPORT QgsSymbolV2PropertiesDialog : public QDialog, private Ui::DlgSymbolV2Properties
{
    Q_OBJECT

  public:
    QgsSymbolV2PropertiesDialog( QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent = NULL );


  public slots:
    void moveLayerDown();
    void moveLayerUp();

    void addLayer();
    void removeLayer();

    void lockLayer();

    void layerTypeChanged();

    void layerChanged();

    void updateLayerPreview();
    void updatePreview();

  protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

    void loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent );
    void loadSymbol();

    void populateLayerTypes( QgsSymbolV2* symbol );

    void updateUi();

    //void loadPropertyWidgets();

    void updateSymbolLayerWidget( QgsSymbolLayerV2* layer );
    void updateLockButton();

    SymbolLayerItem* currentLayerItem();
    QgsSymbolLayerV2* currentLayer();

    void moveLayerByOffset( int offset );

  protected: // data
    QgsSymbolV2* mSymbol;

    QMap<QString, QgsSymbolLayerV2Widget*> mWidgets;

    const QgsVectorLayer* mVectorLayer;

    QStandardItemModel* model;
};

#endif
