/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <iostream>


void QgisAppBase::init()
{
}

void QgisAppBase::fileExit()
{

}


void QgisAppBase::fileOpen()
{

}


void QgisAppBase::addDatabaseLayer()
{
#ifdef QGISDEBUG
    std::cerr << __FILE__ << ":" << __LINE__ 
	    << " QgisAppBase::addDatabaseLayer() invoked instead of QgisApp::addDatabaseLayer().\n\tIs HAVE_POSTGRESQL defined?\n";
#endif
}


void QgisAppBase::zoomIn()
{

}

void QgisAppBase::zoomOut()
{

}

void QgisAppBase::zoomToSelected()
{

}




void QgisAppBase::drawLayers()
{

}


void QgisAppBase::zoomFull()
{

}

void QgisAppBase::pan()
{

}

void QgisAppBase::capturePolygon()
{

}

void QgisAppBase::testButton()
{

}

void QgisAppBase::addLayer()
{

}

void QgisAppBase::identify()
{

}

void QgisAppBase::attributeTable()
{
	
}

void QgisAppBase::zoomPrevious()
{

}

void QgisAppBase::testPluginFunctions()
{

}

void QgisAppBase::actionOptions_activated()
{

}


void QgisAppBase::options()
{

}


void QgisAppBase::fileSave()
{

}


void QgisAppBase::fileSaveAs()
{

}


void QgisAppBase::fileNew()
{

}


void QgisAppBase::actionPluginManager_activated()
{

}


void QgisAppBase::checkQgisVersion()
{

}

void QgisAppBase::select()
{

}


void QgisAppBase::exportMapServer()
{

}


void QgisAppBase::addRasterLayer()
{

}


void QgisAppBase::helpContents()
{

}


void QgisAppBase::helpQgisSourceForge()
{

}


void QgisAppBase::helpQgisHomePage()
{

}


void QgisAppBase::saveMapAsImage()
{

}


void QgisAppBase::whatsThis()
{

}



void QgisAppBase::projectProperties()
{

}





void QgisAppBase::filePrint()
{

}


void QgisAppBase::capturePoint()
{

}



void QgisAppBase::addAllToOverview()
{

}


void QgisAppBase::removeAllFromOverview()
{

}



void QgisAppBase::hideAllLayers()
{
    std::cout <<"hide all layers" << std::endl;
}


void QgisAppBase::showAllLayers()
{

}


void QgisAppBase::captureLine()
{

}




void QgisAppBase::newVectorLayer()
{

}


void QgisAppBase::removeLayer()
{
#ifdef QGISDEBUG
    qDebug("QgisAppBase::removeLayer()");
#endif
}


void QgisAppBase::inOverview( bool )
{

}


void QgisAppBase::menubar_activated( int )
{

}



void QgisAppBase::menubar_highlighted(int)
{

}

void QgisAppBase::about()
{
}
