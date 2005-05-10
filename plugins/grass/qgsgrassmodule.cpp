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
#include <qvgroupbox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <qimage.h>

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
    QString mpath = mPath + ".qgm";
    QFile qFile ( mpath );
    if ( !qFile.exists() ) {
	QMessageBox::warning( 0, "Warning", "The module file (" + mpath + ") not found." );
	return;
    }
    if ( ! qFile.open( IO_ReadOnly ) ) {
	QMessageBox::warning( 0, "Warning", "Cannot open module file (" + mpath + ")" );
	return;
    }
    QDomDocument qDoc ( "qgisgrassmodule" );
    QString err;
    int line, column;
    if ( !qDoc.setContent( &qFile,  &err, &line, &column ) ) {
	QString errmsg = "Cannot read module file (" + mpath + "):\n" + err + "\nat line "
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
    //mXPath = gisBase + "/bin/" + mXName;
    QProcess *process = new QProcess( this );
    process->addArgument( mXName );
    process->addArgument( "--interface-description" );
    if ( !process->start() ) {
	QMessageBox::warning( 0, "Warning", "Cannot start module " + mXName );
	return;
    }
    while ( process->isRunning () ) { // TODO: check time, if it is not running too long
    }
    QByteArray gDescArray = process->readStdout();
    delete process;

    QDomDocument gDoc ( "task" );
    if ( !gDoc.setContent( (QByteArray)gDescArray, &err, &line, &column ) ) {
	QString errmsg = "Cannot read module description (" + mXName + "):\n" + err + "\nat line "
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

	    QString key = e.attribute("key");
	    std::cout << "key = " << key << std::endl;

	    QDomNode gnode = nodeByKey ( gDocElem, key );
	    if ( gnode.isNull() ) {
		QMessageBox::warning( 0, "Warning", "Cannot find key " +  key );
		return;
	    }

	    if ( optionType == "option" ) {
	        bool created = false;

		// Check option type and create appropriate control
		QDomNode promptNode = gnode.namedItem ( "gisprompt" );
		if ( !promptNode.isNull() ) {
		    QDomElement promptElem = promptNode.toElement();
		    QString element = promptElem.attribute("element"); 
		    QString age = promptElem.attribute("age"); 
		    //std::cout << "element = " << element << " age = " << age << std::endl;
		    if ( age == "old" && ( element == "vector" || element == "cell") ) {
			QgsGrassModuleInput *mi = new QgsGrassModuleInput ( this, key, e, gDocElem, 
				                                gnode, mTabWidget->page(0) );

			layout->addWidget ( mi );
			created = true;
			mItems.push_back(mi);
		    }
		} 

		if ( !created ) {
		    QgsGrassModuleOption *so = 
			new QgsGrassModuleOption ( this, key, e, gDocElem, gnode, mTabWidget->page(0) );
		    
			layout->addWidget ( so );
			created = true;
		    mItems.push_back(so);
		}
	    } else if ( optionType == "flag" )  {
		    QgsGrassModuleFlag *flag = 
			new QgsGrassModuleFlag ( this, key, e, gDocElem, gnode, mTabWidget->page(0) );
		    
		    layout->addWidget ( flag );
		    mItems.push_back(flag);
	    }
	}
	n = n.nextSibling();
    }

    QSpacerItem *si = new QSpacerItem ( 10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout->addItem ( si );

    // Create manual if available
    QString manPath = gisBase + "/docs/html/" + mXName + ".html";
    QFile manFile ( manPath );
    if ( manFile.exists() ) {
	mManualTextBrowser->setSource ( manPath );
    }
    
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
    path.append ( ".qgm" );
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

QPixmap QgsGrassModule::pixmap ( QString path, int height ) 
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModule::pixmap()" << std::endl;
    #endif

    std::vector<QPixmap> pixmaps;

    // Create vector of available pictures
    int cnt = 1;
    while ( 1 ) 
    {
	// SVG
	QString fpath = path + "." + QString::number(cnt) + ".svg";
        QFileInfo fi ( fpath );
        if ( fi.exists() ) 
	{
	    QPicture pic;
	    if ( ! pic.load ( fpath, "svg" ) ) break;

	    QRect br = pic.boundingRect();

	    double scale = 1. * height / br.height();
	    int width = (int) ( scale * br.width() );
            if ( width <= 0 ) width = height; //should not happen
	    QPixmap pixmap ( width, height );
	    pixmap.fill ( QColor(255,255,255) );
	    QPainter painter ( &pixmap );
	    painter.scale ( scale, scale );
	    painter.drawPicture ( -br.x(), -br.y(), pic );
	    painter.end();
	    
	    pixmaps.push_back ( pixmap );
	} 
	else // PNG 
	{
	    fpath = path + "." + QString::number(cnt) + ".png";
	    fi.setFile ( fpath );

	    if ( !fi.exists() ) break;

	    QPixmap pixmap;
	   
	    if ( ! pixmap.load(fpath, "PNG" ) ) break;
	    
	    double scale = 1. * height / pixmap.height();
	    int width = (int) ( scale * pixmap.width() );

	    QImage img = pixmap.convertToImage();
	    img = img.smoothScale ( width, height ); 
	    pixmap.convertFromImage ( img );

	    pixmaps.push_back ( pixmap );
	}
        cnt++;
    }	

    // Get total width
    int width = 0;
    for ( int i = 0; i < pixmaps.size(); i++ ) {
	width += pixmaps[i].width();
    }

    if ( width <= 0 ) width = height; //should not happen
    
    int swidth = 20; // sign
    if ( pixmaps.size() > 1 ) width += swidth; // ->
    if ( pixmaps.size() > 2 ) width += swidth; // +

    QPixmap pixmap ( width, height );
    pixmap.fill(QColor(255,255,255));
    QPainter painter ( &pixmap );
    
    int pos = 0;
    for ( int i = 0; i < pixmaps.size(); i++ ) {
	if ( i == 1 && pixmaps.size() == 3 ) { // +
	    painter.drawLine ( (int)pos+swidth/2-3, (int)height/2, (int)pos+swidth/2+3, (int)height/2 );
	    painter.drawLine ( (int)pos+swidth/2, (int)height/2-3, (int)pos+swidth/2, (int)height/2+3 );
	    pos += swidth;
	}
	if ( (i == 1 && pixmaps.size() == 2) || (i == 2 && pixmaps.size() == 3)  ) { // ->
	    painter.setPen ( QColor(0,0,0) );
	    painter.drawLine ( pos+3, (int)height/2, pos+swidth-3, (int)height/2 );
	    painter.drawLine ( (int)pos+swidth/2, (int)height/2-3, pos+swidth-2, (int)height/2 );
	    painter.drawLine ( (int)pos+swidth/2, (int)height/2+3, pos+swidth-2, (int)height/2 );
	    pos += swidth;
	}
	painter.drawPixmap ( pos,0, pixmaps[i] );
	pos += pixmaps[i].width();
    }
    painter.end();

    return pixmap;
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
	mProcess.addArgument( mXName );
	command = mXName;

	for ( int i = 0; i < mItems.size(); i++ ) {
	    QStringList list = mItems[i]->options();

	    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		std::cerr << "option: " << *it << std::endl;
		command.append ( " " + *it );
		mProcess.addArgument( *it );
	    }
	}

	/* WARNING - TODO: there was a bug in GRASS 6.0.0 / 6.1.CVS (< 2005-04-29):
	 * db_start_driver set GISRC_MODE_MEMORY eviroment variable to 1 if 
	 * G_get_gisrc_mode() == G_GISRC_MODE_MEMORY but the variable wasn't unset 
	 * if  G_get_gisrc_mode() == G_GISRC_MODE_FILE. Because QGIS GRASS provider starts drivers in 
	 * G_GISRC_MODE_MEMORY mode, the variable remains set in variable when a module is run
	 * -> unset GISRC_MODE_MEMORY. Remove later once 6.1.x / 6.0.1 is widespread.
	 */
	putenv ( "GISRC_MODE_MEMORY" );  // unset
	
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
       	line = QString::fromLocal8Bit( mProcess.readLineStdout().ascii() );
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
       	line = QString::fromLocal8Bit( mProcess.readLineStderr().ascii() );
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

