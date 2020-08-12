/***************************************************************************
   qgsfeaturelistcomboboxplugin.h
    --------------------------------------
   Date                 : 26.03.2020
   Copyright            : (C) 2020 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFEATURELISTCOMBOBOXPLUGIN_H
#define QGSFEATURELISTCOMBOBOXPLUGIN_H


#include <QtGlobal>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtUiPlugin/QDesignerExportWidget>
#include "qgis_customwidgets.h"


class CUSTOMWIDGETS_EXPORT QgsFeatureListComboBoxPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsFeatureListComboBoxPlugin( QObject *parent = nullptr );

  private:
    bool mInitialized;

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
#endif // QGSFEATURELISTCOMBOBOXPLUGIN_H
