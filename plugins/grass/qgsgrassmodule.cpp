/***************************************************************************
                              qgsgrasstools.cpp
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h> 
#include <qsettings.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qpointarray.h>
#include <qcursor.h>
#include <qnamespace.h>
#include <qlistview.h>
#include <qcolordialog.h>
#include <qtable.h>
#include <qstatusbar.h>
#include <qevent.h>
#include <qpoint.h>
#include <qsize.h>
#include <qdom.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qprocess.h>
#include <qcstring.h>
#include <qlineedit.h>
#include <qtextbrowser.h>
#include <qdir.h>
#include <qregexp.h>
#include <qprogressbar.h>
#include <qstylesheet.h>

#include "../../src/qgis.h"
#include "../../src/qgsmapcanvas.h"
#include "../../src/qgsmaplayer.h"
#include "../../src/qgsvectorlayer.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsfeatureattribute.h"

extern "C" {
#include <gis.h>
#include <Vect.h>
}

#include "../../providers/grass/qgsgrass.h"
#include "../../providers/grass/qgsgrassprovider.h"
#include "qgsgrassattributes.h"
#include "qgsgrassmodule.h"
#include "qgsgrasstools.h"

QgsGrassModule::QgsGrassModule ( QgsGrassTools *tools, QgisApp *qgisApp, QgisIface *iface, 
	                     QString path, QWidget * parent, const char * name, WFlags f )
             :QgsGrassModuleBase ( parent, name, f )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule()" << std::endl;
    #endif

    mPath = path;
    mTools = tools;
    mQgisApp = qgisApp;
    mIface = iface;
    mCanvas = mIface->getMapCanvas();
    mParent = parent;
    mAppDir = mTools->appDir();

    /* Read module description and create options */

    // Open QGIS module description
    QFile qFile ( mPath );
    if ( !qFile.exists() ) {
	QMessageBox::warning( 0, "Warning", "The module file (" + mPath + ") not found." );
	return;
    }
    if ( ! qFile.open( IO_ReadOnly ) ) {
	QMessageBox::warning( 0, "Warning", "Cannot open module file (" + mPath + ")" );
	return;
    }
    QDomDocument qDoc ( "qgisgrassmodule" );
    QString err;
    int line, column;
    if ( !qDoc.setContent( &qFile,  &err, &line, &column ) ) {
	QString errmsg = "Cannot read module file (" + mPath + "):\n" + err + "\nat line "
	                 + QString::number(line) + " column " + QString::number(column);
	std::cerr << errmsg << std::endl;
	QMessageBox::warning( 0, "Warning", errmsg );
	qFile.close();
	return;
    }
    qFile.close();
    QDomElement qDocElem = qDoc.documentElement();

    // Read GRASS module description
    QString gisBase = getenv("GISBASE"); // TODO read from QgsGrassPlugin
    mXName = qDocElem.attribute("module");
    mXPath = gisBase + "/bin/" + mXName;
    QProcess *process = new QProcess( this );
    process->addArgument( mXPath );
    process->addArgument( "--interface-description" );
    if ( !process->start() ) {
	QMessageBox::warning( 0, "Warning", "Cannot start module " + mXPath );
	return;
    }
    while ( process->isRunning () ) { // TODO: check time, if it is not running too long
    }
    QByteArray gDescArray = process->readStdout();
    delete process;

    QDomDocument gDoc ( "task" );
    if ( !gDoc.setContent( (QByteArray)gDescArray, &err, &line, &column ) ) {
	QString errmsg = "Cannot read module description (" + mXPath + "):\n" + err + "\nat line "
	                 + QString::number(line) + " column " + QString::number(column);
	std::cerr << errmsg << std::endl;
	QMessageBox::warning( 0, "Warning", errmsg );
	return;
    }
    QDomElement gDocElem = gDoc.documentElement();

    
    // Read QGIS options and create controls
    QDomNode n = qDocElem.firstChild();
    QVBoxLayout *layout = new QVBoxLayout ( mTabWidget->page(0), 10 );
    while( !n.isNull() ) {
	QDomElement e = n.toElement();
	if( !e.isNull() ) {
	    QString optionType = e.tagName();
	    //std::cout << "optionType = " << optionType << std::endl;

	    if ( optionType == "option" ) {
	        bool created = false;

		// Check option type and create appropriate control
		QString key = e.attribute("key");
		std::cout << "key = " << key << std::endl;

		QDomNode gnode = nodeByKey ( gDocElem, key );
		if ( gnode.isNull() ) {
		    QMessageBox::warning( 0, "Warning", "Cannot find key " +  key );
		    return;
		}

		QDomNode promptNode = gnode.namedItem ( "gisprompt" );
		if ( !promptNode.isNull() ) {
		    QDomElement promptElem = promptNode.toElement();
		    QString element = promptElem.attribute("element"); 
		    QString age = promptElem.attribute("age"); 
		    //std::cout << "element = " << element << " age = " << age << std::endl;
		    if ( age == "old" && ( element == "vector" || element == "cell") ) {
			QgsGrassModuleInput *mi = 
				 new QgsGrassModuleInput ( this, e, gDocElem, mTabWidget->page(0) );

			layout->addWidget ( mi );
			created = true;
			mItems.push_back(mi);
		    }
		} 

		if ( !created ) {
		    QgsGrassModuleOption *so = 
			new QgsGrassModuleOption ( this, e, gDocElem, mTabWidget->page(0) );
		    
			layout->addWidget ( so );
			created = true;
		    mItems.push_back(so);
		}
	    } else if ( optionType == "flag" )  {
		    QgsGrassModuleFlag *flag = 
			new QgsGrassModuleFlag ( this, e, gDocElem, mTabWidget->page(0) );
		    
		    layout->addWidget ( flag );
		    mItems.push_back(flag);
	    }
	}
	n = n.nextSibling();
    }

    QSpacerItem *si = new QSpacerItem ( 10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addItem ( si );

    connect ( &mProcess, SIGNAL(readyReadStdout()), this, SLOT(readStdout()));
    connect ( &mProcess, SIGNAL(readyReadStderr()), this, SLOT(readStderr()));
    connect ( &mProcess, SIGNAL(launchFinished()), this, SLOT(finished()));
    connect ( &mProcess, SIGNAL(processExited()), this, SLOT(finished()));

    char *env = "GRASS_MESSAGE_FORMAT=gui";
    char *envstr = new char[strlen(env)+1];
    strcpy ( envstr, env );
    putenv( envstr );

    mOutputTextBrowser->setTextFormat(RichText);
    mOutputTextBrowser->setReadOnly(TRUE);
}

