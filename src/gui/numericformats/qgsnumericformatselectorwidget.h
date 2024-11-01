/***************************************************************************
    qgsnumericformatselectorwidget.h
    --------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNUMERICFORMATSELECTORWIDGET_H
#define QGSNUMERICFORMATSELECTORWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"
#include "ui_qgsnumericformatselectorbase.h"
#include <memory>
#include <QDialog>

class QgsNumericFormat;
class QgsBasicNumericFormat;
class QgsExpressionContextGenerator;
class QDialogButtonBox;

/**
 * \ingroup gui
 * \class QgsNumericFormatSelectorWidget
 * \brief A widget which allows choice of numeric formats and the properties of them.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsNumericFormatSelectorWidget : public QgsPanelWidget, private Ui::QgsNumericFormatSelectorBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsNumericFormatSelectorWidget with the specified \a parent widget.
     */
    QgsNumericFormatSelectorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsNumericFormatSelectorWidget() override;

    /**
     * Sets the format to show in the widget.
     */
    void setFormat( const QgsNumericFormat *format );

    /**
     * Returns a new format object representing the settings currently configured in the widget.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *format() const SIP_TRANSFERBACK;

    /**
     * Register an expression context \a generator class that will be used to retrieve
     * an expression context for the widget when required.
     *
     * Ownership is not transferred, and the \a generator must exist for the lifetime of this widget.
     *
     * \since QGIS 3.40
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

  signals:

    /**
     * Emitted whenever the format configured55 in the widget is changed.
     */
    void changed();

  private slots:
    void formatTypeChanged();
    void formatChanged();

  private:
    void populateTypes();
    void updateFormatWidget();
    void updateSampleText();

    std::unique_ptr<QgsNumericFormat> mCurrentFormat;
    std::unique_ptr<QgsBasicNumericFormat> mPreviewFormat;

    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;
};


/**
 * \class QgsNumericFormatSelectorDialog
 * \ingroup gui
 * \brief A simple dialog for customizing a numeric format.
 *
 * \see QgsNumericFormatSelectorWidget()
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsNumericFormatSelectorDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsNumericFormatSelectorDialog.
     * \param parent parent widget
     * \param flags window flags for dialog
     */
    QgsNumericFormatSelectorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets the format to show in the dialog.
     */
    void setFormat( const QgsNumericFormat *format );

    /**
     * Returns a new format object representing the settings currently configured in the dialog.
     *
     * The caller takes ownership of the returned object.
     */
    QgsNumericFormat *format() const SIP_TRANSFERBACK;

    /**
     * Register an expression context \a generator class that will be used to retrieve
     * an expression context for the dialog when required.
     *
     * Ownership is not transferred, and the \a generator must exist for the lifetime of this dialog.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

  private:
    QgsNumericFormatSelectorWidget *mFormatWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif //QGSNUMERICFORMATSELECTORWIDGET_H
