/***************************************************************************
    qgsmasksourceselectionwidget.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMASKSOURCESELECTIONWIDGET_H
#define QGSMASKSOURCESELECTIONWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QWidget>
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgssymbollayerreference.h"

class QTreeWidget;
class QTreeWidgetItem;
class QgsSymbolLayer;

/**
 * \ingroup gui
 * \brief A widget that allows the selection of a list of sources for selective masking.
 * A masking source can be either a label mask or a mask symbol layer.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMaskSourceSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    struct MaskSource
    {
        //! The source layer id
        QString layerId;

        //! Whether it is a labeling mask or not
        bool isLabeling = false;

        //! The symbol layer id
        QString symbolLayerId;
    };

    //! constructor
    explicit QgsMaskSourceSelectionWidget( QWidget *parent = nullptr );

    //! Updates the possible sources, from the project layers
    void update();

    //! Returns the current selection
    QList<MaskSource> selection() const;

    //! Sets the symbol layer selection
    void setSelection( const QList<MaskSource> &sel );

  signals:
    //! Emitted when an item was changed
    void changed();

  private:
    QTreeWidget *mTree;
    QHash<QgsSymbolLayerReference, QTreeWidgetItem *> mItems;

    friend class TestQgsMaskingWidget;
};

#endif