QString QgsGrassModule::label ( QString path ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::label()" << std::endl;
    #endif

    // Open QGIS module description
    QFile qFile ( path );
    if ( !qFile.exists() ) {
	return QString ( "Not available, decription not found (" + path + ")" );
    }
    if ( ! qFile.open( IO_ReadOnly ) ) {
	return QString ( "Not available, cannot open description (" + path + ")" ) ;
    }
    QDomDocument qDoc ( "qgisgrassmodule" );
    QString err;
    int line, column;
    if ( !qDoc.setContent( &qFile,  &err, &line, &column ) ) {
	QString errmsg = "Cannot read module file (" + path + "):\n" + err + "\nat line "
	                 + QString::number(line) + " column " + QString::number(column);
	std::cerr << errmsg << std::endl;
	QMessageBox::warning( 0, "Warning", errmsg );
	qFile.close();
	return QString ( "Not available, incorrect description (" + path + ")" );
    }
    qFile.close();
    QDomElement qDocElem = qDoc.documentElement();

    return ( qDocElem.attribute("label") );
}


void QgsGrassModule::run()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::run()" << std::endl;
    #endif

    if ( mProcess.isRunning() ) {
	mProcess.kill();
	mRunButton->setText( tr("Run") );
    } else { 
	QString command;
	
	if ( mProcess.isRunning() ) {
	}
	mProcess.clearArguments();
	mProcess.addArgument( mXPath );
	command = mXName;

	for ( int i = 0; i < mItems.size(); i++ ) {
	    QStringList list = mItems[i]->options();

	    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		std::cerr << "option: " << *it << std::endl;
		command.append ( " " + *it );
		mProcess.addArgument( *it );
	    }
	}
	mProcess.start( );
	
	std::cerr << "command" << command << std::endl;
	mOutputTextBrowser->clear();

	mOutputTextBrowser->append( "<B>" +  command + "</B>" );
	mTabWidget->setCurrentPage(1);
	mRunButton->setText( tr("Stop") );
    }
}

