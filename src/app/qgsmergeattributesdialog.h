/***************************************************************************
                         qgsmergeattributesdialog.h
                         --------------------------
    begin                : May 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMERGEATTRIBUTESDIALOG_H
#define QGSMERGEATTRIBUTESDIALOG_H

#include "ui_qgsmergeattributesdialogbase.h"
#include "qgsfeature.h"
#include "qgsstatisticalsummary.h"

class QgsMapCanvas;
class QgsRubberBand;
class QgsVectorLayer;
class QComboBox;


/** A dialog to insert the merge behaviour for attributes (e.g. for the union features editing tool)*/
class APP_EXPORT QgsMergeAttributesDialog: public QDialog, private Ui::QgsMergeAttributesDialogBase
{
    Q_OBJECT
  public:

    enum ItemDataRole
    {
      FieldIndex = Qt::UserRole /*!< index of corresponding field in source table */
    };


    QgsMergeAttributesDialog( const QgsFeatureList& features, QgsVectorLayer* vl, QgsMapCanvas* canvas, QWidget * parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsMergeAttributesDialog();

    QgsAttributes mergedAttributes() const;

    /** Returns a list of attribute indexes which should be skipped when merging (eg, attributes
     * which have been set to "skip"
     */
    QSet<int> skippedAttributeIndexes() const;

  public slots:

    /** Resets all columns to "skip"
     */
    void setAllToSkip();

  private slots:
    void comboValueChanged( const QString & text );
    void selectedRowChanged();
    void on_mFromSelectedPushButton_clicked();
    void on_mRemoveFeatureFromSelectionButton_clicked();
    void tableWidgetCellChanged( int row, int column );

  private:
    QgsMergeAttributesDialog(); //default constructor forbidden
    void createTableWidgetContents();
    /** Create new combo box with the options for featureXX / mean / min / max */
    QComboBox* createMergeComboBox( QVariant::Type columnType ) const;
    /** Returns the table widget column index of a combo box
    @return the column index or -1 in case of error*/
    int findComboColumn( QComboBox* c ) const;
    /** Calculates the merged value of a column (depending on the selected merge behaviour) and inserts the value in the corresponding cell*/
    void refreshMergedValue( int col );
    /** Inserts the attribute value of a specific feature into the row of merged attributes*/
    QVariant featureAttribute( QgsFeatureId featureId, int col );
    /** Appends the values of the features for the final value*/
    QVariant concatenationAttribute( int col );

    /** Calculates a summary statistic for a column. Returns null if no valid numerical
     * values found in column.
     */
    QVariant calcStatistic( int col, QgsStatisticalSummary::Statistic stat );

    /** Sets mSelectionRubberBand to a new feature*/
    void createRubberBandForFeature( QgsFeatureId featureId );

    QgsFeatureList mFeatureList;
    QgsVectorLayer* mVectorLayer;
    QgsMapCanvas* mMapCanvas;
    /** Item that highlights the selected feature in the merge table*/
    QgsRubberBand* mSelectionRubberBand;

    QgsFields mFields;
    QSet<int> mHiddenAttributes;

    static QList< QgsStatisticalSummary::Statistic > mDisplayStats;

};

#endif // QGSMERGEATTRIBUTESDIALOG_H
