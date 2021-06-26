/***************************************************************************
    qgsattributeformlegacyinterface.h
     --------------------------------------
    Date                 : 13.5.2014
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

#ifndef QGSATTRIBUTEFORMLEGACYINTERFACE_H
#define QGSATTRIBUTEFORMLEGACYINTERFACE_H

#include <QString>

#include "qgsattributeforminterface.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * This class helps to support legacy open form scripts to be compatible with the new
 * QgsAttributeForm style interface.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsAttributeFormLegacyInterface : public QgsAttributeFormInterface
{
  public:
    explicit QgsAttributeFormLegacyInterface( const QString &function, const QString &pyFormName, QgsAttributeForm *form );
    ~QgsAttributeFormLegacyInterface() override;

    // QgsAttributeFormInterface interface
    void featureChanged() override;

  private:
    QString mPyFunctionName;
    QString mPyFormVarName;
    QString mPyLayerVarName;
};

#endif // QGSATTRIBUTEFORMLEGACYINTERFACE_H
