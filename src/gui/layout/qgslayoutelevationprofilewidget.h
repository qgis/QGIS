/***************************************************************************
                         qgslayoutelevationprofilewidget.h
                         ----------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTELEVATIONPROFILEWIDGET_H
#define QGSLAYOUTELEVATIONPROFILEWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutelevationprofilewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemelevationprofile.h"
#include <functional>
#include <QPointer>

/**
 * \ingroup gui
 * \brief A widget for layout elevation profile item settings.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsLayoutElevationProfileWidget: public QgsLayoutItemBaseWidget, public QgsExpressionContextGenerator, private Ui::QgsLayoutElevationProfileWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    explicit QgsLayoutElevationProfileWidget( QgsLayoutItemElevationProfile *profile );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;
    QgsExpressionContext createExpressionContext() const override;

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private slots:

    void setGuiElementValues();

  private:

    QPointer< QgsLayoutItemElevationProfile > mProfile = nullptr;

    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;
};

#endif //QGSLAYOUTELEVATIONPROFILEWIDGET_H
