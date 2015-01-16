/***************************************************************************
    qgsattributeformlegacyinterface.h
     --------------------------------------
    Date                 : 13.5.2014
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

#ifndef QGSATTRIBUTEFORMLEGACYINTERFACE_H
#define QGSATTRIBUTEFORMLEGACYINTERFACE_H

#include <QString>

#include "qgsattributeforminterface.h"

/**
 * This class helps to support legacy open form scripts to be compatible with the new
 * QgsAttributeForm style interface.
 */
class GUI_EXPORT QgsAttributeFormLegacyInterface : public QgsAttributeFormInterface
{
  public:
    explicit QgsAttributeFormLegacyInterface( const QString& function, const QString& pyFormName, QgsAttributeForm* form );
    ~QgsAttributeFormLegacyInterface();

    // QgsAttributeFormInterface interface
    void featureChanged() override;

  private:
    QString mPyFunctionName;
    QString mPyFormVarName;
    QString mPyLayerVarName;
};

#endif // QGSATTRIBUTEFORMLEGACYINTERFACE_H
