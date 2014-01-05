/***************************************************************************
    qgsattributeforminterface.h
     --------------------------------------
    Date                 : 12.5.2014
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

#ifndef QGSATTRIBUTEFORMINTERFACE_H
#define QGSATTRIBUTEFORMINTERFACE_H

class QgsAttributeForm;
class QgsFeature;

class GUI_EXPORT QgsAttributeFormInterface
{
  public:
    explicit QgsAttributeFormInterface( QgsAttributeForm* form );

    virtual bool acceptChanges( const QgsFeature& feature );

    virtual void initForm();

    virtual void featureChanged();

    QgsAttributeForm* form();

    const QgsFeature& feature();

  private:
    QgsAttributeForm* mForm;
};

#endif // QGSATTRIBUTEFORMINTERFACE_H