QgsGrassModuleOption::QgsGrassModuleOption ( QgsGrassModule *module, QString key, 
	                                   QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
	                                   QWidget * parent)
                    :QVGroupBox ( parent ), QgsGrassModuleItem ( module, key, qdesc, gdesc, gnode )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModuleOption::QgsGrassModuleOption" << std::endl;
    #endif
    setSizePolicy ( QSizePolicy::Preferred, QSizePolicy::Minimum );
    
    if ( mHidden ) hide();

    QString tit;
    if ( mDescription.length() > 40 ) {
	tit = mDescription.left(40) + " ...";
    } else {
	tit = mDescription;
    }
	    
    setTitle ( " " + tit + " " );
	
    // String without options 
    if ( !mHidden ) 
    {
	
	// Predefined values ?
	QDomNode valuesNode = gnode.namedItem ( "values" );
	
	if ( !valuesNode.isNull() ) // predefined values -> ComboBox or CheckBox
	{
	    // one or many?
	    QDomElement gelem = gnode.toElement();
   	    if ( gelem.attribute("multiple") == "yes" ) {
		mControlType = CheckBoxes;
	    } else {
	        mControlType = ComboBox;
		mComboBox = new QComboBox ( this );
	    }

	    // List of values to be excluded 
	    QStringList exclude = QStringList::split ( ',', qdesc.attribute("exclude") );

	    QDomElement valuesElem = valuesNode.toElement();
	    QDomNode valueNode = valuesElem.firstChild();

	    while( !valueNode.isNull() ) {
		QDomElement valueElem = valueNode.toElement();

		if( !valueElem.isNull() && valueElem.tagName() == "value" ) 
		{

		    QDomNode n = valueNode.namedItem ( "name" );
		    if ( !n.isNull() ) {
			QDomElement e = n.toElement();
			QString val = e.text().stripWhiteSpace();
			
			if ( exclude.contains(val) == 0 ) { 
			    n = valueNode.namedItem ( "description" );
			    QString desc;
			    if ( !n.isNull() ) {
				e = n.toElement();
				desc = e.text().stripWhiteSpace();
			    } else {
				desc = val;
			    }
			    desc.replace( 0, 1, desc.left(1).upper() );

			    if ( mControlType == ComboBox ) {
				mComboBox->insertItem ( desc );
			    } else {
				QCheckBox *cb = new QCheckBox ( desc, this );
				mCheckBoxes.push_back ( cb );
			    }
			    
			    mValues.push_back ( val );
			}
		    }
		}
		
		valueNode = valueNode.nextSibling();
	     }
	} 
	else // No values
	{
	    // Line edit
	    mControlType = LineEdit;

	    mLineEdit = new QLineEdit ( this );
	    QDomNode n = gnode.namedItem ( "default" );
	    if ( !n.isNull() ) {
		QDomElement e = n.toElement();
		QString def = e.text().stripWhiteSpace();
		mLineEdit->setText ( def );
	    }
	}
    }
}

