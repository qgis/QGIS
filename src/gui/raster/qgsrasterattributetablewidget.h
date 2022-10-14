/***************************************************************************
  qgsrasterattributetablewidget.h - QgsRasterAttributeTableWidget

 ---------------------
 begin                : 6.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLEWIDGET_H
#define QGSRASTERATTRIBUTETABLEWIDGET_H

#include "qgis_gui.h"
#include "ui_qgsrasterattributetablewidgetbase.h"
#include "qgsrasterattributetablemodel.h"
#include "qgscolorrampimpl.h"

#include <QWidget>
#include <QStyledItemDelegate>


#ifndef SIP_RUN
class QgsRasterLayer;
class QgsRasterAttributeTable;
class QgsMessageBar;
class QSortFilterProxyModel;
#endif


///@cond private
#ifndef SIP_RUN
class ColorDelegate: public QStyledItemDelegate
{

  public:

    ColorDelegate( QObject *parent = nullptr ): QStyledItemDelegate( parent ) {};

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

class ColorAlphaDelegate: public ColorDelegate
{

  public:

    ColorAlphaDelegate( QObject *parent = nullptr ): ColorDelegate( parent ) {};

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

class ColorRampDelegate: public QStyledItemDelegate
{

  public:

    ColorRampDelegate( QObject *parent = nullptr ): QStyledItemDelegate( parent ) {};

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  protected:

    QgsGradientColorRamp mRamp;

};

class ColorRampAlphaDelegate: public ColorRampDelegate
{

  public:

    ColorRampAlphaDelegate( QObject *parent = nullptr ): ColorRampDelegate( parent ) {};

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};
#endif
///@endcond

/**
 * \ingroup gui
 * \brief The QgsRasterAttributeTableWidget class provides an attribute table for rasters and methods to edit the table.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsRasterAttributeTableWidget : public QWidget, private Ui::QgsRasterAttributeTableWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsRasterAttributeTableWidget
     * \param parent parent widget
     * \param rasterLayer raster layer
     * \param bandNumber optional initial selected band number (default to 0, which makes the widget use the first available RAT, if any)
     * \param parent parent widget
     */
    explicit QgsRasterAttributeTableWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsRasterLayer *rasterLayer = nullptr, const int bandNumber = 0 );

    /**
     * Sets the raster layer and an optional band number.
     * \param rasterLayer raster layer
     * \param bandNumber optional initial selected band number (default to 0, which makes the widget use the first available RAT, if any)
     */
    void setRasterLayer( QgsRasterLayer *rasterLayer, const int bandNumber = 0 );

    /**
     * Returns TRUE if the associated raster attribute table is dirty
     */
    bool isDirty( ) const;

  private:

    QgsRasterLayer *mRasterLayer = nullptr;
    QgsRasterAttributeTable *mAttributeTable = nullptr;
    // Default to invalid (bands are 1-indexed)
    int mCurrentBand = 0;
    bool mEditable = false;

    std::unique_ptr<QgsRasterAttributeTableModel> mModel;

    QAction *mActionToggleEditing = nullptr;
    QAction *mActionAddColumn = nullptr;
    QAction *mActionRemoveColumn = nullptr;
    QAction *mActionAddRow = nullptr;
    QAction *mActionRemoveRow = nullptr;
    QAction *mActionSaveChanges = nullptr;

    QgsMessageBar *mMessageBar = nullptr;
    QSortFilterProxyModel *mProxyModel = nullptr;

    void init( int bandNumber = 0 );
    void updateButtons();

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );


  private slots:

    void setEditable( bool editable );
    void saveChanges();
    void classify();
    void addColumn();
    void removeColumn();
    void addRow();
    void removeRow();
    void bandChanged( const int index );
    void notify( const QString &title, const QString &message, Qgis::MessageLevel level = Qgis::MessageLevel::Info );

};

#endif // QGSRASTERATTRIBUTETABLEWIDGET_H
