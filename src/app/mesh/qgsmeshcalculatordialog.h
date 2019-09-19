/***************************************************************************
                          qgsmeshcalculatordialog.h
                          -------------------------
    begin                : January 2019
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHCALCULATORDIALOG_H
#define QGSMESHCALCULATORDIALOG_H

#include "ui_qgsmeshcalculatordialogbase.h"
#include "qgsmeshcalculator.h"
#include "qgshelp.h"
#include "qgis_app.h"

//! A dialog to enter a mesh calculation expression
class APP_EXPORT QgsMeshCalculatorDialog: public QDialog, private Ui::QgsMeshCalculatorDialogBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for raster calculator dialog
     * \param meshLayer main mesh layer, will be used for default extent and projection
     * \param parent widget
     * \param f window flags
     */
    QgsMeshCalculatorDialog( QgsMeshLayer *meshLayer = nullptr, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsMeshCalculatorDialog();

    //! Returns new mesh calculator created from dialog options
    std::unique_ptr<QgsMeshCalculator> calculator() const;

  private slots:
    void mDatasetsListWidget_doubleClicked( QListWidgetItem *item );
    void mCurrentLayerExtentButton_clicked();
    void mAllTimesButton_clicked();
    void mExpressionTextEdit_textChanged();
    void toggleExtendMask( int state );

    //calculator buttons
    void mPlusPushButton_clicked();
    void mMinusPushButton_clicked();
    void mLessButton_clicked();
    void mLesserEqualButton_clicked();
    void mMultiplyPushButton_clicked();
    void mDividePushButton_clicked();
    void mGreaterButton_clicked();
    void mGreaterEqualButton_clicked();
    void mOpenBracketPushButton_clicked();
    void mCloseBracketPushButton_clicked();
    void mEqualButton_clicked();
    void mNotEqualButton_clicked();
    void mMinButton_clicked();
    void mMaxButton_clicked();
    void mAbsButton_clicked();
    void mPowButton_clicked();
    void mIfButton_clicked();
    void mAndButton_clicked();
    void mOrButton_clicked();
    void mNotButton_clicked();
    void mSumAggrButton_clicked();
    void mMaxAggrButton_clicked();
    void mMinAggrButton_clicked();
    void mAverageAggrButton_clicked();
    void mNoDataButton_clicked();

  private:
    QString formulaString() const;
    QgsMeshLayer *meshLayer() const;

    QString outputFile() const;
    QgsRectangle outputExtent() const;
    QgsGeometry maskGeometry() const;

    double startTime() const;
    double endTime() const;

    //! Combines geometries from selected vector layer to create mask filter geometry
    QgsGeometry maskGeometry( QgsVectorLayer *layer ) const;

    //! Sets widget to match full layer extent
    void useFullLayerExtent();

    //! Sets time combos from current layer
    void useAllTimesFromLayer();

    //! Returns name of the selected dataset group
    QString currentDatasetGroup() const;

    //! Sets time combos from selected group
    void setTimesByDatasetGroupName( const QString group );

    //! Sets available times from layer
    void repopulateTimeCombos();

    //! Enables OK button if calculator expression is valid and output file path exists
    void setAcceptButtonState();

    //! Quotes the dataset group name for calculator
    QString quoteDatasetGroupEntry( const QString group );

    //! Gets all datasets groups from layer and populated list
    void getDatasetGroupNames();

    //! Returns true if mesh calculator expression has valid syntax
    bool expressionValid() const;

    //! Returns true if file path is valid and writable
    bool filePathValid() const;

    //! Add file suffix if not present
    QString addSuffix( const QString fileName ) const;

    QgsMeshLayer *mLayer;
    friend class TestQgsMeshCalculatorDialog;
};

#endif // QGSMESHCALCULATORDIALOG_H
