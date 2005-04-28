/***************************************************************************
                          qgslayerprojectionselector.h
                        Set user layerprojectionselector and preferences
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSLAYERPROJECTIONSELECTOR_H
#define QGSLAYERPROJECTIONSELECTOR_H
class QString;
#ifdef WIN32
#include "qgslayerprojectionselectorbase.h"
#else
#include "qgslayerprojectionselectorbase.uic.h"
#endif
/**
 * \class QgsLayerProjectionSelector
 * \brief Set Projection system for a layer
 */
class QgsLayerProjectionSelector :public QgsLayerProjectionSelectorBase{
  Q_OBJECT;
  public:
    /**
     * Constructor
     */
    QgsLayerProjectionSelector();
    //! Destructor
    ~QgsLayerProjectionSelector();

 public slots:
      void pbnOK_clicked();
      QString getCurrentWKT();
      long getCurrentSRID();
      void setSelectedWKT(QString theWKTName);
};

#endif // #ifndef QGSLAYERPROJECTIONSELECTOR_H
