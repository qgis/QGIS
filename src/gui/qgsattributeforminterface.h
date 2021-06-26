/***************************************************************************
    qgsattributeforminterface.h
     --------------------------------------
    Date                 : 12.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEFORMINTERFACE_H
#define QGSATTRIBUTEFORMINTERFACE_H

#include "qgis_gui.h"

class QgsAttributeForm;
class QgsFeature;

/**
 * \ingroup gui
 * \class QgsAttributeFormInterface
 */
class GUI_EXPORT QgsAttributeFormInterface
{
  public:
    explicit QgsAttributeFormInterface( QgsAttributeForm *form );

    virtual ~QgsAttributeFormInterface() = default;

    virtual bool acceptChanges( const QgsFeature &feature );

    virtual void initForm();

    virtual void featureChanged();

    QgsAttributeForm *form();

    const QgsFeature &feature();

  private:
    QgsAttributeForm *mForm = nullptr;
};

#endif // QGSATTRIBUTEFORMINTERFACE_H
