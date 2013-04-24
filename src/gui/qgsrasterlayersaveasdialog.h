/***************************************************************************
    qgsrasterlayersaveasdialog.h
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLAYERSAVEASDIALOG_H
#define QGSRASTERLAYERSAVEASDIALOG_H

#include "ui_qgsrasterlayersaveasdialogbase.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasternuller.h"

class QgsRasterLayer;
class QgsRasterDataProvider;
class QgsRasterFormatOptionsWidget;

class GUI_EXPORT QgsRasterLayerSaveAsDialog: public QDialog, private Ui::QgsRasterLayerSaveAsDialogBase
{
    Q_OBJECT
  public:
    enum Mode
    {
      RawDataMode,
      RenderedImageMode
    };
    enum CrsState
    {
      OriginalCrs,
      CurrentCrs,
      UserCrs
    };
    enum ExtentState
    {
      OriginalExtent,
      CurrentExtent,
      UserExtent,
    };
    enum ResolutionState
    {
      OriginalResolution,
      UserResolution
    };

    QgsRasterLayerSaveAsDialog( QgsRasterLayer* rasterLayer,
                                QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent,
                                const QgsCoordinateReferenceSystem& layerCrs, const QgsCoordinateReferenceSystem& currentCrs,
                                QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterLayerSaveAsDialog();

    Mode mode() const;
    int nColumns() const;
    int nRows() const;
    double xResolution() const;
    double yResolution() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;
    QString outputFileName() const;
    QString outputFormat() const;
    QgsCoordinateReferenceSystem outputCrs();
    QStringList createOptions() const;
    QgsRectangle outputRectangle() const;
    QList<QgsRasterNuller::NoData> noData() const;

    QList< int > pyramidsList() const;
    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
    QString pyramidsResamplingMethod() const { return mPyramidsOptionsWidget->resamplingMethod(); }
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const
    { return mPyramidsOptionsWidget->pyramidsFormat(); }
    QStringList pyramidsConfigOptions() const
    { return mPyramidsOptionsWidget->configOptions(); }

    void hideFormat();
    void hideOutput();

  public slots:
    virtual void accept() { if ( validate() ) return QDialog::accept(); }

  private slots:
    void on_mRawModeRadioButton_toggled( bool );
    void on_mBrowseButton_clicked();
    void on_mSaveAsLineEdit_textChanged( const QString& text );
    void on_mCurrentExtentButton_clicked();
    void on_mOriginalExtentButton_clicked();
    void on_mFormatComboBox_currentIndexChanged( const QString& text );
    void on_mResolutionRadioButton_toggled( bool ) { toggleResolutionSize(); }
    void on_mOriginalResolutionPushButton_clicked() { setOriginalResolution(); }
    void on_mXResolutionLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcSize(); }
    void on_mYResolutionLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcSize(); }

    void on_mOriginalSizePushButton_clicked() { setOriginalSize(); }
    void on_mColumnsLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcResolution(); }
    void on_mRowsLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcResolution(); }

    void on_mXMinLineEdit_textEdited( const QString & ) { mExtentState = UserExtent; extentChanged(); }
    void on_mXMaxLineEdit_textEdited( const QString & ) { mExtentState = UserExtent; extentChanged(); }
    void on_mYMinLineEdit_textEdited( const QString & ) { mExtentState = UserExtent; extentChanged(); }
    void on_mYMaxLineEdit_textEdited( const QString & ) { mExtentState = UserExtent; extentChanged(); }

    void on_mChangeCrsPushButton_clicked();

    void on_mCrsComboBox_currentIndexChanged( int ) { crsChanged(); }

    void on_mAddNoDataManuallyToolButton_clicked();
    void on_mLoadTransparentNoDataToolButton_clicked();
    void on_mRemoveSelectedNoDataToolButton_clicked();
    void on_mRemoveAllNoDataToolButton_clicked();
    void noDataCellTextEdited( const QString & text );
    void on_mTileModeCheckBox_toggled( bool toggled );
    void on_mPyramidsGroupBox_toggled( bool toggled );
    void populatePyramidsLevels();

  private:
    QgsRasterLayer* mRasterLayer;
    QgsRasterDataProvider* mDataProvider;
    QgsRectangle mCurrentExtent;
    QgsCoordinateReferenceSystem mLayerCrs; // may differ from provider CRS
    QgsCoordinateReferenceSystem mCurrentCrs;
    QgsCoordinateReferenceSystem mUserCrs;
    QgsCoordinateReferenceSystem mPreviousCrs;
    ExtentState mExtentState;
    ResolutionState mResolutionState;
    QVector<bool> mNoDataToEdited;

    void setValidators();
    void setOutputExtent( const QgsRectangle& r, const QgsCoordinateReferenceSystem& srcCrs, ExtentState state );
    void extentChanged();
    void updateExtentStateMsg();
    void toggleResolutionSize();
    void setResolution( double xRes, double yRes, const QgsCoordinateReferenceSystem& srcCrs );
    void setOriginalResolution();
    void setOriginalSize();
    void recalcSize();
    void recalcResolution();
    void updateResolutionStateMsg();
    void recalcResolutionSize();
    void crsChanged();
    void updateCrsGroup();

    void addNoDataRow( double min, double max );
    void setNoDataToEdited( int row );
    double noDataCellValue( int row, int column ) const;
    void adjustNoDataCellWidth( int row, int column );
    bool validate() const;
};


#endif // QGSRASTERLAYERSAVEASDIALOG_H

