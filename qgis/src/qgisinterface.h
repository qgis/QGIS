#ifndef QGISINTERFACE_H
#define QGISINTERFACE_H
/*
 * $Id$
 */
//#include "qgisapp.h"
#include <qwidget.h>
class QgisApp;
class QgsMapLayer;
class QPopupMenu;

// interface class for plugins
class QgisInterface : public QWidget{
  Q_OBJECT
    public:
      QgisInterface(QgisApp *qgis=0, const char *name=0) {};
      virtual ~QgisInterface() {};
    public slots:
      virtual void zoomFull()=0;
      virtual void zoomPrevious()=0;
      virtual void zoomActiveLayer()=0;
      //! Add a vector layer
      virtual void addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
      //! Add a raster layer
      virtual void addRasterLayer(QString rasterLayerPath)=0;
      //! Get pointer to the active layer (layer selected in the legend)
      virtual QgsMapLayer *activeLayer()=0;
      //! add a menu item to the main menu, postioned to the left of the Help menu
      virtual int addMenu(QString menuText, QPopupMenu *menu) =0;
      /** Open a url in the users browser. By default the QGIS doc directory is used
       * as the base for the URL. To open a URL that is not relative to the installed
       * QGIS documentation, set useQgisDocDirectory to false.
       * @param url URL to open
       * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
       * url to the QGIS documentation directory path (<prefix>/share/doc)
       */
      virtual void openURL(QString url, bool useQgisDocDirectory=true)=0;
    private:
      //QgisApp *qgis;
};

#endif //#ifndef QGISINTERFACE_H