void QgsGrassModule::finished()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::finished()" << std::endl;
    #endif

    if ( mProcess.normalExit() ) {
	if ( mProcess.exitStatus () == 0 ) {
	    mOutputTextBrowser->append( "<B>Successfully finished</B>" );
	    mProgressBar->setProgress ( 100, 100 ); 
	} else {
	    mOutputTextBrowser->append( "<B>Finished with error</B>" );
	}
    } else {
	mOutputTextBrowser->append( "<B>Module crashed or killed</B>" );
    }
    mRunButton->setText( tr("Run") );
}

void QgsGrassModule::readStdout()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::readStdout()" << std::endl;
    #endif

    QString line;
    while ( mProcess.canReadLineStdout() ) {
       	line = mProcess.readLineStdout ();
        //std::cerr << "stdout: " << line << std::endl;
	mOutputTextBrowser->append ( line );
    }
}

void QgsGrassModule::readStderr()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::readStderr()" << std::endl;
    #endif

    QString line;
    QRegExp rxpercent ( "GRASS_INFO_PERCENT: (\\d+)" );
    QRegExp rxwarning ( "GRASS_INFO_WARNING\\(\\d+,\\d+\\): (.*)" );
    QRegExp rxerror ( "GRASS_INFO_ERROR\\(\\d+,\\d+\\): (.*)" );
    QRegExp rxend ( "GRASS_INFO_END\\(\\d+,\\d+\\)" );

    while ( mProcess.canReadLineStderr() ) {
       	line = mProcess.readLineStderr ();
        //std::cerr << "stderr: " << line << std::endl;

	if ( rxpercent.search ( line ) != -1 ) {
	    int progress = rxpercent.cap(1).toInt();
	    mProgressBar->setProgress ( progress, 100 );
	} else if ( rxwarning.search ( line ) != -1 ) {
	    QString warn = rxwarning.cap(1);
	    QString img = mAppDir + "/share/qgis/themes/default/grass/grass_module_warning.png";
	    mOutputTextBrowser->append ( "<img src=\"" + img + "\">" + warn );
	} else if ( rxerror.search ( line ) != -1 ) {
	    QString error = rxerror.cap(1);
	    QString img = mAppDir + "/share/qgis/themes/default/grass/grass_module_error.png";
	    mOutputTextBrowser->append ( "<img src=\"" + img + "\">" + error );
	} else if ( rxend.search ( line ) != -1 ) {
	    // Do nothing
	} else {
	    mOutputTextBrowser->append ( line + "\n" );
	}
    }
}

void QgsGrassModule::close()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::close()" << std::endl;
    #endif

    QTabWidget *tw = dynamic_cast<QTabWidget *>(mParent);
    tw->removePage (this );
    delete this;
}

QgisIface *QgsGrassModule::qgisIface() { return mIface; }
QgisApp *QgsGrassModule::qgisApp() { return mQgisApp; }

QgsGrassModule::~QgsGrassModule()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::~QgsGrassModule()" << std::endl;
    #endif
}

QDomNode QgsGrassModule::nodeByKey ( QDomElement elem, QString key )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::nodeByKey() key = " << key << std::endl;
    #endif
    QDomNode n = elem.firstChild();

    while( !n.isNull() ) {
	QDomElement e = n.toElement();

	if( !e.isNull() ) {
	    if ( e.tagName() == "parameter" || e.tagName() == "flag" ) {
		if ( e.attribute("name") == key ) {
		    return n;
		}
	    }
	}
	n = n.nextSibling();
     }

     return QDomNode();
}

