/***************************************************************************
  qgslayertreeembeddedconfigwidget.h
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
#define QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H

#include "ui_qgslayertreeembeddedconfigwidgetbase.h"
#include "qgis.h"
#include "qgis_gui.h"

class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsLayerTreeEmbeddedConfigWidget
 * \brief A widget to configure layer tree embedded widgets for a particular map layer.
 */
class GUI_EXPORT QgsLayerTreeEmbeddedConfigWidget : public QWidget, protected Ui::QgsLayerTreeEmbeddedConfigWidgetBase
{
    Q_OBJECT
  public:
    /**
     * A widget to configure layer tree embedded widgets for a particular map layer.
     * \param parent The parent of the widget.
     */
    QgsLayerTreeEmbeddedConfigWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Initialize widget with a map layer
    void setLayer( QgsMapLayer *layer );

    //! Store changes made in the widget to the layer
    void applyToLayer();

  private slots:
    void onAddClicked();
    void onRemoveClicked();

  private:
    QgsMapLayer *mLayer = nullptr;
};

#endif // QGSLAYERTREEEMBEDDEDCONFIGWIDGET_H
