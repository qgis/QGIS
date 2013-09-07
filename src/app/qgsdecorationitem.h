/***************************************************************************
                         qgsdecorationitem.h
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONITEM_H
#define QGSDECORATIONITEM_H

#include <QObject>
#include "qgslogger.h"

class QPainter;

class APP_EXPORT QgsDecorationItem: public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    QgsDecorationItem( QObject* parent = NULL );
    //! Destructor
    virtual ~ QgsDecorationItem();

    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool enabled() const { return mEnabled; }

    void update();

  signals:
    void toggled( bool t );

  public slots:
    //! set values on the gui when a project is read or the gui first loaded
    virtual void projectRead();
    //! save values to the project
    virtual void saveToProject();

    //! this does the meaty bit of the work
    virtual void render( QPainter * ) {}
    //! Show the dialog box
    virtual void run() {}

    virtual void setName( const char *name );
    virtual QString name() { return mName; }

  protected:

    /**True if decoration item has to be displayed*/
    bool mEnabled;

    QString mName;
    QString mNameConfig;
    QString mNameTranslated;
};

#endif