QgsGrassModuleOption::QgsGrassModuleOption ( QgsGrassModule *module, 
	                                   QDomElement &qdesc, QDomElement &gdesc,
	                                   QWidget * parent)
                    :QFrame ( parent )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModuleOption::QgsGrassModuleOption" << std::endl;
    #endif
    mModule = module;
    setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Minimum );

    mKey = qdesc.attribute("key");
    QDomNode gnode = QgsGrassModule::nodeByKey ( gdesc, mKey );
    if ( gnode.isNull() ) {
	QMessageBox::warning( 0, "Warning", "Cannot find key " +  mKey );
	return;
    }
    
    mAnswer = qdesc.attribute("answer", "");
    
    if ( qdesc.attribute("hidden") == "yes" ) {
	mHidden = true;
	hide();
    }

    // String without options 
    if ( !mHidden ) {
	QHBoxLayout *layout = new QHBoxLayout ( this, 10 );
	QLabel *lab = new QLabel ( this );

	QDomNode n = gnode.namedItem ( "description" );
	if ( !n.isNull() ) {
            QDomElement e = n.toElement();
	    QString description = e.text().stripWhiteSpace();
	    description.replace( 0, 1, description.left(1).upper() );
	    lab->setText ( description );
	} else {
	    lab->setText ( mKey ); 
	}
	layout->addWidget ( lab );

	mLineEdit = new QLineEdit ( this );
	n = gnode.namedItem ( "default" );
	if ( !n.isNull() ) {
            QDomElement e = n.toElement();
	    QString def = e.text().stripWhiteSpace();
	    mLineEdit->setText ( def );
	}
	layout->addWidget ( mLineEdit );
    }
}

QStringList QgsGrassModuleOption::options()
{
    QStringList list;
    
    if ( mHidden ) {
        list.push_back( mKey + "=" + mAnswer );
    } else {
        list.push_back( mKey + "=" + mLineEdit->text() );
    }
	
    return list;
}

QgsGrassModuleOption::~QgsGrassModuleOption()
{
}

QgsGrassModuleFlag::QgsGrassModuleFlag ( QgsGrassModule *module, 
	                                   QDomElement &qdesc, QDomElement &gdesc,
	                                   QWidget * parent)
                    :QCheckBox ( parent )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModuleFlag::QgsGrassModuleFlag" << std::endl;
    #endif
    mModule = module;
    mAnswer = false;

    mKey = qdesc.attribute("key");
    QDomNode gnode = QgsGrassModule::nodeByKey ( gdesc, mKey );
    if ( gnode.isNull() ) {
	QMessageBox::warning( 0, "Warning", "Cannot find key " +  mKey );
	return;
    }
    
    if ( qdesc.attribute("answer") == "on" ) {
	mAnswer = true;
    }
    
    if ( qdesc.attribute("hidden") == "yes" ) {
	mHidden = true;
	hide();
    }
    
    QDomNode n = gnode.namedItem ( "description" );
    if ( !n.isNull() ) {
        QDomElement e = n.toElement();
	mDescription = e.text().stripWhiteSpace();
    } else {
	// Should not happen
	std::cerr << "description not found or it is not text" << std::endl;
    }

    setChecked ( mAnswer );
    setText ( mDescription );
}

QStringList QgsGrassModuleFlag::options()
{
    QStringList list;
    if (  isChecked() ) {
        list.push_back( "-" + mKey );
    }
    return list;
}

QgsGrassModuleFlag::~QgsGrassModuleFlag()
{
}

QgsGrassModuleInput::QgsGrassModuleInput ( QgsGrassModule *module, 
	                                   QDomElement &qdesc, QDomElement &gdesc,
	                                   QWidget * parent)
                    :QGroupBox ( parent )
{
    mModule = module;
    setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Minimum );

	    
    mKey = qdesc.attribute("key");
    QDomNode gnode = QgsGrassModule::nodeByKey ( gdesc, mKey );
    if ( gnode.isNull() ) {
	QMessageBox::warning( 0, "Warning", "Cannot find key " +  mKey );
	return;
    }

    setTitle ( "Input" );
    QDomNode n = gnode.namedItem ( "description" );
    if ( !n.isNull() ) {
        QDomElement e = n.toElement();
	QString description = e.text().stripWhiteSpace();
	description.replace( 0, 1, description.left(1).upper() );
        setTitle ( " " + description + " " );
    }

    QDomNode promptNode = gnode.namedItem ( "gisprompt" );
    if ( promptNode.isNull() ) {
	QMessageBox::warning( 0, "Warning", "Cannot find gisprompt" );
	return;
    }
    QDomElement promptElem = promptNode.toElement();
    QString element = promptElem.attribute("element"); 

    if ( element == "vector" ) {
       mType = Vector;	
    } else if ( element == "cell" ) {
       mType = Raster;
    } else {
	QMessageBox::warning( 0, "Warning", "GRASS element " + element + " not supported" );
    }

    QVBoxLayout *layout = new QVBoxLayout ( this, 25 );
    mLayerComboBox = new QComboBox ( this );
    layout->addWidget ( mLayerComboBox );

    // Of course, activated(int) is not enough, but there is no signal BEFORE the cobo is opened
    connect ( mLayerComboBox, SIGNAL( activated(int) ), this, SLOT(updateQgisLayers()) );
    
    // Fill in QGIS layers 
    updateQgisLayers();
}

