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
#include "qgsrasterrange.h"
#include "qgis_gui.h"
#include "qgshelp.h"

class QgsRasterLayer;
class QgsRasterDataProvider;
class QgsRasterFormatOptionsWidget;

/**
 * \ingroup gui
 * \class QgsRasterLayerSaveAsDialog
 */
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
    enum ResolutionState
    {
      OriginalResolution,
      UserResolution
    };

    //! Constructor for QgsRasterLayerSaveAsDialog
    QgsRasterLayerSaveAsDialog( QgsRasterLayer *rasterLayer,
                                QgsRasterDataProvider *sourceProvider,
                                const QgsRectangle &currentExtent,
                                const QgsCoordinateReferenceSystem &layerCrs,
                                const QgsCoordinateReferenceSystem &currentCrs,
                                QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                Qt::WindowFlags f = Qt::WindowFlags() );

    Mode mode() const;
    int nColumns() const;
    int nRows() const;
    double xResolution() const;
    double yResolution() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;

    /**
     * Returns TRUE if the "add to canvas" checkbox is checked.
     *
     * \see setAddToCanvas()
     */
    bool addToCanvas() const;

    /**
     * Sets whether the  "add to canvas" checkbox should be \a checked.
     *
     * \see addToCanvas()
     * \since QGIS 3.6
     */
    void setAddToCanvas( bool checked );

    QString outputFileName() const;

    /**
     * Name of the output layer within GeoPackage file
     * \since QGIS 3.4
     */
    QString outputLayerName() const;
    QString outputFormat() const;
    QgsCoordinateReferenceSystem outputCrs();
    QStringList createOptions() const;
    QgsRectangle outputRectangle() const;
    QgsRasterRangeList noData() const;

    QList< int > pyramidsList() const;

    /**
     * Returns the pyramid building option.
     */
    Qgis::RasterBuildPyramidOption buildPyramidsFlag() const;

    QString pyramidsResamplingMethod() const { return mPyramidsOptionsWidget->resamplingMethod(); }

    /**
     * Returns the selected pyramid format.
     */
    Qgis::RasterPyramidFormat pyramidsFormat() const { return mPyramidsOptionsWidget->pyramidsFormat(); }

    QStringList pyramidsConfigOptions() const { return mPyramidsOptionsWidget->configOptions(); }

    void hideFormat();
    void hideOutput();

  public slots:
    void accept() override;

  private slots:
    void mRawModeRadioButton_toggled( bool );
    void mFormatComboBox_currentIndexChanged( const QString &text );
    void mResolutionRadioButton_toggled( bool ) { toggleResolutionSize(); }
    void mOriginalResolutionPushButton_clicked() { setOriginalResolution(); }
    void mXResolutionLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcSize(); }
    void mYResolutionLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcSize(); }

    void mOriginalSizePushButton_clicked() { setOriginalSize(); }
    void mColumnsLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcResolution(); }
    void mRowsLineEdit_textEdited( const QString & ) { mResolutionState = UserResolution; recalcResolution(); }

    void mAddNoDataManuallyToolButton_clicked();
    void mLoadTransparentNoDataToolButton_clicked();
    void mRemoveSelectedNoDataToolButton_clicked();
    void mRemoveAllNoDataToolButton_clicked();
    void noDataCellTextEdited( const QString &text );
    void mTileModeCheckBox_toggled( bool toggled );
    void mPyramidsGroupBox_toggled( bool toggled );
    void populatePyramidsLevels();
    void extentChanged();
    void crsChanged();
    void showHelp();

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsRasterDataProvider *mDataProvider = nullptr;
    QgsRectangle mCurrentExtent;
    QgsCoordinateReferenceSystem mLayerCrs; // may differ from provider CRS
    QgsCoordinateReferenceSystem mCurrentCrs;
    QgsCoordinateReferenceSystem mPreviousCrs;
    ResolutionState mResolutionState;
    QVector<bool> mNoDataToEdited;

    void setValidators();
    void toggleResolutionSize();
    void setResolution( double xRes, double yRes, const QgsCoordinateReferenceSystem &srcCrs );
    void setOriginalResolution();
    void setOriginalSize();
    void recalcSize();
    void recalcResolution();
    void updateResolutionStateMsg();
    void recalcResolutionSize();

    void addNoDataRow( double min, double max );
    void setNoDataToEdited( int row );
    double noDataCellValue( int row, int column ) const;
    void adjustNoDataCellWidth( int row, int column );
    bool validate() const;
    // Returns true if the output layer already exists.
    bool outputLayerExists() const;

    void insertAvailableOutputFormats();

    friend class TestQgsRasterLayerSaveAsDialog;
};


#endif // QGSRASTERLAYERSAVEASDIALOG_H

