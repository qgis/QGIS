/***************************************************************************
   qgscollapsiblegroupboxplugin.h
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSCOLLAPSIBLEGROUPBOXPLUGIN_H
#define QGSCOLLAPSIBLEGROUPBOXPLUGIN_H

#include <QDesignerExportWidget>
#include <QDesignerCustomWidgetInterface>


class CUSTOMWIDGETS_EXPORT QgsCollapsibleGroupBoxPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsCollapsibleGroupBoxPlugin( QObject *parent = 0 );

  private:
    bool mInitialized;

    // QDesignerCustomWidgetInterface interface
  public:
    QString name() const;
    QString group() const;
    QString includeFile() const;
    QIcon icon() const;
    bool isContainer() const;
    QWidget *createWidget( QWidget *parent );
    bool isInitialized() const;
    void initialize( QDesignerFormEditorInterface *core );
    QString toolTip() const;
    QString whatsThis() const;
    QString domXml() const;
};
#endif // QGSCOLLAPSIBLEGROUPBOXPLUGIN_H
