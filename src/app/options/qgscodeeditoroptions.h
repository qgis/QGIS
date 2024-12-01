/***************************************************************************
    qgscodeeditoroptions.h
    -------------------------
    begin                : September 2020
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
#ifndef QGSCODEEDITOROPTIONS_H
#define QGSCODEEDITOROPTIONS_H

#include "ui_qgscodeditorsettings.h"
#include "qgsoptionswidgetfactory.h"
#include "qgscodeeditor.h"

class QgsCodeEditorShell;

/**
 * \ingroup app
 * \class QgsCodeEditorOptionsWidget
 * \brief An options widget showing code editor settings.
 *
 * \since QGIS 3.16
 */
class QgsCodeEditorOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsCodeEditorSettingsBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsCodeEditorOptionsWidget with the specified \a parent widget.
     */
    QgsCodeEditorOptionsWidget( QWidget *parent );
    ~QgsCodeEditorOptionsWidget() override;

    QString helpKey() const override;

    void apply() override;

  private:
    QMap<QgsCodeEditorColorScheme::ColorRole, QgsColorButton *> mColorButtonMap;
    bool mBlockCustomColorChange = false;

    void updatePreview();

    QgsCodeEditorShell *mBashPreview = nullptr;
    QgsCodeEditorShell *mBatchPreview = nullptr;
};


class QgsCodeEditorOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    QgsCodeEditorOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;
    QString pagePositionHint() const override;
};


#endif // QGSCODEEDITOROPTIONS_H
