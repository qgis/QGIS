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
#include "qgscollapsiblegroupbox.h"
#include "qgsfeature.h"

#include <QComboBox>
#include <QVBoxLayout>

class QgsAttributeDialog;
class QgsVectorLayerTools;

class GUI_EXPORT QgsRelationReferenceWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsRelationReferenceWidget( QWidget* parent );

    void displayEmbedForm( bool display );

    void setRelation( QgsRelation relation , bool allowNullValue );

    void setRelationEditable( bool editable );

    void setRelatedFeature( const QVariant &value );

    QVariant relatedFeature();

    void setEditorContext( QgsAttributeEditorContext context );

  signals:
    void relatedFeatureChanged( QVariant );

  private slots:
    void buttonTriggered( QAction* action );
    void referenceChanged( int index );
    void openForm();

  private:
    QgsVectorLayer* mReferencedLayer;
    bool mInitialValueAssigned;
    QgsAttributeDialog* mAttributeDialog;
    QGridLayout* mLayout;
    QHash<QgsFeatureId, QVariant> mFidFkMap; // Mapping from feature id => foreign key
    QAction* mShowFormAction;
    QComboBox* mComboBox;
    QgsCollapsibleGroupBox* mAttributeEditorFrame;
    QVBoxLayout* mAttributeEditorLayout;
    QgsAttributeEditorContext mEditorContext;
};




#endif // QGSRELATIONREFERENCEWIDGET_H
