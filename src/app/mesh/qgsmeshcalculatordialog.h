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
#include "qgis_app.h"

//! A dialog to enter a mesh calculation expression
class APP_EXPORT QgsMeshCalculatorDialog: public QDialog, private Ui::QgsMeshCalculatorDialogBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for mesh calculator dialog
     * \param meshLayer main mesh layer, will be used for default extent and projection
     * \param parent widget
     * \param f window flags
     */
    QgsMeshCalculatorDialog( QgsMeshLayer *meshLayer = nullptr, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    ~QgsMeshCalculatorDialog();

    //! Returns new mesh calculator created from dialog options
    std::unique_ptr<QgsMeshCalculator> calculator() const;

  private slots:
    void datasetGroupEntry( const QModelIndex &index );
    void mCurrentLayerExtentButton_clicked();
    void mAllTimesButton_clicked();
    void toggleExtendMask();
    void updateInfoMessage();
    //! Disables some options that are not required if using Virtual Provider
    void onVirtualCheckboxChange();
    void onOutputFormatChange();

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
    QString driver() const;
    QString groupName() const;

    double startTime() const;
    double endTime() const;

    QString datasetGroupName( const QModelIndex &index ) const;

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

    //! Quotes the dataset group name for calculator
    QString quoteDatasetGroupEntry( const QString group );

    //! Add file suffix if not present
    QString controlSuffix( const QString &fileName ) const;

    //! Returns the current output suffix
    QString currentOutputSuffix() const;

    //! Gets all mesh drivers that can persist datasets
    void getMeshDrivers();

    //! Populates the combo box with output formats
    void populateDriversComboBox( );

    QgsMeshLayer *mLayer;
    QHash<QString, QgsMeshDriverMetadata> mMeshDrivers;
    QStringList mVariableNames;

    friend class TestQgsMeshCalculatorDialog;
};

#endif // QGSMESHCALCULATORDIALOG_H
