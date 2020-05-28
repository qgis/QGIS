/***************************************************************************
   qgsauthconfigselectplugin.h
    --------------------------------------
   Date                 : October 2019
   Copyright            : (C) 2019 Denis Rouzaud
   Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSAUTHCONFIGSELECTPLUGIN_H
#define QGSAUTHCONFIGSELECTPLUGIN_H


#include <QtGlobal>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtUiPlugin/QDesignerExportWidget>
#include "qgis_customwidgets.h"


class CUSTOMWIDGETS_EXPORT QgsAuthConfigSelectPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsAuthConfigSelectPlugin( QObject *parent = nullptr );

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
#endif // QGSAUTHCONFIGSELECTPLUGIN_H
