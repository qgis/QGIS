/***************************************************************************
                          qgsgenericprojectionselector.h
                Set user defined projection using projection selector widget 
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
#ifndef QGSGENERICPROJECTIONSELECTOR_H
#define QGSGENERICPROJECTIONSELECTOR_H
#include "ui_qgsgenericprojectionselectorbase.h"
#include "qgisgui.h"

#include <QSet>

/**
 * \class QgsGenericProjectionSelector 
 * \brief A generic dialog to prompt the user for a Coordinate Reference System
 */

class GUI_EXPORT QgsGenericProjectionSelector : public QDialog, private Ui::QgsGenericProjectionSelectorBase
{
  Q_OBJECT;
  public:
    /**
     * Constructor
     */
    QgsGenericProjectionSelector(QWidget *parent = 0, 
                               Qt::WFlags fl = QgisGui::ModalDialogFlags);

    //! Destructor
    ~QgsGenericProjectionSelector();

 public slots:
      /** If no paramter is passed, the message will be a generic
       * 'define the CRS for this layer'.
       */
      void setMessage(QString theMessage="");
      QString getSelectedProj4String();
      long getSelectedSRSID();
      long getSelectedEpsg();

      void setSelectedSRSName(QString theName);
      void setSelectedSRSID(long theID);
      void setSelectedEpsg(long theID);

      /**
       * \brief filters this dialog by the given CRSs
       *
       * Sets this dialog to filter the available projections to those listed
       * by the given Coordinate Reference Systems.
       *
       * \param crsFilter a list of OGC Coordinate Reference Systems to filter the 
       *                  list of projections by.  This is useful in (e.g.) WMS situations
       *                  where you just want to offer what the WMS server can support.
       *
       * \note This function only deals with EPSG labels only at this time.
       *
       * \warning This function's behaviour is undefined if it is called after the dialog is shown.
       */
      void setOgcWmsCrsFilter(QSet<QString> crsFilter);


};

#endif // #ifndef QGSLAYERPROJECTIONSELECTOR_H
