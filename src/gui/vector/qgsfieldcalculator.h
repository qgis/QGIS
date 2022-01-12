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
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsFieldCalculator
 *
 * \brief A dialog class that provides calculation of new fields using existing fields, values and a set of operators
 *
 * Sample usage of the QgsFieldCalculator class:
 *
 * \code{.py}
 *     uri = "point?crs=epsg:4326&field=id:integer"
 *     layer = QgsVectorLayer(uri, "Scratch point layer",  "memory")
 *     layer.startEditing()
 *     dialog = QgsFieldCalculator(layer)
 *     dialog.exec_()
 * \endcode
 */
class GUI_EXPORT QgsFieldCalculator: public QDialog, private Ui::QgsFieldCalculatorBase
{
    Q_OBJECT
  public:
    QgsFieldCalculator( QgsVectorLayer *vl, QWidget *parent = nullptr );

    /**
     * \brief Returns the field index of the field for which new attribute values were calculated.
     *
     * \returns The field index if attribute values were calculated or -1, e.g. in case of geometry changes.
     */
    int changedAttributeId() const { return mAttributeId; }

  public slots:
    void accept() override;

  private slots:
    void mNewFieldGroupBox_toggled( bool on );
    void mUpdateExistingGroupBox_toggled( bool on );
    void mCreateVirtualFieldCheckbox_stateChanged( int state );
    void mOutputFieldNameLineEdit_textChanged( const QString &text );
    void mOutputFieldTypeComboBox_activated( int index );

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

    bool mCanAddAttribute = false;
    bool mCanChangeAttributeValue = false;

    //! Create a field based on the definitions
    QgsField fieldDefinition();

    //! Idx of changed attribute
    int mAttributeId;

    friend class TestQgsFieldCalculator;
};

#endif // QGSFIELDCALCULATOR_H
