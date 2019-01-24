/***************************************************************************
                             qgslayoutpagepropertieswidget.h
                             -------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPAGEPROPERTIESWIDGET_H
#define QGSLAYOUTPAGEPROPERTIESWIDGET_H

#include "qgis_sip.h"
#include "ui_qgslayoutpagepropertieswidget.h"

#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgslayoutpagecollection.h"

class QgsLayoutItem;
class QgsLayoutItemPage;

/**
 * A widget for configuring properties of pages in a layout
 */
class QgsLayoutPagePropertiesWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPagePropertiesWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutPagePropertiesWidget.
     */
    QgsLayoutPagePropertiesWidget( QWidget *parent, QgsLayoutItem *page );

  signals:

    //! Is emitted when page orientation changes
    void pageOrientationChanged();

  private slots:

    void pageSizeChanged( int index );
    void orientationChanged( int index );
    void updatePageSize();
    void setToCustomSize();
    void symbolChanged();
    void excludeExportsToggled( bool checked );
    void refreshLayout();

  private:

    QgsLayoutItemPage *mPage = nullptr;

    QgsLayoutMeasurementConverter mConverter;

    bool mSettingPresetSize = false;
    bool mBlockPageUpdate = false;

    void showCurrentPageSize();

};

#endif // QGSLAYOUTPAGEPROPERTIESWIDGET_H
