/***************************************************************************
                              qgsgrassmodule.h
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

#include "ui_qgsgrassmodulebase.h"

#include "qgis.h"
#include "qgsfield.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QProcess>

class QgsGrassTools;
class QgsGrassModuleItem;
class QgsGrassModuleOptions;

class QgisInterface;
class QgsMapCanvas;
class QgsMapLayer;
class QgsVectorLayer;

class QComboBox;
class QDomNode;
class QDomElement;
class QLineEdit;
class QValidator;


/*! \class QgsGrassModule
 *  \brief Interface to GRASS modules.
 *
 */
class QgsGrassModule: public QDialog, private  Ui::QgsGrassModuleBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassModule( QgsGrassTools *tools, QString moduleName, QgisInterface *iface,
                    QString path, QWidget * parent = 0, Qt::WFlags f = 0 );

    //! Destructor
    ~QgsGrassModule();

    QString translate( QString string );

    //! Returns module label for module description path
    static QString label( QString path );

    /** \brief Returns pixmap representing the module
     * \param path module path without .qgm extension
     */
    static QPixmap pixmap( QString path, int height );

    //! Find element in GRASS module description by key, if not found, returned element is null
    static QDomNode nodeByKey( QDomElement gDocElem, QString key );

    //! Returns pointer to QGIS interface
    QgisInterface *qgisIface();

    // ! Options widget
    QgsGrassModuleOptions *options() { return mOptions; }

    // ! List of directories in PATH variable + current directory on Windows
    static QStringList mExecPath;
    static bool mExecPathInited;

    // ! Find in exec path
    //   returns full path or null string
    //   appends automaticaly .exe on Windows
    static QString findExec( QString file );

    // ! Check if file is in mExecPath
    static bool inExecPath( QString file );

    // ! Get executable + arguments. Executable is returned as first string.
    // On Window if the module is script the executable will be path to shell
    // Returns empty list if not found.
    static QStringList execArguments( QString module );

  public slots:
    //! Run the module with current options
    void on_mRunButton_clicked() { run(); }
    void run();

    //! Close the module tab
    void on_mCloseButton_clicked() { close(); }
    void close();

    //! Show output in map view
    void on_mViewButton_clicked() { viewOutput(); }
    void viewOutput();

    //! Running process finished
    void finished( int exitCode, QProcess::ExitStatus exitStatus );

    //! Read module's standard output
    void readStdout();

    //! Read module's standard error
    void readStderr();

  private:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

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

    //! Last raster output
    QStringList mOutputRaster;

    //! Last vector output
    QStringList mOutputVector;

    //! True if the module successfully finished
    bool mSuccess;
};

/*! \class QgsGrassModuleOptions
 *  \brief Widget with GRASS options.
 *
 */
