/***************************************************************************
                         qgspalettedrendererwidget.h
                         ---------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALETTEDRENDERERWIDGET_H
#define QGSPALETTEDRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgspalettedrasterrenderer.h"
#include "qgscolorschemelist.h"
#include "ui_qgspalettedrendererwidgetbase.h"
#include "qgis_gui.h"

class QgsRasterLayer;

/// @cond PRIVATE
class QgsPalettedRendererModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    enum Column
    {
      ValueColumn = 0,
      ColorColumn = 1,
      LabelColumn = 2,
    };

    QgsPalettedRendererModel( QObject *parent = nullptr );

    void setClassData( const QgsPalettedRasterRenderer::ClassData &data );

    QgsPalettedRasterRenderer::ClassData classData() const { return mData; }

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    void addEntry( const QColor &color );

  signals:

    void classesChanged();

  private:

    QgsPalettedRasterRenderer::ClassData mData;


};
///@endcond PRIVATE

/** \ingroup gui
 * \class QgsPalettedRendererWidget
 */
class GUI_EXPORT QgsPalettedRendererWidget: public QgsRasterRendererWidget, private Ui::QgsPalettedRendererWidgetBase
{
    Q_OBJECT

  public:

    QgsPalettedRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) { return new QgsPalettedRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() override;

    void setFromRenderer( const QgsRasterRenderer *r );

  private:

    QMenu *contextMenu = nullptr;
    QgsPalettedRendererModel *mModel = nullptr;
    QgsColorSwatchDelegate *mSwatchDelegate = nullptr;

    void setSelectionColor( const QItemSelection &selection, const QColor &color );

  private slots:

    void deleteEntry();
    void addEntry();
    void changeColor();
    void changeTransparency();
    void changeLabel();
    void applyColorRamp();
    void loadColorTable();
    void saveColorTable();
    void classify();

};

#endif // QGSPALETTEDRENDERERWIDGET_H
