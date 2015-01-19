/***************************************************************************
   qgscustomwidgets.h
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

#ifndef QGISCUSTOMWIDGETS_H
#define QGISCUSTOMWIDGETS_H


#include <QDesignerCustomWidgetCollectionInterface>
#include <qplugin.h>


class QgisCustomWidgets : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA( IID "org.qgis.CustomWidgets" )
#endif
    Q_INTERFACES( QDesignerCustomWidgetCollectionInterface )

  public:
    explicit QgisCustomWidgets( QObject *parent = 0 );

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const override;

    static QString groupName() { return tr( "QGIS custom widgets" ); }

  private:
    QList<QDesignerCustomWidgetInterface*> mWidgets;
};

#endif // QGISCUSTOMWIDGETS_H
