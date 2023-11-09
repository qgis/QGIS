/***************************************************************************
  qgspointcloud3dsymbolwidget.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUD3DSYMBOLWIDGET_H
#define QGSPOINTCLOUD3DSYMBOLWIDGET_H

#include "ui_qgspointcloud3dsymbolwidget.h"
#include "qgspointcloud3dsymbol.h"

class QgsPointCloudLayer;
class QgsPointCloud3DSymbol;
class QgsPointCloudClassifiedRendererWidget;

class QgsPointCloud3DSymbolWidget : public QWidget, private Ui::QgsPointCloud3DSymbolWidget
{
    Q_OBJECT

  public:
    explicit QgsPointCloud3DSymbolWidget( QgsPointCloudLayer *layer, QgsPointCloud3DSymbol *symbol, QWidget *parent = nullptr );

    void setSymbol( QgsPointCloud3DSymbol *symbol );

    void setDockMode( bool dockMode );

    QgsPointCloud3DSymbol *symbol() const;

    void setMaximumScreenError( double maxScreenError );
    double maximumScreenError() const;

    void setShowBoundingBoxes( bool showBoundingBoxes );
    bool showBoundingBoxes() const;

    void setPointBudget( double budget );
    double pointBudget() const;

    void connectChildPanels( QgsPanelWidget *parent );

  private slots:
    void reloadColorRampShaderMinMax();
    void onRenderingStyleChanged();
    void emitChangedSignal();
    void rampAttributeChanged();
    void setMinMaxFromLayer();
    void minMaxChanged();

    void mRedMinLineEdit_textChanged( const QString & );
    void mRedMaxLineEdit_textChanged( const QString & );
    void mGreenMinLineEdit_textChanged( const QString & );
    void mGreenMaxLineEdit_textChanged( const QString & );
    void mBlueMinLineEdit_textChanged( const QString & );
    void mBlueMaxLineEdit_textChanged( const QString & );
    void redAttributeChanged();
    void greenAttributeChanged();
    void blueAttributeChanged();


  signals:
    void changed();

  private:
    void setColorRampMinMax( double min, double max );

  private:
    int mBlockChangedSignals = 0;
    int mDisableMinMaxWidgetRefresh = 0;
    QgsPointCloudClassifiedRendererWidget *mClassifiedRendererWidget = nullptr;
    QgsPointCloudLayer *mLayer = nullptr;

    bool mBlockMinMaxChanged = false;
    bool mBlockSetMinMaxFromLayer = false;

    double mProviderMin = std::numeric_limits< double >::quiet_NaN();
    double mProviderMax = std::numeric_limits< double >::quiet_NaN();

    void createValidators();
    void setCustomMinMaxValues( QgsRgbPointCloud3DSymbol *r ) const;
    void minMaxModified();
    void setMinMaxValue( const QgsContrastEnhancement *ce, QLineEdit *minEdit, QLineEdit *maxEdit );

    double mPointBudget = 1000000;
};

#endif // QGSPOINTCLOUD3DSYMBOLWIDGET_H
