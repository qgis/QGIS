/***************************************************************************
    qgsfieldcalculator.h
    ---------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDCALCULATOR_H
#define QGSFIELDCALCULATOR_H

#include "ui_qgsfieldcalculatorbase.h"
#include "qgscontexthelp.h"

class QgsVectorLayer;

/**A dialog class that provides calculation of new fields using existing fields, values and a set of operators*/
class QgsFieldCalculator: public QDialog, private Ui::QgsFieldCalculatorBase
{
    Q_OBJECT
  public:
    QgsFieldCalculator( QgsVectorLayer* vl );
    ~QgsFieldCalculator();

  public slots:
    void accept();

    void on_mUpdateExistingFieldCheckBox_stateChanged( int state );
    void on_mFieldsListWidget_itemDoubleClicked( QListWidgetItem * item );
    void on_mValueListWidget_itemDoubleClicked( QListWidgetItem * item );
    void on_mPlusPushButton_clicked();
    void on_mMinusPushButton_clicked();
    void on_mMultiplyPushButton_clicked();
    void on_mDividePushButton_clicked();
    void on_mSqrtButton_clicked();
    void on_mExpButton_clicked();
    void on_mSinButton_clicked();
    void on_mCosButton_clicked();
    void on_mTanButton_clicked();
    void on_mASinButton_clicked();
    void on_mACosButton_clicked();
    void on_mATanButton_clicked();
    void on_mOpenBracketPushButton_clicked();
    void on_mCloseBracketPushButton_clicked();
    void on_mToRealButton_clicked();
    void on_mToIntButton_clicked();
    void on_mToStringButton_clicked();
    void on_mLengthButton_clicked();
    void on_mAreaButton_clicked();
    void on_mRowNumButton_clicked();
    void on_mConcatButton_clicked();
    void on_mSamplePushButton_clicked();
    void on_mAllPushButton_clicked();
    void on_mOutputFieldNameLineEdit_textChanged( const QString& text );
    void on_mExpressionTextEdit_textChanged();
    void on_mOutputFieldTypeComboBox_activated( int index );

    void on_mButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:
    //default constructor forbidden
    QgsFieldCalculator();
    /**Inserts existing fields into the combo box*/
    void populateFields();
    /**Inserts the types supported by the provider into the combo box*/
    void populateOutputFieldTypes();
    /**Gets field values and inserts them into mValueListWidget
    @limit 0: get all feature, != 0 stop after getting limit values*/
    void getFieldValues( int limit );
    /**Sets the ok button enabled / disabled*/
    void setOkButtonState();

    QgsVectorLayer* mVectorLayer;
    /**Key: field name, Value: field index*/
    QMap<QString, int> mFieldMap;
};

#endif // QGSFIELDCALCULATOR_H
