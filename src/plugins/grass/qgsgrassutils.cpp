/*******************************************************************
                              qgsgrassutils.cpp
                             -------------------
    begin                : March, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify  
 it under the terms of the GNU General Public License as published by 
 the Free Software Foundation; either version 2 of the License, or     
 (at your option) any later version.                                   
*******************************************************************/
#include <iostream>
#include <vector>

#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>

//#include "qgis.h"
//#include "qgsapplication.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}

#include "../../src/providers/grass/qgsgrass.h"
#include "qgsgrassutils.h"
#include "qgsgrassselect.h"

QgsGrassUtils::QgsGrassUtils() {}
QgsGrassUtils::~QgsGrassUtils() {}

QString QgsGrassUtils::vectorLayerName( QString map, QString layer, 
                                        int nLayers )
{
    QString name = map;
    if ( nLayers > 1 ) name += " " + layer;
    return name;
}

void QgsGrassUtils::addVectorLayers ( QgisIface *iface,
        QString gisbase, QString location, QString mapset, QString map)
{
    QStringList layers = QgsGrassSelect::vectorLayers(
			   gisbase, location, mapset, map );


    for ( int i = 0; i < layers.count(); i++ )
    {
        QString name = QgsGrassUtils::vectorLayerName (
	  		      map, layers[i], layers.size() );

	QString uri = gisbase + "/" + location + "/"
		   + mapset + "/" + map + "/" + layers[i];

#ifdef QGISDEBUG
         std::cerr << "layer = " << layers[i].local8Bit().data() << std::endl;
         std::cerr << "uri = " << uri.local8Bit().data() << std::endl;
         std::cerr << "name = " << name.local8Bit().data() << std::endl;
#endif

	iface->addVectorLayer( uri, name, "grass");
    }
}

bool QgsGrassUtils::itemExists ( QString element, QString item )
{
    QString path = QgsGrass::getDefaultGisdbase() + "/"
                  + QgsGrass::getDefaultLocation() + "/"
                  + QgsGrass::getDefaultMapset() + "/"
                  + "/" + element + "/" + item;

    QFileInfo fi(path);
    return fi.exists(); 
}

QgsGrassElementDialog::QgsGrassElementDialog() : QObject() 
{
}
QgsGrassElementDialog::~QgsGrassElementDialog() {}

QString QgsGrassElementDialog::getItem ( QString element,
                       QString text, bool * ok )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassElementDialog::getItem" << std::endl;
#endif
    *ok = false;
    mElement = element;
    mDialog = new QDialog ();
    QVBoxLayout *layout = new QVBoxLayout ( mDialog );
    QHBoxLayout *buttonLayout = new QHBoxLayout ( );

    mLineEdit = new QLineEdit ( text );
    layout->addWidget( mLineEdit );
    mErrorLabel = new QLabel ( );
    layout->addWidget( mErrorLabel );

    mOkButton = new QPushButton ( "Ok" );
    mCancelButton = new QPushButton ( "Cancel" );
     
    layout->insertLayout( -1, buttonLayout );
    buttonLayout->addWidget( mOkButton );
    buttonLayout->addWidget( mCancelButton );

    connect ( mLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged() ) );
    connect ( mOkButton, SIGNAL(clicked()), mDialog, SLOT(accept() ) );
    connect ( mCancelButton, SIGNAL(clicked()), mDialog, SLOT(reject() ) );

    textChanged ();
    if ( mDialog->exec() == QDialog::Accepted )
    {
        *ok = true;
    }

    QString name = mLineEdit->text();
    delete mDialog;

    return name;
}

void QgsGrassElementDialog::textChanged ()
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrassElementDialog::textChanged" << std::endl;
#endif

   mErrorLabel->setText ( "" );
   if ( QgsGrassUtils::itemExists( mElement, mLineEdit->text() ) )
   {
       mErrorLabel->setText ( "Exists!" );
   }
}
