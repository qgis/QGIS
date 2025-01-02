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
#include "qgsfields.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsRubberBand;
class QgsVectorLayer;
class QComboBox;
class QgsAttributeTableConfig;


//! A dialog to insert the merge behavior for attributes (e.g. for the union features editing tool)
class APP_EXPORT QgsMergeAttributesDialog : public QDialog, private Ui::QgsMergeAttributesDialogBase
{
    Q_OBJECT
  public:
    enum ItemDataRole
    {
      FieldIndex = Qt::UserRole //!< Index of corresponding field in source table for table header
    };


    QgsMergeAttributesDialog( const QgsFeatureList &features, QgsVectorLayer *vl, QgsMapCanvas *canvas, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    ~QgsMergeAttributesDialog() override;

    QgsAttributes mergedAttributes() const;

    /**
     * Returns the id of the target feature.
     * By default it is the first feature of the list. Otherwise the feature explicitly selected
     * with buttons "Take attributes from selected feature" or "Take attributes from feature with
     * the largest area".
     *
     * \returns The id of the target feature.
     *
     * \since QGIS 3.30
     */
    QgsFeatureId targetFeatureId() const;

    /**
     * Returns a list of attribute indexes which should be skipped when merging (e.g., attributes
     * which have been set to "skip"
     */
    QSet<int> skippedAttributeIndexes() const;

  public slots:

    /**
     * Resets all columns to "skip"
     */
    void setAllToSkip();

  private slots:
    void selectedRowChanged();
    void mFromSelectedPushButton_clicked();
    void mFromLargestPushButton_clicked();
    void mRemoveFeatureFromSelectionButton_clicked();
    void tableWidgetCellClicked( int row, int column );
    void updateManualWidget( int column, bool isManual );

  private:
    QgsMergeAttributesDialog(); //default constructor forbidden
    void createTableWidgetContents();
    void setAttributeTableConfig( const QgsAttributeTableConfig &config );

    //! Create new combo box with the options for featureXX / mean / min / max
    QComboBox *createMergeComboBox( QMetaType::Type columnType, int column );

    /**
     * Returns the table widget column index of a combo box
     * \returns the column index or -1 in case of error
    */
    int findComboColumn( QComboBox *c ) const;
    //! Calculates the merged value of a column (depending on the selected merge behavior) and inserts the value in the corresponding cell
    void refreshMergedValue( int col );
    //! Inserts the attribute value of a specific feature into the row of merged attributes
    QVariant featureAttribute( QgsFeatureId featureId, int fieldIdx );
    //! Inserts all attribute values of a specific feature into the row of merged attributes
    void setAllAttributesFromFeature( QgsFeatureId featureId );
    //! Appends the values of the features for the final value
    QVariant concatenationAttribute( int col );

    /**
     * Calculates a summary statistic for a column. Returns null if no valid numerical
     * values found in column.
     */
    QVariant calcStatistic( int col, Qgis::Statistic stat );

    //! Sets mSelectionRubberBand to a new feature
    void createRubberBandForFeature( QgsFeatureId featureId );

    QgsFeatureList mFeatureList;
    QgsFeatureId mTargetFeatureId = FID_NULL;
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    //! Item that highlights the selected feature in the merge table
    QgsRubberBand *mSelectionRubberBand = nullptr;

    QgsFields mFields;
    QSet<int> mHiddenAttributes;
    QMap<QString, int> mFieldToColumnMap;
    bool mUpdating = false;

    static const QList<Qgis::Statistic> DISPLAY_STATS;
};

#endif // QGSMERGEATTRIBUTESDIALOG_H
