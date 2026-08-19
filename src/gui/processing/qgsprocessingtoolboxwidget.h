/***************************************************************************
    qgsprocessingtoolboxwidget.h
    -------------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Valentin Buira
    email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGTOOLBOXWIDGET_H
#define QGSPROCESSINGTOOLBOXWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsfilterlineedit.h"
#include "qgsprocessingtoolboxtreeview.h"

#include <QWidget>

/**
 * \ingroup gui
 *
 * \brief Base class TODO
 * TODO: add description
 *
 */
class GUI_EXPORT QgsProcessingToolboxWidget : public QWidget // SIP_ABSTRACT
{
    Q_OBJECT
  public:
    QgsProcessingToolboxWidget();

    // setRegistry
    // setFilters
    // setInPlaceLayer
    // indexAt
    // selectedAlgorithm
    // mapToGlobal

    // setToolboxProxyModel
    // setDragDropMode
    // setDropIndicatorShown
    // setFilterString

    // signal
    // customContextMenuRequested
    // doubleClicked

    // OR expose directly the treeview and the search edit as public members ?


  private:
    std::unique_ptr<QgsProcessingToolboxTreeView> mToolboxTreeView;
    std::unique_ptr<QgsFilterLineEdit> mToolboxSearchEdit;
};

#endif // QGSPROCESSINGTOOLBOXWIDGET_H
