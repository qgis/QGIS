/***************************************************************************
                         qgslayoutmarkerwidget.h
                         --------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSLAYOUTMARKERWIDGET_H
#define QGSLAYOUTMARKERWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutmarkerwidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmarker.h"

/**
 * \ingroup gui
 * \brief A widget for configuring layout shape items.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutMarkerWidget : public QgsLayoutItemBaseWidget, private Ui::QgsLayoutMarkerWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    explicit QgsLayoutMarkerWidget( QgsLayoutItemMarker *marker );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:
    bool setNewItem( QgsLayoutItem *item ) override;


  private:
    QPointer<QgsLayoutItemMarker> mMarker;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    //! Blocks / unblocks the signal of all GUI elements
    void blockAllSignals( bool block );

  private slots:

    void symbolChanged();
    void rotationFromMapCheckBoxChanged( int state );
    void mapChanged( QgsLayoutItem *item );
    void northOffsetSpinBoxChanged( double d );
    void northTypeComboBoxChanged( int index );

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();
};

#endif // QGSLAYOUTMARKERWIDGET_H
