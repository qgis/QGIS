/***************************************************************************
   %CLASSLOWERCASE%plugin.h
    --------------------------------------
   Date                 : %DATE%
   Copyright            : (C) %YEAR% %AUTHOR%
   Email                : %EMAIL%
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef %CLASSUPPERCASE%PLUGIN_H
#define %CLASSUPPERCASE%PLUGIN_H


#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QDesignerCustomWidgetCollectionInterface>
#include <QDesignerExportWidget>
#else
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtUiPlugin/QDesignerExportWidget>
#endif


class CUSTOMWIDGETS_EXPORT %CLASSMIXEDCASE%Plugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit %CLASSMIXEDCASE%Plugin( QObject *parent = 0 );

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
#endif // %CLASSUPPERCASE%PLUGIN_H
