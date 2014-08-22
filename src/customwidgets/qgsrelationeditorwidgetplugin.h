/***************************************************************************
   qgsrelationeditorwidgetplugin.h
    --------------------------------------
   Date                 : 20.08.2014
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

#ifndef QGSRELATIONEDITORWIDGETPLUGIN_H
#define QGSRELATIONEDITORWIDGETPLUGIN_H

#include <QDesignerExportWidget>
#include <QDesignerCustomWidgetInterface>


class CUSTOMWIDGETS_EXPORT QgsRelationEditorWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsRelationEditorWidgetPlugin( QObject *parent = 0 );

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
#endif // QGSRELATIONEDITORWIDGETPLUGIN_H
