/***************************************************************************
                         qgslayoutchartwidget.h
                         --------------------------
     begin                : August 2025
     copyright            : (C) 2025 by Mathieu
     email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTCHARTWIDGET_H
#define QGSLAYOUTCHARTWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgslayoutchartwidgetbase.h"

#include "qgis_gui.h"
#include "qgslayoutitemwidget.h"

class QgsLayoutItemChart;

/**
 * \ingroup gui
 * \brief A widget for configuring layout chart items.
 *
 * \note This class is not a part of public API
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsLayoutChartWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutChartWidgetBase
{
    Q_OBJECT

  public:
    //! constructor
    explicit QgsLayoutChartWidget( QgsLayoutItemChart *chartItem );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  private slots:
    void changeLayer( QgsMapLayer *layer );
    void changeSortExpression( const QString &expression, bool valid );

    void mChartTypeComboBox_currentIndexChanged( int index );
    void mChartPropertiesButton_clicked();
    void mFlipAxesCheckBox_stateChanged( int state );

    void mSortCheckBox_stateChanged( int state );
    void mSortDirectionButton_clicked();

    void mSeriesListWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );
    void mSeriesListWidget_itemChanged( QListWidgetItem *item );
    void mAddSeriesPushButton_clicked();
    void mRemoveSeriesPushButton_clicked();
    void mSeriesPropertiesButton_clicked();

    void mLinkedMapComboBox_itemChanged( QgsLayoutItem *item );
    void mFilterOnlyVisibleFeaturesCheckBox_stateChanged( int state );
    void mIntersectAtlasCheckBox_stateChanged( int state );

  private:
    //! Sets the GUI elements to the values of mChartItem
    void setGuiElementValues();

    //! Adds a new item to the series list widget
    QListWidgetItem *addSeriesListItem( const QString &name );

    QPointer<QgsLayoutItemChart> mChartItem;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
};

#endif // QGSLAYOUTCHARTWIDGET_H