void QgsGrassModuleInput::updateQgisLayers()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModuleInput::updateQgisLayers" << std::endl;
    #endif
    QString current = mLayerComboBox->currentText ();
    mLayerComboBox->clear();
    mMaps.resize(0);

    QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

    int nlayers = canvas->layerCount();
    for ( int i = 0; i < nlayers; i++ ) {
	QgsMapLayer *layer = canvas->getZpos(i);

	if (  mType == Vector && layer->type() == QgsMapLayer::VECTOR ) {
	    QgsVectorLayer *vector = (QgsVectorLayer*)layer;
	    if ( vector->providerType() != "grass" ) continue;

	    //TODO dynamic_cast ?
	    QgsGrassProvider *provider = (QgsGrassProvider *) vector->getDataProvider();

	    // TODO add map() mapset() location() gisbase() to grass provider
	    QString source = QDir::cleanDirPath ( provider->getDataSourceUri() );

	    char sep = QDir::separator();
	    
	    // Check GISBASE and LOCATION
	    QStringList split = QStringList::split ( sep, source );
            
	    if ( split.size() < 4 ) continue;
	    split.pop_back(); // layer

	    QString map = split.last();
	    split.pop_back(); // map

	    QString mapset = split.last();
	    split.pop_back(); // mapset
	    
	    QString loc = sep + split.join ( QChar(sep) );

	    QString curloc = QDir::cleanDirPath ( QgsGrass::getDefaultGisdbase() + sep 
		             + QgsGrass::getDefaultLocation() );
            
	    if ( loc != curloc ) continue;

	    mLayerComboBox->insertItem( layer->name() );
	    if ( layer->name() == current ) mLayerComboBox->setCurrentText ( current );
	    mMaps.push_back ( map + "@" + mapset );
	} 
	else if ( mType == Raster && layer->type() == QgsMapLayer::RASTER ) 
	{
	    // Check if it is GRASS raster
	    QString source = QDir::cleanDirPath ( layer->source() ); 

	    char sep = QDir::separator();
	    if ( source.contains( "cellhd" ) == 0 ) continue;
	    
	    // Most probably GRASS layer, check GISBASE and LOCATION
	    QStringList split = QStringList::split ( sep, source );
            
	    if ( split.size() < 4 ) continue;

	    QString map = split.last();
	    split.pop_back(); // map
	    if ( split.last() != "cellhd" ) continue;
	    split.pop_back(); // cellhd

	    QString mapset = split.last();
	    split.pop_back(); // mapset
	    
	    QString loc = sep + split.join ( QChar(sep) );

	    QString curloc = QDir::cleanDirPath ( QgsGrass::getDefaultGisdbase() + sep 
		             + QgsGrass::getDefaultLocation() );
            
	    if ( loc != curloc ) continue;

	    mLayerComboBox->insertItem( layer->name() );
	    if ( layer->name() == current ) mLayerComboBox->setCurrentText ( current );
	    mMaps.push_back ( map + "@" + mapset );
	}
    }

}

QStringList QgsGrassModuleInput::options()
{
    QStringList list;
    list.push_back( mKey + "=" + mMaps[mLayerComboBox->currentItem()] );
    return list;
}

QgsGrassModuleInput::~QgsGrassModuleInput()
{
}

QgsGrassModuleItem::QgsGrassModuleItem() { mHidden = false; }

bool QgsGrassModuleItem::hidden() { return mHidden; }

QStringList QgsGrassModuleItem::options() { return QStringList(); }

QgsGrassModuleItem::~QgsGrassModuleItem() {}