class QgsGrassModuleOptions
{
  public:
    //! Constructor
    QgsGrassModuleOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface );

    //! Destructor
    virtual ~QgsGrassModuleOptions();

    //! Get module options as list of arguments for QProcess
    virtual QStringList arguments();

    //! Check if output exists
    // return empty list
    // return list of existing output maps
    virtual QStringList checkOutput() { return QStringList() ; }

    //! Freeze output vector maps used in QGIS on Windows
    virtual void freezeOutput() { return; }

    //! Thaw output vector maps used in QGIS on Windows
    virtual void thawOutput() { return; }

    //! Check if option is ready
    //  Returns empty string or error message
    virtual QStringList ready() { return QStringList() ; }

    //! Get list of current output maps
    virtual QStringList output( int type ) { return QStringList() ; }

    //! Has any output
    virtual bool hasOutput( int type ) { return true; }

    //! Has raster input or output
    virtual bool usesRegion() { return false; }

    //! One or more input maps were switched on to be used as region
    virtual bool requestsRegion() { return false; }

    //! Check region
    // return empty list
    // return list of input maps (both raster and vector) outside region
    virtual QStringList checkRegion() { return QStringList() ; }

    //! Get region covering all input maps
    // \param all true all input maps
    // \param all false only the mas which were switched on
    virtual bool inputRegion( struct Cell_head *window, bool all ) { return false; }

    // ! Flag names
    virtual QStringList flagNames() { return QStringList() ; }

  protected:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

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
class QgsGrassModuleStandardOptions: QWidget, public QgsGrassModuleOptions
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassModuleStandardOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface,
      QString xname, QDomElement docElem,
      QWidget * parent = 0, Qt::WFlags f = 0 );

    //! Destructor
    ~QgsGrassModuleStandardOptions();

    //! Get module options as list of arguments for QProcess
    QStringList arguments();

    // ! Get item by ID
    QgsGrassModuleItem *item( QString id );

    // ! Get item by key
    QgsGrassModuleItem *itemByKey( QString key );

    // Reimplemented methods from QgsGrassModuleOptions
    QStringList checkOutput();
    void freezeOutput();
    void thawOutput();
    QStringList ready() ;
    QStringList output( int type );
    bool hasOutput( int type );
    QStringList checkRegion();
    bool usesRegion();
    bool requestsRegion();
    bool inputRegion( struct Cell_head *window, bool all );
    QStringList flagNames() { return mFlagNames; }

  public slots:
    // ! Show/hide advanced options
    void switchAdvanced();

  private:
    //! Name of module executable
    QString mXName;

    //! Path to module executable
    QString mXPath;

    //! Option items
    std::vector<QgsGrassModuleItem*> mItems;

    //! List of all flags. Necessary for scripts.
    QStringList mFlagNames;

    //! Use of region defined in qgm
    bool mUsesRegion;

    // ! Advanced options switch button
    QPushButton mAdvancedPushButton;

    // ! Advanced options frame
    QFrame mAdvancedFrame;
};

/****************** QgsGrassModuleGroupBoxItem ************************/

/*! \class QgsGrassModuleCheckBox
 *  \brief Checkbox with elided text
 */
class QgsGrassModuleCheckBox: public QCheckBox
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     */
    QgsGrassModuleCheckBox( const QString & text, QWidget * parent = 0 );

    //! Destructor
    virtual ~QgsGrassModuleCheckBox();

    void resizeEvent( QResizeEvent * event );

  public slots:
    void setText( const QString & text );

    void setToolTip( const QString & text );

    //! Adjust title size, called on resize
    void adjustText();

  private:
    QString mText;

    QString mTip;
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
    QgsGrassModuleItem( QgsGrassModule *module, QString key,
                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode );

    //! Destructor
    virtual ~QgsGrassModuleItem();

    //! Is the item hidden
    bool hidden();

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

    //! Item's key
    QString key() { return mKey; }

    //! Item's id
    QString id() { return mId; }

    //! Check if otpion is ready
    //  Returns empty string or error message
    virtual QString ready() { return QString() ; }

  protected:

    //! Pointer to GRASS module
    QgsGrassModule *mModule;

    //! Option key, for flags without '-'
    QString mKey;

    //! Optional option id used by other options which depend on this
    QString mId;

    //! Item title, GRASS label or description
    QString mTitle;

    //! Item tooltip, GRASS description if defined
    QString mToolTip;

    //! Hidden option or displayed
    bool mHidden;

    //! Predefined answer from config
    QString mAnswer;

    //! Is it required
    bool mRequired;

  private:

};

/****************** QgsGrassModuleGroupBoxItem ************************/

/*! \class QgsGrassModuleGroupBoxItem
 *  \brief GRASS module option box
 */
class QgsGrassModuleGroupBoxItem: public QGroupBox, public QgsGrassModuleItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     * \param gnode option node in GRASS module XML description file
     */
    QgsGrassModuleGroupBoxItem( QgsGrassModule *module, QString key,
                                QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                                QWidget * parent = 0 );

    //! Destructor
    virtual ~QgsGrassModuleGroupBoxItem();

    void resizeEvent( QResizeEvent * event );

  public slots:
    //! Adjust title size, called on resize
    void adjustTitle();

};

/****************** QgsGrassModuleOption ************************/

/*! \class QgsGrassModuleOption
 *  \brief  GRASS option
 */
