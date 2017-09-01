/***************************************************************************
    qgssymbollevelsdialog.h
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
#ifndef QGSSYMBOLLEVELSDIALOG_H
#define QGSSYMBOLLEVELSDIALOG_H

#include <QDialog>
#include "qgis_sip.h"
#include <QList>
#include <QItemDelegate>

#include "qgsrenderer.h"

#include "ui_qgssymbollevelsdialogbase.h"
#include "qgis_gui.h"

/** \ingroup gui
 * \class QgsSymbolLevelsDialog
 */
class GUI_EXPORT QgsSymbolLevelsDialog : public QDialog, private Ui::QgsSymbolLevelsDialogBase
{
    Q_OBJECT
  public:
    //! \note not available in Python bindings
    QgsSymbolLevelsDialog( const QgsLegendSymbolList &list, bool usingSymbolLevels, QWidget *parent = nullptr ) SIP_SKIP;

    ~QgsSymbolLevelsDialog();

    bool usingLevels() const;

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    //! \note not available in Python bindings
    void populateTable() SIP_SKIP;
    //! \note not available in Python bindings
    void setDefaultLevels() SIP_SKIP;

    //! maximal number of layers from all symbols
    int mMaxLayers;
    QgsLegendSymbolList mList;
    //! whether symbol layers always should be used (default false)
    bool mForceOrderingEnabled;

  private:
#ifdef SIP_RUN
    QgsSymbolLevelsDialog();
#endif
};

#ifndef SIP_RUN
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
#endif

#endif // QGSSYMBOLLEVELSDIALOG_H
