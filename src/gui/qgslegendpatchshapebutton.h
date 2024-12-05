/***************************************************************************
     qgslegendpatchshapebutton.h
     -----------------
    Date                 : April 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLEGENDPATCHSHAPEBUTTON_H
#define QGSLEGENDPATCHSHAPEBUTTON_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgslegendpatchshape.h"
#include <QToolButton>
#include <QPointer>
#include <memory>

class QgsPanelWidget;
class QgsMessageBar;
class QgsSymbol;

/**
 * \ingroup gui
 * \class QgsLegendPatchShapeButton
 * \brief A button for creating and modifying QgsLegendPatchShape settings.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsLegendPatchShapeButton : public QToolButton
{
    Q_OBJECT

  public:
    /**
     * Construct a new patch shape button with the specified \a parent widget.
     * Use \a dialogTitle string to define the title to show in the legend patch shape widget.
     */
    QgsLegendPatchShapeButton( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &dialogTitle = QString() );
    ~QgsLegendPatchShapeButton() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    /**
     * Sets the symbol \a type which the button requires.
     * If the type differs from the current symbol type, the symbol will be reset
     * to a default symbol style of the new type.
     * \see symbolType()
     */
    void setSymbolType( Qgis::SymbolType type );

    /**
     * Returns the symbol type which the button requires.
     * \see setSymbolType()
     */
    Qgis::SymbolType symbolType() const { return mType; }

    /**
     * Sets the symbol to use for previewing the legend patch shape.
     *
     * Ownership is transferred to the button. It is the caller's responsibility
     * to ensure that the symbol type matches the button's symbolType()
     */
    void setPreviewSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Sets the \a title for the symbol settings dialog window.
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the symbol settings dialog window.
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

    /**
    * Returns the current shape defined by the button.
    * \see setShape()
    * \see changed()
    */
    QgsLegendPatchShape shape();

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     * \since QGIS 3.6
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget.
     * \see setMessageBar()
     * \since QGIS 3.6
     */
    QgsMessageBar *messageBar() const;

  public slots:

    /**
     * Sets the \a shape for the button.
     * \see shape()
     * \see changed()
     */
    void setShape( const QgsLegendPatchShape &shape );

    /**
     * Resets the shape to the default shape.
     */
    void setToDefault();

  signals:

    /**
     * Emitted when the shape's settings are changed.
     * \see shape()
     * \see setShape()
     */
    void changed();

  protected:
    void changeEvent( QEvent *e ) override;
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent *event ) override;

    // Reimplemented to detect right mouse button clicks on the button
    void mousePressEvent( QMouseEvent *e ) override;

  private slots:

    void showSettingsDialog();

    /**
     * Creates the drop-down menu entries
     */
    void prepareMenu();

    void loadPatchFromStyle( const QString &name );

  private:
    QgsLegendPatchShape mShape;

    QSize mSizeHint;

    QString mDialogTitle;

    Qgis::SymbolType mType = Qgis::SymbolType::Fill;

    std::unique_ptr<QgsSymbol> mPreviewSymbol;

    QgsMessageBar *mMessageBar = nullptr;

    QMenu *mMenu = nullptr;

    QSize mIconSize;

    bool mIsDefault = true;

    /**
     * Regenerates the shape preview.
     */
    void updatePreview();
};

#endif // QGSLEGENDPATCHSHAPEBUTTON_H
