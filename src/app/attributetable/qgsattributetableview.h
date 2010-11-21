/***************************************************************************
     QgsAttributeTableView.h
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEVIEW_H
#define QGSATTRIBUTETABLEVIEW_H

#include <QTableView>
#include <QAction>

class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;

class QgsVectorLayer;
class QMenu;


class QgsAttributeTableView : public QTableView
{
    Q_OBJECT

  public:
    QgsAttributeTableView( QWidget* parent = NULL );
    virtual ~QgsAttributeTableView();

    /**
     * Sets the layer
     * @param layer layer pointer
     */
    void setLayer( QgsVectorLayer* layer );

    /**
     * Saves geometry to the settings on close
     * @param event not used
     */
    void closeEvent( QCloseEvent *event );

    void contextMenuEvent( QContextMenuEvent* );

  private:
    QgsAttributeTableModel* mModel;
    QgsAttributeTableFilterModel* mFilterModel;
    QMenu *mActionPopup;
};

class QgsAttributeTableAction : public QAction
{
    Q_OBJECT

  public:
    QgsAttributeTableAction( const QString &name, QgsAttributeTableView *view, QgsAttributeTableModel *model, int action, const QModelIndex &fieldIdx ) :
        QAction( name, view ), mModel( model ), mAction( action ), mFieldIdx( fieldIdx )
    {}

  public slots:
    void execute();
    void featureForm();

  private:
    QgsAttributeTableModel *mModel;
    int mAction;
    QModelIndex mFieldIdx;
};


#endif
