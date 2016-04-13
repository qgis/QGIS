/***************************************************************************
    qgswfsdescribefeaturetype.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSDESCRIBEFEATURETYPE_H
#define QGSWFSDESCRIBEFEATURETYPE_H

#include "qgswfsrequest.h"

/** Manages the DescribeFeatureType request */
class QgsWFSDescribeFeatureType : public QgsWFSRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSDescribeFeatureType( const QString& theUri );

    /** Issue the request */
    bool requestFeatureType( const QString& WFSVersion, const QString& typeName );

  protected:
    virtual QString errorMessageWithReason( const QString& reason ) override;
};

#endif // QGSWFSDESCRIBEFEATURETYPE_H
