/***************************************************************************
    qgsmaplayerstylemanagerwidget.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPLAYERSTYLEMANAGERWIDGET_H
#define QGSMAPLAYERSTYLEMANAGERWIDGET_H

#include "qgis_gui.h"
#include "qgsmaplayerconfigwidget.h"

#include <QAbstractListModel>
#include <QListView>
#include <QWidget>

class QgsMapLayer;
class QgsMapCanvas;

class QStandardItem;

/**
 * \ingroup gui
 * \brief A model for display of styles from a QgsMapLayerStyleManager.
 *
 * Also allows for renaming of styles.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsMapLayerStyleModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsMapLayerStyleModel, showing styles from the specified \a manager.
     */
    QgsMapLayerStyleModel( QgsMapLayerStyleManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns the model index corresponding to the specified style name.
     */
    QModelIndex indexForName( const QString &name ) const;

  private slots:

    void styleAdded( const QString &name );
    void styleRemoved( const QString &name );
    void styleRenamed( const QString &oldname, const QString &newname );

  private:
    QPointer< QgsMapLayerStyleManager > mManager;
    QStringList mStyleNames;
};

/**
 * \ingroup gui
 * \brief A widget which is used to visually manage the layer styles.
 */
class GUI_EXPORT QgsMapLayerStyleManagerWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:
    /**
     * \brief Style manager widget to manage the layers styles.
     * \param layer The layer for the widget
     * \param canvas The canvas object.
     * \param parent The parent.
     */
    QgsMapLayerStyleManagerWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    void syncToLayer( QgsMapLayer *layer ) final;

  public slots:
    void apply() override {}

  private slots:
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void currentStyleChanged( const QString &name );
    void addStyle();
    void removeStyle();
    void saveAsDefault();
    void loadDefault();
    void saveStyle();
    void loadStyle();

  private:
    QgsMapLayerStyleModel *mModel = nullptr;
    QListView *mStyleList = nullptr;
};

#endif // QGSMAPLAYERSTYLEMANAGERWIDGET_H
