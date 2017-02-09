/***************************************************************************
                               qgsnodeeditor.h
                               ---------------
        begin                : Tue Mar 24 2015
        copyright            : (C) 2015 Sandro Mani / Sourcepole AG
        email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNODEEDITOR_H
#define QGSNODEEDITOR_H

#include "qgsdockwidget.h"
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QStyledItemDelegate>

class QgsMapCanvas;
class QgsRubberBand;
class QgsSelectedFeature;
class QgsVectorLayer;
class QTableView;

class QgsNodeEditorModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    QgsNodeEditorModel( QgsVectorLayer* layer,
                        QgsSelectedFeature* selectedFeature,
                        QgsMapCanvas* canvas, QObject* parent = nullptr );

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    virtual QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  private:

    QgsVectorLayer* mLayer;
    QgsSelectedFeature* mSelectedFeature;
    QgsMapCanvas* mCanvas;

    bool mHasZ;
    bool mHasM;
    bool mHasR;

    int mZCol;
    int mMCol;
    int mRCol;

    QFont mWidgetFont;

    bool calcR( int row, double& r, double &minRadius ) const;

  private slots:

    void featureChanged();
};

class QgsNodeEditor : public QgsDockWidget
{
    Q_OBJECT
  public:
    QgsNodeEditor( QgsVectorLayer* layer,
                   QgsSelectedFeature* selectedFeature,
                   QgsMapCanvas* canvas );

  public:
    QgsVectorLayer* mLayer;
    QgsSelectedFeature* mSelectedFeature;
    QgsMapCanvas* mCanvas;
    QTableView* mTableView;
    QgsNodeEditorModel* mNodeModel;

  signals:
    void deleteSelectedRequested( );

  protected:
    void keyPressEvent( QKeyEvent * event );

  private slots:
    void updateTableSelection();
    void updateNodeSelection( const QItemSelection& selected, const QItemSelection& deselected );
    void zoomToNode( int idx );

  private:

    bool mUpdatingTableSelection;
    bool mUpdatingNodeSelection;
};


class CoordinateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QString displayText( const QVariant & value, const QLocale & locale ) const override;

  protected:
    QWidget* createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

#endif // QGSNODEEDITOR_H