QStringList QgsGrassModuleOption::options()
{
    QStringList list;
    
    if ( mHidden ) {
        list.push_back( mKey + "=" + mAnswer );
    } else {
	if ( mControlType == LineEdit ) {
            list.push_back( mKey + "=" + mLineEdit->text() );
	} else if ( mControlType == ComboBox ) {
            list.push_back( mKey + "=" + mValues[mComboBox->currentItem()] );
	} else if ( mControlType == CheckBoxes ) {
	    QString opt = mKey + "=";
	    int cnt = 0;
	    for ( int i = 0; i < mCheckBoxes.size(); i++ ) {
		if ( mCheckBoxes[i]->isChecked() ) {
		    if ( cnt > 0 ) opt.append ( "," );
		    opt.append ( mValues[i] );
		}
	    }
	    list.push_back( opt );
	}
    }
    return list;
}

QgsGrassModuleOption::~QgsGrassModuleOption()
{
}

QgsGrassModuleFlag::QgsGrassModuleFlag ( QgsGrassModule *module, QString key,
	                                   QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
	                                   QWidget * parent)
                    :QCheckBox ( parent ), QgsGrassModuleItem ( module, key, qdesc, gdesc, gnode )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrassModuleFlag::QgsGrassModuleFlag" << std::endl;
    #endif

    if ( mHidden ) hide();
    
    if ( mAnswer == "on" )
        setChecked ( true );
    else 
        setChecked ( false );
    
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

