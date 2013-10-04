/***************************************************************************
    qgsrelationreferencewidget.h
     --------------------------------------
    Date                 : 20.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONREFERENCEWIDGET_H
#define QGSRELATIONREFERENCEWIDGET_H

#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsfeature.h"

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

class QgsAttributeDialog;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationReferenceWidget : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsRelationReferenceWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QgsAttributeEditorContext context, QWidget* parent = 0 );
    virtual QWidget* createWidget( QWidget* parent );
    virtual void initWidget( QWidget* editor );
    virtual QVariant value();

  signals:
    void valueChanged( const QVariant& value );

  public slots:
    virtual void setValue( const QVariant& value );
    virtual void setEnabled( bool enabled );

  private slots:
    void referenceChanged( int index );
    void openForm();

  private:
    bool mInitialValueAssigned;
    QComboBox* mComboBox;
    QWidget* mAttributeEditorFrame;
    QVBoxLayout* mAttributeEditorLayout;
    QPushButton* mAttributeEditorButton;
    QgsVectorLayer* mReferencedLayer;
    QVariant mCurrentValue;
    QgsAttributeDialog* mAttributeDialog;
    QHash<QgsFeatureId, QVariant> mFidFkMap; // Mapping from feature id => foreign key
    QgsAttributeEditorContext mEditorContext;
};

#endif // QGSRELATIONREFERENCEWIDGET_H
