/***************************************************************************
  qgslayertreeviewindicator.h
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWINDICATOR_H
#define QGSLAYERTREEVIEWINDICATOR_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QIcon>
#include <QObject>

/**
 * \ingroup gui
 * \brief Indicator that can be used in a layer tree view to display icons next to items of the layer tree.
 * They add extra context to the item and interactivity (using clicked() signal).
 *
 * Indicators can be added/removed to individual layer tree items using QgsLayerTreeView::addIndicator()
 * and QgsLayerTreeView::removeIndicator() calls.
 *
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsLayerTreeViewIndicator : public QObject
{
    Q_OBJECT
  public:
    //! Constructs an indicator, optionally transferring ownership to a parent QObject
    explicit QgsLayerTreeViewIndicator( QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Indicator icon that will be displayed in the layer tree view
    QIcon icon() const { return mIcon; }
    //! Sets indicator icon that will be displayed in the layer tree view
    void setIcon( const QIcon &icon )
    {
      mIcon = icon;
      emit changed();
    }

    //! Returns tool tip text that will be shown when user hovers mouse over the indicator
    QString toolTip() const { return mToolTip; }
    //! Sets tool tip text
    void setToolTip( const QString &tip ) { mToolTip = tip; }

  signals:
    //! Emitted when user clicks on the indicator
    void clicked( const QModelIndex &index );

    /**
     * Emitted when the indicator changes state (e.g. icon).
     * \since QGIS 3.10
     */
    void changed();

  private:
    QIcon mIcon;
    QString mToolTip;
};

#endif // QGSLAYERTREEVIEWINDICATOR_H
