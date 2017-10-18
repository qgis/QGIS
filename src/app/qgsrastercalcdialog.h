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
#include "qgsrastercalculator.h"
#include "qgshelp.h"
#include "qgis_app.h"

//! A dialog to enter a raster calculation expression
class APP_EXPORT QgsRasterCalcDialog: public QDialog, private Ui::QgsRasterCalcDialogBase
{
    Q_OBJECT
  public:
    QgsRasterCalcDialog( QWidget *parent = nullptr, Qt::WindowFlags f = 0 );
    ~QgsRasterCalcDialog();

    QString formulaString() const;
    QString outputFile() const;
    QString outputFormat() const;
    QgsCoordinateReferenceSystem outputCrs() const;
    bool addLayerToProject() const;

    //! Bounding box for output raster
    QgsRectangle outputRectangle() const;
    //! Number of pixels in x-direction
    int numberOfColumns() const;
    //! Number of pixels in y-direction
    int numberOfRows() const;

    QVector<QgsRasterCalculatorEntry> rasterEntries() const;

  private slots:
    void mOutputLayerPushButton_clicked();
    void mRasterBandsListWidget_itemDoubleClicked( QListWidgetItem *item );
    void mButtonBox_accepted();
    void mCurrentLayerExtentButton_clicked();
    void mExpressionTextEdit_textChanged();
    void mOutputLayerLineEdit_textChanged( const QString &text );
    //! Enables OK button if calculator expression is valid and output file path exists
    void setAcceptButtonState();
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

  private:
    //insert available GDAL drivers that support the create() option
    void insertAvailableOutputFormats();
    //! Accesses the available raster layers/bands from the layer registry
    void insertAvailableRasterBands();

    //! Returns true if raster calculator expression has valid syntax
    bool expressionValid() const;
    //! Returns true if output file directory exists
    bool filePathValid() const;

    static QString quoteBandEntry( const QString &layerName );

    //! Stores relation between driver name and extension
    QMap<QString, QString> mDriverExtensionMap;

    QList<QgsRasterCalculatorEntry> mAvailableRasterBands;
};

#endif // QGSRASTERCALCDIALOG_H
