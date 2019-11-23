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
#include "qgshelp.h"
#include "qgsfields.h"
#include "qgis_app.h"

class QgsVectorLayer;

//! A dialog class that provides calculation of new fields using existing fields, values and a set of operators
class APP_EXPORT QgsFieldCalculator: public QDialog, private Ui::QgsFieldCalculatorBase
{
    Q_OBJECT
  public:
    QgsFieldCalculator( QgsVectorLayer *vl, QWidget *parent = nullptr );

    int changedAttributeId() const { return mAttributeId; }

  public slots:
    void accept() override;

    void mNewFieldGroupBox_toggled( bool on );
    void mUpdateExistingGroupBox_toggled( bool on );
    void mCreateVirtualFieldCheckbox_stateChanged( int state );
    void mOutputFieldNameLineEdit_textChanged( const QString &text );
    void mOutputFieldTypeComboBox_activated( int index );

  private slots:
    //! Sets the OK button enabled / disabled
    void setOkButtonState();
    void setPrecisionMinMax();
    void showHelp();

  private:
    //! default constructor forbidden
    QgsFieldCalculator();
    //! Inserts existing fields into the combo box
    void populateFields();
    //! Inserts the types supported by the provider into the combo box
    void populateOutputFieldTypes();

    QgsVectorLayer *mVectorLayer = nullptr;

    //! Create a field based on the definitions
    QgsField fieldDefinition();

    //! Idx of changed attribute
    int mAttributeId;

    friend class TestQgsFieldCalculator;
};

#endif // QGSFIELDCALCULATOR_H
