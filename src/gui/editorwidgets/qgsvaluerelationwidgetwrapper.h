/***************************************************************************
    qgsvaluerelationwidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUERELATIONWIDGETWRAPPER_H
#define QGSVALUERELATIONWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"

#include <QComboBox>
#include <QListWidget>

class QgsValueRelationWidgetFactory;

class GUI_EXPORT QgsValueRelationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT

  public:
    typedef QPair < QVariant, QString > ValueRelationItem;
    typedef QVector < ValueRelationItem > ValueRelationCache;

  public:
    explicit QgsValueRelationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );


    // QgsEditorWidgetWrapper interface
  public:
    QVariant value();

  protected:
    QWidget* createWidget( QWidget* parent );
    void initWidget( QWidget* editor );
    static ValueRelationCache createCache( const QgsEditorWidgetConfig& config );

  public slots:
    void setValue( const QVariant& value );

  private:
    QComboBox* mComboBox;
    QListWidget* mListWidget;

    ValueRelationCache mCache;
    QgsVectorLayer* mLayer;

    friend class QgsValueRelationWidgetFactory;
};

Q_DECLARE_METATYPE( QgsValueRelationWidgetWrapper::ValueRelationCache )

#endif // QGSVALUERELATIONWIDGETWRAPPER_H
