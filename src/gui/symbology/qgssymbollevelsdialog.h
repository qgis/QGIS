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

#include "qgshelp.h"
#include "qgspanelwidget.h"
#include "qgsrenderer.h"

#include "ui_qgssymbollevelsdialogbase.h"
#include "qgis_gui.h"

/**
 * \class QgsSymbolLevelsWidget
 * \ingroup gui
 * A widget which allows the user to modify the rendering order of symbol layers.
 * \see QgsSymbolLevelsDialog
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSymbolLevelsWidget : public QgsPanelWidget, private Ui::QgsSymbolLevelsDialogBase
{
    Q_OBJECT
  public:
    //! Constructor for QgsSymbolLevelsWidget
    QgsSymbolLevelsWidget( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns whether the level ordering is enabled
    bool usingLevels() const;

    /**
     * Sets whether the level ordering is always forced on and hide the checkbox (used by rule-based renderer)
     * \param enabled toggle level ordering
     */
    void setForceOrderingEnabled( bool enabled );

  public slots:
    //! Apply button
    void apply();

  private slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    //! \note not available in Python bindings
    void populateTable() SIP_SKIP;
    //! \note not available in Python bindings
    void setDefaultLevels() SIP_SKIP;

    //! maximal number of layers from all symbols
    int mMaxLayers;

    QgsFeatureRenderer *mRenderer = nullptr;
    QgsLegendSymbolList mList;

    //! whether symbol layers always should be used (default FALSE)
    bool mForceOrderingEnabled;

  private:
#ifdef SIP_RUN
    QgsSymbolLevelsWidget();

#endif
};

/**
 * \class QgsSymbolLevelsDialog
 * \ingroup gui
 * A dialog which allows the user to modify the rendering order of symbol layers.
 * \see QgsSymbolLevelsWidget
*/
class GUI_EXPORT QgsSymbolLevelsDialog : public QDialog
{
    Q_OBJECT
  public:

    //! Constructor for QgsSymbolLevelsDialog.
    QgsSymbolLevelsDialog( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  private:

    QgsSymbolLevelsWidget *mWidget = nullptr;

  private slots:

    void showHelp();
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
