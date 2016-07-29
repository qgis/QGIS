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

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>

#include "qgsmaplayerconfigwidget.h"

class QgsMapLayer;
class QgsMapCanvas;


/** \ingroup gui
 * @brief The QgsMapLayerStyleManagerWidget class which is used to visually manage
 * the layer styles.
 */
class GUI_EXPORT QgsMapLayerStyleManagerWidget : public QgsMapLayerConfigWidget
{
    Q_OBJECT
  public:

    /**
     * @brief Style manager widget to manage the layers styles.
     * @param layer The layer for the widget
     * @param canvas The canvas object.
     * @param parent The parent.
     */
    QgsMapLayerStyleManagerWidget( QgsMapLayer* layer, QgsMapCanvas* canvas, QWidget *parent = 0 );

  public slots:
    void apply() override {}

  private slots:
    void styleClicked( QModelIndex index );
    void currentStyleChanged( QString name );
    void styleAdded( QString name );
    void styleRemoved( QString name );
    void styleRenamed( QString oldname, QString newname );
    void addStyle();
    void removeStyle();
    void saveAsDefault();
    void loadDefault();
    void saveStyle();
    void loadStyle();

  private:
    QStandardItemModel* mModel;
    QListView* mStyleList;
};

#endif // QGSMAPLAYERSTYLEMANAGERWIDGET_H
