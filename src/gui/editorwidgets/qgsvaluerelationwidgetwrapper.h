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
#include <QLineEdit>

class QgsValueRelationWidgetFactory;

/**
 * Wraps a value relation widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * This is useful for having value lists on a separate layer containing codes and their
 * translation to human readable names.
 *
 * Options:
 *
 * <ul>
 * <li><b>Layer</b> <i>The id of the referenced layer.</i></li>
 * <li><b>Key</b> <i>The key field on the referenced layer (code).</i></li>
 * <li><b>Value</b> <i>The value field on the referenced layer (human readable name).</i></li>
 * <li><b>AllowMulti</b> <i>If set to True, will allow multiple selections. This requires the data type to be a string. This does NOT work with normalized database structures.</i></li>
 * <li><b>AllowNull</b> <i>Will offer NULL as a possible value.</i></li>
 * <li><b>FilterExpression</b> <i>If not empty, will be used as expression. Only if this evaluates to True, the value will be shown.</i></li>
 * <li><b>OrderByValue</b> <i>Will order by value instead of key.</i></li>
 * </ul>
 *
 */

class GUI_EXPORT QgsValueRelationWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT

  public:
    typedef QPair < QVariant, QString > ValueRelationItem;
    typedef QVector < ValueRelationItem > ValueRelationCache;

  public:
    explicit QgsValueRelationWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor = 0, QWidget* parent = 0 );
    static bool orderByKeyLessThan( const QgsValueRelationWidgetWrapper::ValueRelationItem& p1 ,
            const QgsValueRelationWidgetWrapper::ValueRelationItem& p2 );
    static bool orderByValueLessThan( const QgsValueRelationWidgetWrapper::ValueRelationItem& p1 ,
            const QgsValueRelationWidgetWrapper::ValueRelationItem& p2 );



    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() override;
    // TODO or have friend class :)
    static ValueRelationCache createCache( const QgsEditorWidgetConfig& config );

  protected:
    QWidget* createWidget( QWidget* parent ) override;
    void initWidget( QWidget* editor ) override;

  public slots:
    void setValue( const QVariant& value ) override;

  private:
    QComboBox* mComboBox;
    QListWidget* mListWidget;
    QLineEdit* mLineEdit;

    ValueRelationCache mCache;
    QgsVectorLayer* mLayer;

    friend class QgsValueRelationWidgetFactory;
};

Q_DECLARE_METATYPE( QgsValueRelationWidgetWrapper::ValueRelationCache )

#endif // QGSVALUERELATIONWIDGETWRAPPER_H
