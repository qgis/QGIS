/***************************************************************************
    qgsbrowserguimodel.h
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERGUIMODEL_H
#define QGSBROWSERGUIMODEL_H

#include "qgis_gui.h"
#include "qgsbrowsermodel.h"

class QgsDataItemGuiContext;
class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsBrowserGuiModel
 *
 * \brief A model for showing available data sources and other items in a structured
 * tree.
 *
 * QgsBrowserGuiModel is the foundation for the QGIS browser panel, and includes
 * items for the different data providers and folders accessible to users.
 *
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsBrowserGuiModel : public QgsBrowserModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsBrowserGuiModel, with the specified \a parent object.
     *
     * \note QgsBrowserModel models are not initially populated and use a deferred initialization
     * approach. After constructing a QgsBrowserModel, a call must be made
     * to initialize() in order to populate the model.
     */
    explicit QgsBrowserGuiModel( QObject *parent = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    //! Sets message bar that will be passed in QgsDataItemGuiContext to data items
    void setMessageBar( QgsMessageBar *bar );

  private:
    QgsDataItemGuiContext createDataItemContext() const;
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSBROWSERGUIMODEL_H
