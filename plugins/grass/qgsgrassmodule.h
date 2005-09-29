/***************************************************************************
                              qgsgrasstools.h 
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
#ifndef QGSGRASSMODULE_H
#define QGSGRASSMODULE_H

class QCloseEvent;
class QString;
class QStringList;
class QGroupBox;
class QFrame;
class QListView;
class QDomNode;
class QDomElement;
class QComboBox;
class QLineEdit;
class QPixmap;

#include <vector>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qprocess.h>

// Must be here, so that it is included to moc file
#include "../../src/qgisapp.h"
#include "../../src/qgisiface.h"

class QgsGrassProvider;
class QgsGrassTools;
class QgsGrassModuleItem;
class QgsGrassModuleOptions; 
class QgsGrassModuleStandardOptions; 
#include "qgsgrassmodulebase.h"

/*! \class QgsGrassModule
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassModule: public QgsGrassModuleBase 
{
    Q_OBJECT;

public:
    //! Constructor
    QgsGrassModule ( QgsGrassTools *tools, QgisApp *qgisApp, QgisIface *iface,  
	           QString path, QWidget * parent = 0, const char * name = 0, WFlags f = 0 );

    //! Destructor
    ~QgsGrassModule();

    //! Returns module label for module description path
    static QString label ( QString path );

    /** \brief Returns pixmap representing the module 
     * \param path module path without .qgm extension
     */
    static QPixmap pixmap ( QString path, int height );

    //! Find element in GRASS module description by key, if not found, returned element is null
    static QDomNode nodeByKey ( QDomElement gDocElem, QString key );

    //! Returns pointer to QGIS interface 
    QgisIface *qgisIface();

    //! Returns pointer to QGIS application
    QgisApp *qgisApp();

public slots:
    //! Run the module with current options
    void run ();

    //! Close the module tab
    void close ();

    //! Running process finished
    void finished ();

    //! Read module's standard output
    void readStdout();

    //! Read module's standard error
    void readStderr();

private:
    //! QGIS application
    QgisApp *mQgisApp;

    //! Pointer to the QGIS interface object
    QgisIface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Pointer to GRASS Tools 
    QgsGrassTools *mTools;

    //! Module definition file path (.qgm file)
    QString mPath;

    //! Name of module executable 
    QString mXName;

    //! Path to module executable 
    QString mXPath;

    //! Parent widget
    QWidget *mParent;

    //! Running GRASS module
    QProcess mProcess;

    //! QGIS directory
    QString mAppDir;

    //! Pointer to options widget
    QgsGrassModuleOptions *mOptions; 
};

/*! \class QgsGrassModuleOptions
 *  \brief Widget with GRASS options.
 *
 */
class QgsGrassModuleOptions
{
public:
    //! Constructor
    QgsGrassModuleOptions ( 
            QgsGrassTools *tools, QgsGrassModule *module, 
            QgisApp *qgisApp, QgisIface *iface ); 

    //! Destructor
    virtual ~QgsGrassModuleOptions();

    //! Get module options as list of arguments for QProcess
    virtual QStringList arguments();

protected:
    //! QGIS application
    QgisApp *mQgisApp;

    //! Pointer to the QGIS interface object
    QgisIface *mIface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    //! Pointer to GRASS Tools 
    QgsGrassTools *mTools;

    //! Pointer to GRASS module
    QgsGrassModule *mModule;

    //! Parent widget
    QWidget *mParent;

    //! QGIS directory
    QString mAppDir;
};

/*! \class QgsGrassModuleStandardOptions
 *  \brief Widget with GRASS standard options.
 *
 */
class QgsGrassModuleStandardOptions: public QgsGrassModuleOptions, QWidget 
{
public:
    //! Constructor
    QgsGrassModuleStandardOptions ( 
            QgsGrassTools *tools, QgsGrassModule *module, 
            QgisApp *qgisApp, QgisIface *iface,  
	    QString xname, QDomElement docElem,
            QWidget * parent = 0, const char * name = 0, WFlags f = 0 );

    //! Destructor
    ~QgsGrassModuleStandardOptions();

    //! Get module options as list of arguments for QProcess
    QStringList arguments();

private:
    //! Name of module executable 
    QString mXName;

    //! Path to module executable 
    QString mXPath;

    //! Option items
    std::vector<QgsGrassModuleItem*> mItems;

};

/*! \class QgsGrassModuleItem
 *  \brief GRASS module option
 */
class QgsGrassModuleItem
{
public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     * \param gnode option node in GRASS module XML description file
     */
    QgsGrassModuleItem ( QgsGrassModule *module, QString key, 
	                 QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode );

    //! Destructor
    virtual ~QgsGrassModuleItem();

    //! Is the item hidden
    bool hidden();

    //! Retruns list of options which will be passed to module
    virtual QStringList options(); 

protected:

    //! Pointer to GRASS module
    QgsGrassModule *mModule;

    //! Option key, for flags without '-'
    QString mKey;

    //! GRASS description
    QString mDescription;

    //! Hidden option or displayed
    bool mHidden;

    //! Predefined answer from config
    QString mAnswer;
private:

};

/*! \class QgsGrassModuleOption
 *  \brief  GRASS option 
 */
class QgsGrassModuleOption: public QGroupBox, public QgsGrassModuleItem
{
    Q_OBJECT;

public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleOption ( QgsGrassModule *module, QString key,
	                  QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                          QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleOption();

    //! Control option
    enum ControlType { LineEdit, ComboBox, SpinBox, CheckBoxes };
    
    //! Retruns list of options which will be passed to module
    virtual QStringList options(); 
    

private:
    //! Control type
    ControlType mControlType;
    
    //! Combobox
    QComboBox *mComboBox;

    //! Vector of values for combobox
    std::vector<QString> mValues;

    //! Check boxes
    std::vector<QCheckBox*> mCheckBoxes;
    
    //! Combobox
    QLineEdit *mLineEdit;
};

/*! \class QgsGrassModuleFlag
 *  \brief  GRASS flag
 */
class QgsGrassModuleFlag: public QCheckBox, public QgsGrassModuleItem
{
    Q_OBJECT;

public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleFlag ( QgsGrassModule *module, QString key,
	                  QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                          QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleFlag();
    //! Retruns list of options which will be passed to module
    virtual QStringList options(); 
private:
};

/*! \class QgsGrassModuleInput
 *  \brief Class representing raster or vector module input
 */
class QgsGrassModuleInput: public QGroupBox, public QgsGrassModuleItem
{
    Q_OBJECT;

public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleInput ( QgsGrassModule *module, QString key,
	                  QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                          QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleInput();

    enum Type { Vector, Raster };

    //! Retruns list of options which will be passed to module
    virtual QStringList options(); 

public slots:
    //! Fill combobox with currently available maps in QGIS canvas
    void updateQgisLayers();

private:
    //! Input type
    Type mType;

    //! Vector type mask read from option defined by "typeoption" tag, used for QGIS layers in combo
    //  + type mask defined in configuration fil
    int mVectorTypeMask;    

    //! Name of vector type option associated with this input
    QString mVectorTypeOption;
    
    //! Name of vector layer option associated with this input
    QString mVectorLayerOption;
    
    //! Combobox for QGIS layers
    QComboBox *mLayerComboBox;

    //! Vector of map@mapset in the combobox
    std::vector<QString> mMaps;
    
    //! Type of vector in the combobox
    std::vector<QString> mVectorTypes;
    
    //! Layers of vector in the combobox
    std::vector<QString> mVectorLayers;

    //! The imput map will be updated -> must be from current mapset
    bool mUpdate;
};
#endif // QGSGRASSMODULE_H
