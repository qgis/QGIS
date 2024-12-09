/***************************************************************************
    qgsextentbufferdialog.h
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Juho Ervasti
    email                : juho dot ervasti at gispo dot fi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXTENTBUFFERDIALOG_H
#define QGSEXTENTBUFFERDIALOG_H

#define SIP_NO_FILE

#include <QDialog>
#include "qgis_sip.h"

#include "qgssymbol.h"
#include "qgssymbolwidgetcontext.h"
#include "ui_qgsextentbufferdialogbase.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \class QgsExtentBufferWidget
 * \ingroup gui
 * \brief A widget which allows the user to modify the rendering order of extent buffers.
 * \see QgsExtentBufferDialog
 * \since QGIS 3.42
 */
class GUI_EXPORT QgsExtentBufferWidget : public QgsPanelWidget, public QgsExpressionContextGenerator, private Ui::QgsExtentBufferDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsExtentBufferWidget
     */
    QgsExtentBufferWidget( QgsSymbol *symbol, QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the extent buffer value currently set in the widget.
     *
     * \returns extent buffer value
     */
    double extentBuffer() const;

    /**
     * Returns the data defined property currently set in the widget.
     *
     * \returns property
     */
    QgsProperty dataDefinedProperty() const;

    /**
     * Sets the context in which widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the extent buffer unit currently set in the widget.
     * \see setContext()
     */
    Qgis::RenderUnit sizeUnit() const;

  private:
    QgsSymbol *mSymbol = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsSymbolWidgetContext mContext;

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbol::Property key );
};

/**
 * \class QgsExtentBufferDialog
 * \ingroup gui
 * \brief A dialog which allows the user to modify the extent buffer of a symbol.
 * \since QGIS 3.42
*/
class GUI_EXPORT QgsExtentBufferDialog : public QDialog
{
    Q_OBJECT
  public:
    //! Constructor for QgsExtentBufferDialog.
    QgsExtentBufferDialog( QgsSymbol *symbol, QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the extent buffer value currently set in the widget.
     */
    double extentBuffer() const;

    /**
     * Returns the extent buffer unit currently set in the widget.
     */
    Qgis::RenderUnit sizeUnit() const;

    /**
     * Returns the extent buffer value currently set in the widget.
     *
     * \note returns 0 if widget does not exist
     *
     * \returns extent buffer value
     */
    QgsProperty dataDefinedProperty() const;

    /**
     * Returns the data defined property currently set in the widget.
     *
     * \note returns empty property if widget does not exist
     *
     * \returns property
    */
    QgsExtentBufferWidget *widget() const;

    /**
     * Sets the context in which widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     */
    void setContext( const QgsSymbolWidgetContext &context );

  private:
    QgsExtentBufferWidget *mWidget;

  private slots:

    void showHelp();
};

#endif // QGSEXTENTBUFFERDIALOG_H
