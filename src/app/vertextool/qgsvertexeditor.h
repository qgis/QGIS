/***************************************************************************
                               qgsvertexeditor.h
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

#ifndef QGSVERTEXEDITOR_H
#define QGSVERTEXEDITOR_H

#include "qgsdockwidget.h"
#include <QAbstractTableModel>
#include <QItemSelection>
#include <QStyledItemDelegate>

class QgsMapCanvas;
class QgsRubberBand;
class QgsSelectedFeature;
class QgsVectorLayer;
class QTableView;

class QgsVertexEditorModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    QgsVertexEditorModel( QgsVectorLayer *layer,
                          QgsSelectedFeature *selectedFeature,
                          QgsMapCanvas *canvas, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  private:

    QgsVectorLayer *mLayer = nullptr;
    QgsSelectedFeature *mSelectedFeature = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    bool mHasZ;
    bool mHasM;
    bool mHasR;

    int mZCol;
    int mMCol;
    int mRCol;

    QFont mWidgetFont;

    bool calcR( int row, double &r, double &minRadius ) const;

};

class QgsVertexEditor : public QgsDockWidget
{
    Q_OBJECT
  public:
    QgsVertexEditor( QgsVectorLayer *layer,
                     QgsSelectedFeature *selectedFeature,
                     QgsMapCanvas *canvas );

  public:
    void updateEditor( QgsVectorLayer *layer, QgsSelectedFeature *selectedFeature );
    QgsVectorLayer *mLayer = nullptr;
    QgsSelectedFeature *mSelectedFeature = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QTableView *mTableView = nullptr;
    QgsVertexEditorModel *mVertexModel = nullptr;

  signals:
    void deleteSelectedRequested();
    void editorClosed();

  protected:
    void keyPressEvent( QKeyEvent *event ) override;
    void closeEvent( QCloseEvent *event ) override;

  private slots:
    void updateTableSelection();
    void updateVertexSelection( const QItemSelection &selected, const QItemSelection &deselected );

  private:

    bool mUpdatingTableSelection = false;
    bool mUpdatingVertexSelection = false;
};


class CoordinateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    explicit CoordinateItemDelegate( QObject *parent = nullptr );

    QString displayText( const QVariant &value, const QLocale &locale ) const override;

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

#endif // QGSVERTEXEDITOR_H
