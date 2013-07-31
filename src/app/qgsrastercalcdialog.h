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

/**A dialog to enter a raster calculation expression*/
class QgsRasterCalcDialog: public QDialog, private Ui::QgsRasterCalcDialogBase
{
    Q_OBJECT
  public:
    QgsRasterCalcDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterCalcDialog();

    QString formulaString() const;
    QString outputFile() const;
    QString outputFormat() const;
    bool addLayerToProject() const;

    /**Bounding box for output raster*/
    QgsRectangle outputRectangle() const;
    /**Number of pixels in x-direction*/
    int numberOfColumns() const;
    /**Number of pixels in y-direction*/
    int numberOfRows() const;

    QVector<QgsRasterCalculatorEntry> rasterEntries() const;

  private slots:
    void on_mOutputLayerPushButton_clicked();
    void on_mRasterBandsListWidget_itemDoubleClicked( QListWidgetItem* item );
    void on_mButtonBox_accepted();
    void on_mCurrentLayerExtentButton_clicked();
    void on_mExpressionTextEdit_textChanged();
    void on_mOutputLayerLineEdit_textChanged( const QString& text );
    /**Enables ok button if calculator expression is valid and output file path exists*/
    void setAcceptButtonState();

    //calculator buttons
    void on_mPlusPushButton_clicked();
    void on_mMinusPushButton_clicked();
    void on_mMultiplyPushButton_clicked();
    void on_mDividePushButton_clicked();
    void on_mSqrtButton_clicked();
    void on_mCosButton_clicked();
    void on_mSinButton_clicked();
    void on_mASinButton_clicked();
    void on_mExpButton_clicked();
    void on_mTanButton_clicked();
    void on_mACosButton_clicked();
    void on_mATanButton_clicked();
    void on_mOpenBracketPushButton_clicked();
    void on_mCloseBracketPushButton_clicked();
    void on_mLessButton_clicked();
    void on_mGreaterButton_clicked();
    void on_mEqualButton_clicked();
    void on_mLesserEqualButton_clicked();
    void on_mGreaterEqualButton_clicked();
    void on_mAndButton_clicked();
    void on_mOrButton_clicked();

  private:
    //insert available GDAL drivers that support the create() option
    void insertAvailableOutputFormats();
    /**Accesses the available raster layers/bands from the layer registry*/
    void insertAvailableRasterBands();

    /**Returns true if raster calculator expression has valid syntax*/
    bool expressionValid() const;
    /**Returns true if output file directory exists*/
    bool filePathValid() const;

    static QString quoteBandEntry( const QString& layerName );

    /**Stores relation between driver name and extension*/
    QMap<QString, QString> mDriverExtensionMap;

    QList<QgsRasterCalculatorEntry> mAvailableRasterBands;
};

#endif // QGSRASTERCALCDIALOG_H
