/***************************************************************************
                             qgslayoutguidewidget.h
                             ----------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTGUIDEWIDGET_H
#define QGSLAYOUTGUIDEWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutguidewidgetbase.h"
#include "qgspanelwidget.h"
#include <QStyledItemDelegate>


class QgsLayoutView;
class QgsLayout;
class QgsLayoutGuideProxyModel;

/**
 * \ingroup gui
 * \brief Widget for managing the layout guides
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutGuideWidget : public QgsPanelWidget, private Ui::QgsLayoutGuideWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsLayoutGuideWidget( QWidget *parent, QgsLayout *layout, QgsLayoutView *layoutView );

  public slots:

    /**
     * Sets the current page number to manage the guides for.
     */
    void setCurrentPage( int page );

  private slots:

    void addHorizontalGuide();
    void addVerticalGuide();

    void deleteHorizontalGuide();
    void deleteVerticalGuide();

    void clearAll();

    void applyToAll();

    void updatePageCount();

  private:
    QgsLayout *mLayout = nullptr;
    QgsLayoutGuideProxyModel *mHozProxyModel = nullptr;
    QgsLayoutGuideProxyModel *mVertProxyModel = nullptr;
    int mPage = 0;
};

/**
 * \ingroup gui
 * \brief View delegate displaying a QgsDoubleSpinBox for the layout guide position
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutGuidePositionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    QgsLayoutGuidePositionDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

/**
 * \ingroup gui
 * \brief View delegate displaying a QgsLayoutUnitsComboBox for the layout guide unit
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutGuideUnitDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    //! constructor
    QgsLayoutGuideUnitDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

#endif // QGSLAYOUTGUIDEWIDGET_H
