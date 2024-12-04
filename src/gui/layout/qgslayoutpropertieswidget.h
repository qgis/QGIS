/***************************************************************************
                             qgslayoutpropertieswidget.h
                             ----------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTPROPERTIESWIDGET_H
#define QGSLAYOUTPROPERTIESWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutwidgetbase.h"
#include "qgspanelwidget.h"

class QgsLayout;
class QgsMasterLayoutInterface;

/**
 * \ingroup gui
 * \brief Widget for configuring the properties of a layout.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutPropertiesWidget : public QgsPanelWidget, private Ui::QgsLayoutWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsLayoutPropertiesWidget( QWidget *parent, QgsLayout *layout );

    //! Sets the master layout
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout );

  public slots:

    //! Refreshes the gui to reflect the current layout settings
    void updateGui();

  private slots:

    void gridResolutionChanged( double d );
    void gridResolutionUnitsChanged( Qgis::LayoutUnit unit );
    void gridOffsetXChanged( double d );
    void gridOffsetYChanged( double d );
    void gridOffsetUnitsChanged( Qgis::LayoutUnit unit );
    void snapToleranceChanged( int tolerance );
    void resizeMarginsChanged();
    void resizeToContents();
    void referenceMapChanged( QgsLayoutItem *item );
    void dpiChanged( int value );
    void worldFileToggled();
    void rasterizeToggled();
    void forceVectorToggled();
    void variablesChanged();
    void updateVariables();

  private:
    QgsLayout *mLayout = nullptr;

    void updateSnappingElements();
    void blockSignals( bool block );
    bool mBlockVariableUpdates = false;
};

#endif // QGSLAYOUTPROPERTIESWIDGET_H
