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

class QgsAttributeForm : QWidget
{
%TypeHeaderCode
#include <qgsattributeform.h>
%End

  public:
    explicit QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature feature = QgsFeature(), QgsAttributeEditorContext context = QgsAttributeEditorContext(), QWidget *parent = 0 );
    ~QgsAttributeForm();

    const QgsFeature& feature();

    void hideButtonBox();

    void showButtonBox();

  signals:
    /**
     * Notifies about changes of attributes
     *
     * @param attribute The name of the attribute that changed.
     * @param value     The new value of the attribute.
     */
    void attributeChanged( QString attribute, const QVariant& value );

  public slots:
    void changeAttribute( const QString& field, const QVariant& value );
    void setFeature( const QgsFeature& feature );
    bool save();
    void accept();
    void resetValues();
};
