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

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgssymbollayerreference.h"

#include <QWidget>

#define SIP_NO_FILE

class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QToolButton;
class QgsSymbolLayer;
class QgsSelectiveMaskingSource;
class QgsSelectiveMaskingSourceSet;
class QgsSelectiveMaskingSourceSetManagerModel;
class QgsSelectiveMaskingSourceSetManagerProxyModel;

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
    //! constructor
    explicit QgsMaskSourceSelectionWidget( QWidget *parent = nullptr );

    //! Updates the possible sources, from the project layers
    void update();

    /**
     * Returns the current masking source set.
     *
     * \see setSourceSet()
     */
    QgsSelectiveMaskingSourceSet sourceSet() const;

    /**
     * Sets the current masking source \a set.
     *
     * \see sourceSet()
     */
    void setSourceSet( const QgsSelectiveMaskingSourceSet &set );

  signals:
    //! Emitted when an item was changed
    void changed();

  private slots:

    void selectedSetChanged();
    void emitChanged();
    void onItemChanged();
    void newSet();
    void removeSet();
    void renameSet();

  private:
    bool isCustomSet() const;

    QComboBox *mSetComboBox = nullptr;
    QTreeWidget *mTree = nullptr;
    QHash<QgsSymbolLayerReference, QTreeWidgetItem *> mItems;
    QgsSelectiveMaskingSourceSetManagerModel *mManagerModel = nullptr;
    QgsSelectiveMaskingSourceSetManagerProxyModel *mManagerProxyModel = nullptr;
    QToolButton *mSetsToolButton = nullptr;
    QAction *mRemoveAction = nullptr;
    QAction *mRenameAction = nullptr;

    int mBlockChangedSignals = 0;

    friend class TestQgsMaskingWidget;
};

#endif
