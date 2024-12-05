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
 * \brief A widget which allows the user to modify the rendering order of symbol layers.
 * \see QgsSymbolLevelsDialog
 */
class GUI_EXPORT QgsSymbolLevelsWidget : public QgsPanelWidget, private Ui::QgsSymbolLevelsDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsSymbolLevelsWidget
     */
    QgsSymbolLevelsWidget( QgsFeatureRenderer *renderer, bool usingSymbolLevels, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsSymbolLevelsWidget, which takes a list of \a symbols to show in the dialog.
     *
     * \since QGIS 3.20
     */
    QgsSymbolLevelsWidget( const QgsLegendSymbolList &symbols, bool usingSymbolLevels, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Returns whether the level ordering is enabled
    bool usingLevels() const;

    /**
     * Returns the current legend symbols with rendering passes set, as defined in the widget.
     *
     * \since QGIS 3.20
     */
    QgsLegendSymbolList symbolLevels() const;

    /**
     * Sets whether the level ordering is always forced on and hide the checkbox (used by rule-based renderer)
     * \param enabled toggle level ordering
     */
    void setForceOrderingEnabled( bool enabled );

  public slots:

    /**
     * Apply button.
     *
     * \deprecated QGIS 3.20. Use symbolLevels() and manually apply the changes to the renderer as appropriate.
     */
    Q_DECL_DEPRECATED void apply() SIP_DEPRECATED;

  private slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  private:
    void populateTable();
    void setDefaultLevels();

    //! maximal number of layers from all symbols
    int mMaxLayers = 0;

    QgsFeatureRenderer *mRenderer = nullptr;
    QgsLegendSymbolList mLegendSymbols;

    //! whether symbol layers always should be used (default FALSE)
    bool mForceOrderingEnabled = false;
};


/**
 * \class QgsSymbolLevelsDialog
 * \ingroup gui
 * \brief A dialog which allows the user to modify the rendering order of symbol layers.
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

    /**
     * Returns whether the level ordering is enabled.
     *
     * \since QGIS 3.20
     */
    bool usingLevels() const;

    /**
     * Returns the current legend symbols with rendering passes set, as defined in the widget.
     *
     * \since QGIS 3.20
     */
    QgsLegendSymbolList symbolLevels() const;

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
    explicit SpinBoxDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent ) {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/ ) const override;
};

///@endcond
#endif

#endif // QGSSYMBOLLEVELSDIALOG_H