class QgsGrassModuleOption: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleOption( QgsGrassModule *module, QString key,
                          QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                          QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleOption();

    //! Control option
    enum ControlType { NoControl, LineEdit, ComboBox, SpinBox, CheckBoxes };

    //! Control option
    enum ValueType { Double, Integer, String };

    //! Output type
    enum OutputType { None, Vector, Raster };

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

    //! True if this option is output
    bool isOutput() { return mIsOutput; }

    //! Returns output type
    int outputType() { return mOutputType; }

    //! If output, check if current output exists
    // Returns emppty string or name of existing output
    QString outputExists();

    QString ready() ;

    //! Current value
    QString value();

    //! Does this options causes use of region?
    //  Raster input/output uses region by default
    //  Use of region can be forced by 'region' attribute in qgm
    bool usesRegion() { return mUsesRegion; }

    //! Check min/max version
    static bool checkVersion( QString version_min, QString version_max );

  public slots:
    // Add new line edit for multiple options
    void addLineEdit();

    // Remove one line edit for multiple options
    void removeLineEdit();

  private:
    //! Control type
    ControlType mControlType;

    //! Value type
    ValueType mValueType;

    //! Output type
    OutputType mOutputType;

    //! If have defined value limits
    bool mHaveLimits;
    double mMin, mMax;

    //! Combobox
    QComboBox *mComboBox;

    //! Vector of values for combobox
    std::vector<QString> mValues;

    //! Check boxes
    std::vector<QgsGrassModuleCheckBox*> mCheckBoxes;

    //! Line
    std::vector<QLineEdit*> mLineEdits;

    //! True if this option is GRASS output
    bool mIsOutput;

    //! Output element
    QString mOutputElement;

    //! Line input validator
    QValidator *mValidator;

    // Layout inside box
    QVBoxLayout *mLayout;

    //! Uses region
    bool mUsesRegion;
};
/********************** QgsGrassModuleFlag ************************/
/*! \class QgsGrassModuleFlag
 *  \brief  GRASS flag
 */
class QgsGrassModuleFlag: public QgsGrassModuleCheckBox, public QgsGrassModuleItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleFlag( QgsGrassModule *module, QString key,
                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                        QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleFlag();

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

};

/************************ QgsGrassModuleInput **********************/

/*! \class QgsGrassModuleInput
 *  \brief Class representing raster or vector module input
 */
class QgsGrassModuleInput: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleInput( QgsGrassModule *module,
                         QgsGrassModuleStandardOptions *options, QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleInput();

    enum Type { Vector, Raster };

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

    // ! Return vector of attribute fields of current vector
    std::vector<QgsField> currentFields();

    //! Returns pointer to currently selected layer or null
    QgsMapLayer * currentLayer();

    QString currentMap();

    QString ready() ;

    //! Does this options causes use of region?
    //  Raster input/output uses region by default
    //  Use of region can be forced by 'region' attribute in qgm
    bool usesRegion() { return mUsesRegion; }

    //! Should be used region of this input
    bool useRegion();

    int type() { return mType; }

  public slots:
    //! Fill combobox with currently available maps in QGIS canvas
    void updateQgisLayers();

    void changed( int );

  signals:
    // emited when value changed/selected
    void valueChanged();

  private:
    //! Input type
    Type mType;

    // Module options
    QgsGrassModuleStandardOptions *mModuleStandardOptions;

    //! Vector type mask read from option defined by "typeoption" tag, used for QGIS layers in combo
    //  + type mask defined in configuration fil
    int mGeometryTypeMask;

    //! Name of vector type option associated with this input
    QString mGeometryTypeOption;

    //! Name of vector layer option associated with this input
    QString mVectorLayerOption;

    //! Combobox for QGIS layers
    QComboBox *mLayerComboBox;

    //! Region button
    QPushButton *mRegionButton;

    //! Optional map option id, if defined, only the layers from the
    //  map currently selected in that option are available.
    //  This is used by nodes layer option for networks.
    QString mMapId;

    //! Vector of map@mapset in the combobox
    QStringList mMaps;

    //! Type of vector in the combobox
    QStringList mGeometryTypes;

    //! Layer names in the combobox
    QStringList mVectorLayerNames;

    //! Pointers to vector layers in combobox
    std::vector<QgsMapLayer*> mMapLayers;

    //! Attribute fields of layers in the combobox
    std::vector< std::vector<QgsField> > mVectorFields;

    //! The imput map will be updated -> must be from current mapset
    bool mUpdate;

    //! Uses region
    bool mUsesRegion;

    //! Required field
    bool mRequired;
};

