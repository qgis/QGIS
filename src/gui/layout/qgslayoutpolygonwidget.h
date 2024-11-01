/***************************************************************************
                         qgslayoutpolygonwidget.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPOLYGONWIDGET_H
#define QGSLAYOUTPOLYGONWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutpolygonwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitempolygon.h"

/**
 * \ingroup gui
 * \brief Input widget for QgsLayoutItemPolygon
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutPolygonWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPolygonWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    explicit QgsLayoutPolygonWidget( QgsLayoutItemPolygon *polygon );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    QPointer<QgsLayoutItemPolygon> mPolygon;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

  private slots:
    void symbolChanged();
};

#endif // QGSLAYOUTPOLYGONWIDGET_H
