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

/** A dialog class that provides calculation of new fields using existing fields, values and a set of operators*/
class APP_EXPORT QgsFieldCalculator: public QDialog, private Ui::QgsFieldCalculatorBase
{
    Q_OBJECT
  public:
    QgsFieldCalculator( QgsVectorLayer* vl );
    ~QgsFieldCalculator();

    int changedAttributeId() const { return mAttributeId; }

  public slots:
    void accept() override;

    void on_mNewFieldGroupBox_toggled( bool on );
    void on_mUpdateExistingGroupBox_toggled( bool on );
    void on_mCreateVirtualFieldCheckbox_stateChanged( int state );
    void on_mOutputFieldNameLineEdit_textChanged( const QString& text );
    void on_mOutputFieldTypeComboBox_activated( int index );

    void on_mButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private slots:
    /** Sets the ok button enabled / disabled*/
    void setOkButtonState();

  private:
    //! default constructor forbidden
    QgsFieldCalculator();
    /** Inserts existing fields into the combo box*/
    void populateFields();
    /** Inserts the types supported by the provider into the combo box*/
    void populateOutputFieldTypes();

    QgsVectorLayer* mVectorLayer;
    /** Key: field name, Value: field index*/
    QMap<QString, int> mFieldMap;

    /** Create a field based on the definitions */
    inline QgsField fieldDefinition()
    {
      return QgsField( mOutputFieldNameLineEdit->text(),
                       ( QVariant::Type ) mOutputFieldTypeComboBox->itemData( mOutputFieldTypeComboBox->currentIndex(), Qt::UserRole ).toInt(),
                       mOutputFieldTypeComboBox->itemData( mOutputFieldTypeComboBox->currentIndex(), Qt::UserRole + 1 ).toString(),
                       mOutputFieldWidthSpinBox->value(),
                       mOutputFieldPrecisionSpinBox->value() );
    }

    /** Idx of changed attribute*/
    int mAttributeId;
};

#endif // QGSFIELDCALCULATOR_H
