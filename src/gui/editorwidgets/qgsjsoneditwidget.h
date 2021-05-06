/***************************************************************************
    qgsjsoneditwidget.h
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSJSONEDITWIDGET_H
#define QGSJSONEDITWIDGET_H

#include "ui_qgsjsoneditwidget.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsJsonEditWidget
 */

class GUI_EXPORT QgsJsonEditWidget : public QWidget, private Ui::QgsJsonEditWidget
{
    Q_OBJECT

  public:

    enum class View : int
    {
      Text = 0,
      Tree = 1
    };

    explicit QgsJsonEditWidget( QWidget *parent SIP_TRANSFERTHIS );

    ~QgsJsonEditWidget() override;


    void setJsonText( const QString &jsonText );

    QString jsonText() const;

    bool validJson() const;

    void setView( View view ) const;

  private slots:

    void textToolButtonClicked( bool checked );
    void treeToolButtonClicked( bool checked );

    void refreshTreeView();

  private:

    enum class TreeWidgetColumn : int
    {
      Key = 0,
      Value = 1
    };

    void refreshTreeViewItemValue( const QJsonValue &jsonValue, QTreeWidgetItem *treeWidgetItemParent );
};

#endif // QGSJSONEDITWIDGET_H
