/***************************************************************************
  qgsmesh3dsymbolwidget.h
  -----------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESH3DSYMBOLWIDGET_H
#define QGSMESH3DSYMBOLWIDGET_H

#include <QWidget>

#include "qgsmesh3dsymbol.h"
#include "ui_qgsmesh3dpropswidget.h"

class QgsMeshDatasetGroupListModel;
class QgsMesh3DSymbol;
class QgsMeshLayer;

//! A widget for configuration of 3D symbol for meshes
class QgsMesh3DSymbolWidget : public QWidget, private Ui::QgsMesh3dPropsWidget
{
    Q_OBJECT
  public:
    explicit QgsMesh3DSymbolWidget( QgsMeshLayer *meshLayer, QWidget *parent = nullptr );

    std::unique_ptr<QgsMesh3DSymbol> symbol() const;

    void setLayer( QgsMeshLayer *meshLayer, bool updateSymbol = true );
    QgsMeshLayer *meshLayer() const;
    void setSymbol( const QgsMesh3DSymbol *symbol );

    void configureForTerrain();
    void configureForDataset();

  public slots:
    void reloadColorRampShaderMinMax();
    void enableVerticalSetting( bool isEnable );
    void enableArrowSettings( bool isEnable );

  signals:
    void changed();

  private slots:

    void onColorRampShaderMinMaxChanged();
    void onColoringTypeChanged();
    void onTextureSettingsCollapseStateChanged( bool collapsed );

  private:
    double lineEditValue( const QLineEdit *lineEdit ) const;
    void setColorRampMinMax( double min, double max );
    QgsMeshLayer *mLayer = nullptr;
    QgsMeshDatasetGroupListModel *mDatasetGroupListModel = nullptr;
    std::unique_ptr<QgsMesh3DSymbol> mSymbol;
};

#endif // QGSMESH3DSYMBOLWIDGET_H
