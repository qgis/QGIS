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

class QgsMapCanvas;
class QgsRubberBand;
class QgsVectorLayer;
class QComboBox;


/**A dialog to insert the merge behaviour for attributes (e.g. for the union features editing tool)*/
class QgsMergeAttributesDialog: public QDialog, private Ui::QgsMergeAttributesDialogBase
{
    Q_OBJECT
  public:
    QgsMergeAttributesDialog( const QgsFeatureList& features, QgsVectorLayer* vl, QgsMapCanvas* canvas, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsMergeAttributesDialog();
    QgsAttributeMap mergedAttributesMap() const;

  private slots:
    void comboValueChanged( const QString & text );
    void selectedRowChanged();
    void on_mFromSelectedPushButton_clicked();
    void on_mRemoveFeatureFromSelectionButton_clicked();

  private:
    QgsMergeAttributesDialog(); //default constructor forbidden
    void createTableWidgetContents();
    /**Create new combo box with the options for featureXX / mean / min / max */
    QComboBox* createMergeComboBox( QVariant::Type columnType ) const;
    /**Returns the table widget column index of a combo box
    @return the column index or -1 in case of error*/
    int findComboColumn( QComboBox* c ) const;
    /**Calculates the merged value of a column (depending on the selected merge behaviour) and inserts the value in the corresponding cell*/
    void refreshMergedValue( int col );
    /**Inserts the attribute value of a specific feature into the row of merged attributes*/
    QString featureAttributeString( int featureId, int col );
    /**Calculates and inserts the minimum attribute value of a column*/
    QString minimumAttributeString( int col );
    /**Calculates and inserts the maximum value of a column*/
    QString maximumAttributeString( int col );
    /**Calculates and inserts the mean value of a column*/
    QString meanAttributeString( int col );
    /**Calculates and inserts the median value of a column*/
    QString medianAttributeString( int col );
    /**Calculates and inserts the sum of a column*/
    QString sumAttributeString( int col );
    /**Appends the values of the features for the final value*/
    QString concatenationAttributeString( int col );
    /**Sets mSelectionRubberBand to a new feature*/
    void createRubberBandForFeature( int featureId );

    QgsFeatureList mFeatureList;
    QgsVectorLayer* mVectorLayer;
    QgsMapCanvas* mMapCanvas;
    /**Item that highlights the selected feature in the merge table*/
    QgsRubberBand* mSelectionRubberBand;
};

#endif // QGSMERGEATTRIBUTESDIALOG_H