QgsGrassModuleInput::QgsGrassModuleInput ( QgsGrassModule *module, QString key,
	                                   QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
	                                   QWidget * parent)
                    :QVGroupBox ( parent ), QgsGrassModuleItem ( module, key, qdesc, gdesc, gnode ),
		     mUpdate(false), mVectorTypeOption(0), mVectorLayerOption(0)
{
    mVectorTypeMask = GV_POINT | GV_LINE | GV_AREA;

    QString tit;
    if ( mDescription.isEmpty() ) {
	tit = "Input";
    } else {
	if ( mDescription.length() > 40 ) {
	    tit = mDescription.left(40) + " ...";
	} else {
	    tit = mDescription;
	}
    }
	    
    setTitle ( " " + tit + " " );

    QDomNode promptNode = gnode.namedItem ( "gisprompt" );
    QDomElement promptElem = promptNode.toElement();
    QString element = promptElem.attribute("element"); 

    if ( element == "vector" ) 
    {
        mType = Vector;	

	// Read type mask if "typeoption" is defined
	QString opt = qdesc.attribute("typeoption");
	if ( ! opt.isNull() ) {
	
	    QDomNode optNode = QgsGrassModule::nodeByKey ( gdesc, opt );

	    if ( optNode.isNull() ) 
	    {
		QMessageBox::warning( 0, "Warning", "Cannot find typeoption " +  opt );
	    } 
	    else 
	    {
		mVectorTypeOption = opt;

		QDomNode valuesNode = optNode.namedItem ( "values" );
		if ( valuesNode.isNull() ) 
		{
		    QMessageBox::warning( 0, "Warning", "Cannot find values for typeoption " +  opt );
		}
		else
		{
		    mVectorTypeMask = 0; GV_POINT | GV_LINE | GV_AREA;
		    
		    QDomElement valuesElem = valuesNode.toElement();
		    QDomNode valueNode = valuesElem.firstChild();
	
		    while( !valueNode.isNull() ) {
			QDomElement valueElem = valueNode.toElement();

			if( !valueElem.isNull() && valueElem.tagName() == "value" ) 
			{
			    QDomNode n = valueNode.namedItem ( "name" );
			    if ( !n.isNull() ) {
				QDomElement e = n.toElement();
				QString val = e.text().stripWhiteSpace();
		
				if ( val == "point" ) {
				    mVectorTypeMask |= GV_POINT;
				} else if ( val == "line" ) {
				    mVectorTypeMask |= GV_LINE;
				} else if ( val == "area" ) {
				    mVectorTypeMask |= GV_AREA;
				}
			    }
			}
			
			valueNode = valueNode.nextSibling();
		     }
		}
	    }
	}

	// Read type mask defined in configuration
	opt = qdesc.attribute("typemask");
	if ( ! opt.isNull() ) {
	    int mask = 0;
	    
	    if ( opt.find("point") >= 0 ) {
		mask |= GV_POINT;
	    }
	    if ( opt.find("line") >= 0 ) {
		mask |= GV_LINE;
	    }
	    if ( opt.find("area") >= 0 ) {
		mask |= GV_AREA;
	    }
	
            mVectorTypeMask &= mask;
	}

	// Read "layeroption" is defined
	opt = qdesc.attribute("layeroption");
	if ( ! opt.isNull() ) {
	
	    QDomNode optNode = QgsGrassModule::nodeByKey ( gdesc, opt );

	    if ( optNode.isNull() ) 
	    {
		QMessageBox::warning( 0, "Warning", "Cannot find layeroption " +  opt );
	    } 
	    else 
	    {
		mVectorLayerOption = opt;
	    }
	}
    } 
    else if ( element == "cell" ) 
    {
       mType = Raster;
    } 
    else 
    {
	QMessageBox::warning( 0, "Warning", "GRASS element " + element + " not supported" );
    }

    if ( qdesc.attribute("update") == "yes" ) {
	mUpdate = true;
    }


    mLayerComboBox = new QComboBox ( this );

    // Of course, activated(int) is not enough, but there is no signal BEFORE the cobo is opened
    //connect ( mLayerComboBox, SIGNAL( activated(int) ), this, SLOT(updateQgisLayers()) );
    
    // Connect to canvas 
    QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();
    connect ( canvas, SIGNAL(addedLayer(QgsMapLayer *)), this, SLOT(updateQgisLayers()) );
    connect ( canvas, SIGNAL(removedLayer(QString)), this, SLOT(updateQgisLayers()) );
    
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
    mVectorTypes.resize(0);
    mVectorLayers.resize(0);

    QgsMapCanvas *canvas = mModule->qgisIface()->getMapCanvas();

    int nlayers = canvas->layerCount();
    for ( int i = 0; i < nlayers; i++ ) {
	QgsMapLayer *layer = canvas->getZpos(i);

	if (  mType == Vector && layer->type() == QgsMapLayer::VECTOR ) {
	    QgsVectorLayer *vector = (QgsVectorLayer*)layer;
	    if ( vector->providerType() != "grass" ) continue;

	    //TODO dynamic_cast ?
	    QgsGrassProvider *provider = (QgsGrassProvider *) vector->getDataProvider();

	    // Check type mask
	    int geomType = provider->geometryType();

	    if ( (geomType == QGis::WKBPoint && !(mVectorTypeMask & GV_POINT) ) ||
	         (geomType == QGis::WKBLineString && !(mVectorTypeMask & GV_LINE) ) ||
	         (geomType == QGis::WKBPolygon && !(mVectorTypeMask & GV_AREA) )
	       )
	    {
		continue;
	    }

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
	    
	    QDir locDir ( sep + split.join ( QChar(sep) ) ) ;
	    QString loc = locDir.canonicalPath();

	    QDir curlocDir ( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
	    QString curloc = curlocDir.canonicalPath();
            
	    if ( loc != curloc ) continue;

	    if ( mUpdate && mapset != QgsGrass::getDefaultMapset() ) continue;

	    mLayerComboBox->insertItem( layer->name() );
	    if ( layer->name() == current ) mLayerComboBox->setCurrentText ( current );
	    mMaps.push_back ( map + "@" + mapset );

	    if ( geomType == QGis::WKBPoint ) {
	        mVectorTypes.push_back ( "point" );
	    } else if ( geomType == QGis::WKBLineString ) {
	        mVectorTypes.push_back ( "line" );
	    } else if ( geomType == QGis::WKBPolygon ) {
	        mVectorTypes.push_back ( "area" );
	    } else {
	        mVectorTypes.push_back ( "unknown" );
	    }

	    mVectorLayers.push_back ( QString::number(provider->grassLayer()) );
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
	    
	    QDir locDir ( sep + split.join ( QChar(sep) ) ) ;
	    QString loc = locDir.canonicalPath();

	    QDir curlocDir ( QgsGrass::getDefaultGisdbase() + sep + QgsGrass::getDefaultLocation() );
	    QString curloc = curlocDir.canonicalPath();
            
	    if ( loc != curloc ) continue;

	    if ( mUpdate && mapset != QgsGrass::getDefaultMapset() ) continue;

	    mLayerComboBox->insertItem( layer->name() );
	    if ( layer->name() == current ) mLayerComboBox->setCurrentText ( current );
	    mMaps.push_back ( map + "@" + mapset );
	}
    }

}

QStringList QgsGrassModuleInput::options()
{
    QStringList list;

    int current = mLayerComboBox->currentItem();

    QString opt(mKey + "=");

    if ( current <  mMaps.size() ) {
	opt.append ( mMaps[current] );
    }
    list.push_back( opt );

    if ( !mVectorTypeOption.isNull() && current < mVectorTypes.size() ) 
    {
	opt = mVectorTypeOption + "=" + mVectorTypes[current] ;
	list.push_back( opt );
    }
	
    if ( !mVectorLayerOption.isNull() && current < mVectorLayers.size() ) 
    {
	opt = mVectorLayerOption + "=" + mVectorLayers[current] ;
	list.push_back( opt );
    }
    
    return list;
}

QgsGrassModuleInput::~QgsGrassModuleInput()
{
}

QgsGrassModuleItem::QgsGrassModuleItem( QgsGrassModule *module, QString key,
	 				QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode )
	:mKey(key),
	mHidden(false), 
	mModule(module)
{ 
    mAnswer = qdesc.attribute("answer", "");
    
    if ( qdesc.attribute("hidden") == "yes" ) {
	mHidden = true;
    }

    QDomNode n = gnode.namedItem ( "description" );
    if ( !n.isNull() ) {
        QDomElement e = n.toElement();
	mDescription = e.text().stripWhiteSpace();
	mDescription.replace( 0, 1, mDescription.left(1).upper() );
    }
}

bool QgsGrassModuleItem::hidden() { return mHidden; }

QStringList QgsGrassModuleItem::options() { return QStringList(); }

QgsGrassModuleItem::~QgsGrassModuleItem() {}
