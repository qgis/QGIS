/***************************************************************************
    qgssymbollevelsv2dialog.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSYMBOLLEVELSV2DIALOG_H
#define QGSSYMBOLLEVELSV2DIALOG_H

#include <QDialog>
#include <QList>
#include <QItemDelegate>

#include "qgsrendererv2.h"

#include "ui_qgssymbollevelsv2dialogbase.h"

/** \ingroup gui
 * \class QgsSymbolLevelsV2Dialog
 */
class GUI_EXPORT QgsSymbolLevelsV2Dialog : public QDialog, private Ui::QgsSymbolLevelsV2DialogBase
{
    Q_OBJECT
  public:
    //! @note not available in python bindings
    QgsSymbolLevelsV2Dialog( const QgsLegendSymbolList& list, bool usingSymbolLevels, QWidget* parent = nullptr );

    ~QgsSymbolLevelsV2Dialog();

    bool usingLevels() const;

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    //! @note not available in python bindings
    void populateTable();
    //! @note not available in python bindings
    void setDefaultLevels();

  protected:
    //! maximal number of layers from all symbols
    int mMaxLayers;
    QgsLegendSymbolList mList;
    //! whether symbol layers always should be used (default false)
    bool mForceOrderingEnabled;
};

///@cond PRIVATE

// delegate used from Qt Spin Box example
class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit SpinBoxDelegate( QObject *parent = nullptr ) : QItemDelegate( parent ) {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &/*index*/ ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/ ) const override;

};

///@endcond

#endif // QGSSYMBOLLEVELSV2DIALOG_H
