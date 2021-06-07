/***************************************************************************
    qgsmaplayerstylemanagerwidget.h
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPLAYERSTYLEMANAGERWIDGET_H
#define QGSMAPLAYERSTYLEMANAGERWIDGET_H

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>

#include "qgsmaplayerconfigwidget.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QgsMapCanvas;


/**
 * \ingroup gui
 * \brief The QgsMapLayerStyleManagerWidget class which is used to visually manage
 * the layer styles.
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

  public slots:
    void apply() override {}

  private slots:
    void styleClicked( const QModelIndex &index );
    void currentStyleChanged( const QString &name );
    void styleAdded( const QString &name );
    void styleRemoved( const QString &name );
    void styleRenamed( const QString &oldname, const QString &newname );
    void addStyle();
    void removeStyle();
    void renameStyle( QStandardItem *item );
    void saveAsDefault();
    void loadDefault();
    void saveStyle();
    void loadStyle();

  private:
    QStandardItemModel *mModel = nullptr;
    QListView *mStyleList = nullptr;
};

#endif // QGSMAPLAYERSTYLEMANAGERWIDGET_H
