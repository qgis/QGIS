/***************************************************************************
                         qgsattributeeditor.h  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSATTRIBUTEEDITOR_H
#define QGSATTRIBUTEEDITOR_H

#include <QVariant>

class QObject;
class QWidget;
class QgsVectorLayer;
class QComboBox;

/* \brief create attribute widget for editing */
class GUI_EXPORT QgsAttributeEditor : public QObject
{
    Q_OBJECT

  public:
    QgsAttributeEditor( QObject *parent ) : QObject( parent ) {};
    static QWidget *createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value );
    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );

  private:
    static QComboBox *comboBox( QWidget *editor, QWidget *parent );

  public slots:
    void selectFileName( void );
    void selectDate( void );
};


#endif