/*********************** QgsGrassModuleGdalInput **********************/

/*! \class QgsGrassModuleGdalInput
 *  \brief GDAL/OGR module input
 */
class QgsGrassModuleGdalInput: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleGdalInput( QgsGrassModule *module, int type, QString key,
                             QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                             QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleGdalInput();

    enum Type { Gdal, Ogr };

    //! Reimplemented
    QStringList options();
    QString ready();

  public slots:
    //! Fill combobox with currently available maps in QGIS canvas
    void updateQgisLayers();

  private:
    //! Input type
    int mType;

    //! Ogr layer option associated with this input
    QString mOgrLayerOption;

    //! Ogr sql option associated with this input
    QString mOgrWhereOption;

    //! Combobox for QGIS layers
    QComboBox *mLayerComboBox;

    //! Vector of URI in the combobox
    QStringList mUri;

    //! Ogr layer options
    QStringList mOgrLayers;

    //! Ogr where clauses
    QStringList mOgrWheres;

    //! Required field
    bool mRequired;
};

/*********************** QgsGrassModuleField **********************/

/*! \class QgsGrassModuleField
 *  \brief GRASS vector attribute column.
 */
class QgsGrassModuleField: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleField( QgsGrassModule *module,
                         QgsGrassModuleStandardOptions *options,
                         QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleField();

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

  public slots:
    //! Fill combobox with currently available maps in QGIS canvas
    void updateFields();

  private:
    // Module options
    QgsGrassModuleStandardOptions *mModuleStandardOptions;

    //! Layer key
    QString mLayerKey;

    //! Pointer to layer input
    QgsGrassModuleInput *mLayerInput;

    // ! Field type (integer,double,string,datetime)
    QString mType;

    //! Combobox for QGIS layer fields
    QComboBox *mFieldComboBox;
};

/*********************** QgsGrassModuleSelection **********************/

/*! \class QgsGrassModuleSelection
 *  \brief List of categories taken from current layer selection.
 */
class QgsGrassModuleSelection: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleSelection( QgsGrassModule *module,
                             QgsGrassModuleStandardOptions *options,
                             QString key,
                             QDomElement &qdesc, QDomElement &gdesc,
                             QDomNode &gnode,
                             QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleSelection();

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

  public slots:
    //! Set selection list to currently selected features
    void updateSelection();

  private:
    // Module options
    QgsGrassModuleStandardOptions *mModuleStandardOptions;

    //! Layer key
    QString mLayerId;

    //! Pointer to layer input
    QgsGrassModuleInput *mLayerInput;

    //! Currently connected layer
    QgsVectorLayer *mVectorLayer;

    // ! Field type (integer,double,string,datetime)
    QString mType;

    //! Line
    QLineEdit *mLineEdit;
};

/*********************** QgsGrassModuleFile **********************/

/*! \class QgsGrassModuleFile
 *  \brief Input/output file.
 */
class QgsGrassModuleFile: public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /*! \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleFile( QgsGrassModule *module,
                        QString key,
                        QDomElement &qdesc, QDomElement &gdesc,
                        QDomNode &gnode,
                        QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleFile();

    //! File type
    enum Type { Old, New, Multiple, Directory };

    // Reimplemented methods from QgsGrassModuleOptions
    QStringList options();
    QString ready() ;

  public slots:
    // browse files
    void browse();

  private:
    // ! File type (New, Old)
    int mType;

    // ! Optionaly split file to dir and file path
    QString mFileOption;

    // ! Default suffix
    QString mSuffix;

    //! Line
    QLineEdit *mLineEdit;

    //! Browse button
    QPushButton *mBrowseButton;

    //! File filters
    QStringList mFilters;
};


#endif // QGSGRASSMODULE_H
