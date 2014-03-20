/***************************************************************************
    qgsattributeform.h
     --------------------------------------
    Date                 : 3.5.2014
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

#ifndef QGSATTRIBUTEFORM_H
#define QGSATTRIBUTEFORM_H

#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsattributeeditorcontext.h"

#include <QWidget>
#include <QDialogButtonBox>

class QgsAttributeFormInterface;

class GUI_EXPORT QgsAttributeForm : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature feature = QgsFeature(), QgsAttributeEditorContext context = QgsAttributeEditorContext(), QWidget *parent = 0 );
    ~QgsAttributeForm();

    const QgsFeature& feature() { return mFeature; }

    void hideButtonBox();

    void showButtonBox();

    /**
     * Takes ownership
     * @param iface
     */
    void addInterface( QgsAttributeFormInterface* iface );

    QgsVectorLayer* layer() { return mLayer; }

    bool editable();

  signals:
    /**
     * Notifies about changes of attributes
     *
     * @param attribute The name of the attribute that changed.
     * @param value     The new value of the attribute.
     */
    void attributeChanged( QString attribute, const QVariant& value );

    /**
     * Will be emitted before the feature is saved. Use this signal to perform sanity checks.
     * You can set the parameter ok to false to notify the form that you don't want it to be saved.
     * If you want the form to be saved, leave the parameter untouched.
     *
     * @param ok  Set this parameter to false if you don't want the form to be saved
     */
    void beforeSave( bool& ok );

  public slots:
    void changeAttribute( const QString& field, const QVariant& value );
    void setFeature( const QgsFeature& feature );
    bool save();
    /**
     * Alias for save()
     *
     * @deprecated
     */
    Q_DECL_DEPRECATED void accept() { save(); }

    /**
     * Alias for resetValues()
     *
     * @deprecated
     */
    Q_DECL_DEPRECATED void reject() { resetValues(); }

    void resetValues();

  private slots:
    void onAttributeChanged( const QVariant& value );
    void onAttributeAdded( int idx );
    void onAttributeDeleted( int idx );

    void synchronizeEnabledState();

  private:
    void init();

    void cleanPython();

    void initPython();

    QWidget* createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributeEditorContext& context, QString& labelText, bool& labelOnTop );

    /**
     * Creates widget wrappers for all suitable widgets found.
     * Called once maximally.
     */
    void createWrappers();
    void connectWrappers();

    QgsVectorLayer* mLayer;
    QgsFeature mFeature;
    QList<QgsWidgetWrapper*> mWidgets;
    QgsAttributeEditorContext mContext;
    QDialogButtonBox* mButtonBox;
    QList<QgsAttributeFormInterface*> mInterfaces;

    // Variables below are used for python
    static int sFormCounter;
    int mFormNr;
    QString mPyFormVarName;

    //! Set to true while saving to prevent recursive saves
    bool mIsSaving;
};

#endif // QGSATTRIBUTEFORM_H
