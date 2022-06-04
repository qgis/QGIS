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

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QStyledItemDelegate>

#include "qgis_app.h"
#include "qgsdockwidget.h"
#include "qgspoint.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvertexid.h"
#include "qgssettingsentryimpl.h"
#include "qgssettings.h"
#include "qgspanelwidget.h"

class QLabel;
class QTableView;

class QgsMapCanvas;
class QgsLockedFeature;
class QgsVectorLayer;
class QCheckBox;
class QStackedWidget;

class APP_EXPORT QgsVertexEntry
{
  public:
    QgsVertexEntry( const QgsPoint &p,
                    QgsVertexId vertexId )
      : mSelected( false )
      , mPoint( p )
      , mVertexId( vertexId )
    {
    }

    QgsVertexEntry( const QgsVertexEntry &rh ) = delete;
    QgsVertexEntry &operator=( const QgsVertexEntry &rh ) = delete;

    const QgsPoint &point() const { return mPoint; }
    QgsVertexId vertexId() const { return mVertexId; }
    bool isSelected() const { return mSelected; }
    void setSelected( bool selected ) { mSelected = selected; }

  private:
    bool mSelected;
    QgsPoint mPoint;
    QgsVertexId mVertexId;
};

class APP_EXPORT QgsVertexEditorModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    QgsVertexEditorModel( QgsMapCanvas *canvas, QObject *parent = nullptr );

    void setFeature( QgsLockedFeature *lockedFeature );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  private:
    QgsLockedFeature *mLockedFeature = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    bool mHasZ = false;
    bool mHasM = false;
    bool mHasR = true; //always show for now - avoids scanning whole feature for curves TODO - avoid this

    int mZCol = -1;
    int mMCol = -1;
    int mRCol = -1;

    QFont mWidgetFont;

    bool calcR( int row, double &r, double &minRadius ) const;

};

class APP_EXPORT QgsVertexEditorWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    QgsVertexEditorWidget( QgsMapCanvas *canvas );

    void updateEditor( QgsLockedFeature *lockedFeature );
    QgsLockedFeature *mLockedFeature = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QTableView *mTableView = nullptr;
    QgsVertexEditorModel *mVertexModel = nullptr;

    QMenu *menuButtonMenu() override;
    QString menuButtonTooltip() const override;

  signals:
    void deleteSelectedRequested();

  protected:
    void keyPressEvent( QKeyEvent *event ) override;

  private slots:
    void updateTableSelection();
    void updateVertexSelection( const QItemSelection &, const QItemSelection &deselected );

  private:

    QLabel *mHintLabel = nullptr;
    QStackedWidget *mStackedWidget = nullptr;
    QWidget *mPageHint = nullptr;
    QWidget *mPageTable = nullptr;

    QMenu *mWidgetMenu = nullptr;

    bool mUpdatingTableSelection = false;
    bool mUpdatingVertexSelection = false;
};

class APP_EXPORT QgsVertexEditor : public QgsDockWidget
{
    Q_OBJECT
  public:

    static const inline QgsSettingsEntryBool settingAutoPopupVertexEditorDock = QgsSettingsEntryBool( QStringLiteral( "auto_popup_vertex_editor_dock" ), QgsSettings::Prefix::QGIS_DIGITIZING, true, QStringLiteral( "Whether the auto-popup behavior of the vertex editor dock should be enabled" ) );

    QgsVertexEditor( QgsMapCanvas *canvas );

    void updateEditor( QgsLockedFeature *lockedFeature );

  signals:
    void deleteSelectedRequested();
    void editorClosed();

  protected:
    void closeEvent( QCloseEvent *event ) override;

  private:

    QgsVertexEditorWidget *mWidget = nullptr;

};


class APP_EXPORT CoordinateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    explicit CoordinateItemDelegate( const QgsCoordinateReferenceSystem &crs, QObject *parent = nullptr );

    QString displayText( const QVariant &value, const QLocale &locale ) const override;

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

  private:
    //! Returns number of decimal places to display after the dot
    int displayDecimalPlaces() const;
    QgsCoordinateReferenceSystem mCrs;
};



#endif // QGSVERTEXEDITOR_H
