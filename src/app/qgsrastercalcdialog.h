/***************************************************************************
                          qgsrastercalcdialog.h  -  description
                          ---------------------
    begin                : September 28th, 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCALCDIALOG_H
#define QGSRASTERCALCDIALOG_H

#include "ui_qgsrastercalcdialogbase.h"

#include "qgis_app.h"
#include "qgsrastercalculator.h"

class QgsMapCanvas;

//! A dialog to enter a raster calculation expression
class APP_EXPORT QgsRasterCalcDialog : public QDialog, private Ui::QgsRasterCalcDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for raster calculator dialog
     * \param rasterLayer main raster layer, will be used for default extent and projection
     * \param parent widget
     * \param f window flags
     */
    QgsRasterCalcDialog( QgsRasterLayer *rasterLayer = nullptr, QgsMapCanvas *canvas = nullptr, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    [[nodiscard]] QString formulaString() const;
    [[nodiscard]] QString outputFile() const;
    [[nodiscard]] QString outputFormat() const;
    [[nodiscard]] QgsCoordinateReferenceSystem outputCrs() const;
    [[nodiscard]] bool addLayerToProject() const;
    //! True if Virtual is checked
    [[nodiscard]] bool useVirtualProvider() const;
    //! Return the name written in the qlineedit
    [[nodiscard]] QString virtualLayerName() const;

    //! Bounding box for output raster
    [[nodiscard]] QgsRectangle outputRectangle() const;
    //! Number of pixels in x-direction
    [[nodiscard]] int numberOfColumns() const;
    //! Number of pixels in y-direction
    [[nodiscard]] int numberOfRows() const;

    //! Raster creation options
    [[nodiscard]] QStringList creationOptions() const;

    /**
     * Extract raster layer information from the current project
     * \return a vector of raster entries from the current project
     * \deprecated QGIS 3.6. Use QgsRasterCalculatorEntry::rasterEntries() instead.
     */
    Q_DECL_DEPRECATED QVector<QgsRasterCalculatorEntry> rasterEntries() const SIP_DEPRECATED;

  private slots:
    void mRasterBandsListWidget_itemDoubleClicked( QListWidgetItem *item );
    void mOutputFormatComboBox_currentIndexChanged( const QString &text );
    void mButtonBox_accepted();
    void mExpressionTextEdit_textChanged();
    void extentLayerChanged( QgsMapLayer *layer );
    //! Enables OK button if calculator expression is valid and output file path exists
    void setAcceptButtonState();
    //! Disables some options that are not required if using Virtual Provider
    void setOutputToVirtual();
    void showHelp();

    //calculator buttons
    void mPlusPushButton_clicked();
    void mMinusPushButton_clicked();
    void mMultiplyPushButton_clicked();
    void mDividePushButton_clicked();
    void mSqrtButton_clicked();
    void mCosButton_clicked();
    void mSinButton_clicked();
    void mASinButton_clicked();
    void mExpButton_clicked();
    void mLnButton_clicked();
    void mLogButton_clicked();
    void mNotEqualButton_clicked();
    void mTanButton_clicked();
    void mACosButton_clicked();
    void mATanButton_clicked();
    void mOpenBracketPushButton_clicked();
    void mCloseBracketPushButton_clicked();
    void mLessButton_clicked();
    void mGreaterButton_clicked();
    void mEqualButton_clicked();
    void mLesserEqualButton_clicked();
    void mGreaterEqualButton_clicked();
    void mAndButton_clicked();
    void mOrButton_clicked();
    void mAbsButton_clicked();
    void mMinButton_clicked();
    void mMaxButton_clicked();
    void mConditionalStatButton_clicked();

  private:
    //! Sets the extent and size of the output
    void setExtentSize( QgsRasterLayer *layer );

    // Insert available GDAL drivers that support the create() option
    void insertAvailableOutputFormats();
    //! Accesses the available raster layers/bands from the layer registry
    void insertAvailableRasterBands();

    //! Returns true if raster calculator expression has valid syntax
    [[nodiscard]] bool expressionValid() const;
    //! Returns true if output file directory exists
    [[nodiscard]] bool filePathValid() const;

    static QString quoteBandEntry( const QString &layerName );

    //! Returns true if the output layer already exists.
    [[nodiscard]] bool outputLayerExists() const;

    //! Stores relation between driver name and extension
    QMap<QString, QString> mDriverExtensionMap;

    QList<QgsRasterCalculatorEntry> mAvailableRasterBands;

    bool mExtentSizeSet = false;
    QgsMapCanvas *mMapCanvas = nullptr;

    friend class TestQgsRasterCalcDialog;
};

#endif // QGSRASTERCALCDIALOG_H
