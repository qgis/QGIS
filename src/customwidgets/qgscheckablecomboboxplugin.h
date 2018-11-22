/***************************************************************************
   qgscheckablecomboboxplugin.h
    --------------------------------------
   Date                 : March 22, 2017
   Copyright            : (C) 2017 Alexander Bruy
   Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSCHECKABLECOMBOBOXPLUGIN_H
#define QGSCHECKABLECOMBOBOXPLUGIN_H


#include <QtGlobal>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtUiPlugin/QDesignerExportWidget>
#include "qgis_customwidgets.h"


class CUSTOMWIDGETS_EXPORT QgsCheckableComboBoxPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsCheckableComboBoxPlugin( QObject *parent = nullptr );

  private:
    bool mInitialized = false;

    // QDesignerCustomWidgetInterface interface
  public:
    QString name() const override;
    QString group() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;
    QWidget *createWidget( QWidget *parent ) override;
    bool isInitialized() const override;
    void initialize( QDesignerFormEditorInterface *core ) override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString domXml() const override;
};
#endif // QGSCHECKABLECOMBOBOXPLUGIN_H
