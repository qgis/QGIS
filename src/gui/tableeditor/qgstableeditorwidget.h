// This file is part of CppSheets.
//
// Copyright 2018 Patrick Flynn <patrick_dev2000@outlook.com>
//
// CppSheets is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppSheets is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CppSheets. If not, see <https://www.gnu.org/licenses/>.

#ifndef QGSTABLEEDITORWIDGET_H
#define QGSTABLEEDITORWIDGET_H

#include "qgis_gui.h"
#include "qgstablecell.h"
#include <QTableWidget>

class GUI_EXPORT QgsTableEditorWidget : public QTableWidget
{
    Q_OBJECT
  public:

    enum Roles
    {
      PresetBackgroundColorRole = Qt::UserRole + 1,

    };

    QgsTableEditorWidget( QWidget *parent = nullptr );
    ~QgsTableEditorWidget();

    void setTableData( const QgsTableContents &contents );
    QgsTableContents tableData() const;

    void setCellNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    QColor selectedCellForegroundColor();

    QColor selectedCellBackgroundColor();

  public slots:

    void insertRowsBelow();
    void insertRowsAbove();
    void insertColumnsBefore();
    void insertColumnsAfter();
    void deleteRows();
    void deleteColumns();

    void selectRows();
    void selectColumns();
    void clearSelectedCells();

    void setCellForegroundColor( const QColor &color );
    void setCellBackgroundColor( const QColor &color );


  protected:
    void keyPressEvent( QKeyEvent *event ) override;

  signals:

    void tableChanged();
    void activeCellChanged();

  private:
    void updateHeaders();

    bool collectConsecutiveRowRange( const QModelIndexList &list, int &minRow, int &maxRow ) const;
    bool collectConsecutiveColumnRange( const QModelIndexList &list, int &minColumn, int &maxColumn ) const;
    QList< int > collectUniqueRows( const QModelIndexList &list ) const;
    QList< int > collectUniqueColumns( const QModelIndexList &list ) const;

    bool mBlockSignals = false;
    QHash< QTableWidgetItem *, QgsNumericFormat * > mNumericFormats;

    QMenu *mHeaderMenu = nullptr;

};

#endif // QGSTABLEEDITORWIDGET_H
