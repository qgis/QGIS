/***************************************************************************
                          qgsgrassmoduleoptions.h
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSMODULEOPTIONS_H
#define QGSGRASSMODULEOPTIONS_H

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "qgis.h"
#include "qgsfield.h"
#include "qgscoordinatereferencesystem.h"

class QgsGrassTools;
class QgsGrassModule;
class QgsGrassModuleItem;
class QgsGrassModuleOptions;

class QgisInterface;
class QgsMapCanvas;
class QgsMapLayer;
class QgsVectorLayer;

class QDomNode;
class QDomElement;
class QValidator;

/** \class QgsGrassModuleOptions
 *  \brief Widget with GRASS options.QgsGrassTools
 *
 */
class QgsGrassModuleOptions
{
  public:
    enum RegionMode
    {
      RegionInput = 1,  // intersection of input maps extent and highest input resolution
      RegionCurrent = 0 // current map canvas extent and resolution
    };
    //! Constructor
    QgsGrassModuleOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface, bool direct );

    //! Destructor
    virtual ~QgsGrassModuleOptions();

    //! Get module options as list of arguments for QProcess
    virtual QStringList arguments();

    //! Check if output exists
    // return empty list
    // return list of existing output maps
    virtual QStringList checkOutput() { return QStringList() ; }

    //! Freeze output vector maps used in QGIS on Windows
    virtual void freezeOutput() {}

    //! Thaw output vector maps used in QGIS on Windows
    virtual void thawOutput() { }

    //! Check if option is ready
    //  Returns empty string or error message
    virtual QStringList ready() { return QStringList() ; }

    //! Get list of current output maps
    virtual QStringList output( int type )
    { Q_UNUSED( type ); return QStringList(); }

    //! Has any output
    virtual bool hasOutput( int type )
    { Q_UNUSED( type ); return true; }

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
    virtual bool inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem & crs, bool all )
    { Q_UNUSED( window ); Q_UNUSED( crs ); Q_UNUSED( all ); return false; }

    // ! Flag names
    virtual QStringList flagNames() { return QStringList() ; }

    QStringList errors() { return mErrors; }

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
    //QWidget *mParent;

    //! QGIS directory
    QString mAppDir;

    //! Region mode select box
    QComboBox * mRegionModeComboBox;

    //! Direct mode
    bool mDirect;

    //! Error messages
    QStringList mErrors;
};

/** \class QgsGrassModuleStandardOptions
 *  \brief Widget with GRASS standard options.
 *
 */
class QgsGrassModuleStandardOptions: public QWidget, public QgsGrassModuleOptions
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassModuleStandardOptions(
      QgsGrassTools *tools, QgsGrassModule *module,
      QgisInterface *iface,
      QString xname, QDomElement docElem,
      bool direct, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    //! Destructor
    ~QgsGrassModuleStandardOptions();

    //! Get module options as list of arguments for QProcess
    QStringList arguments() override;

    // ! Get item by ID
    QgsGrassModuleItem *item( QString id );

    // ! Get item by key
    QgsGrassModuleItem *itemByKey( QString key );

    // Reimplemented methods from QgsGrassModuleOptions
    QStringList checkOutput() override;
    void freezeOutput() override;
    void thawOutput() override;
    QStringList ready() override;
    QStringList output( int type ) override;
    bool hasOutput( int type ) override;
    QStringList checkRegion() override;
    bool usesRegion() override;
    bool requestsRegion() override;
    bool inputRegion( struct Cell_head *window, QgsCoordinateReferenceSystem & crs, bool all ) override;
    QStringList flagNames() override { return mFlagNames; }

  public slots:
    // ! Show/hide advanced options
    void switchAdvanced();

  private:
    //! Name of module executable
    QString mXName;

    //! Path to module executable
    QString mXPath;

    //! Option items
    QList<QgsGrassModuleItem*> mItems;

    //! List of all flags. Necessary for scripts.
    QStringList mFlagNames;

    // ! Advanced options switch button
    QPushButton mAdvancedPushButton;

    // ! Advanced options frame
    QFrame mAdvancedFrame;
};

/****************** QgsGrassModuleGroupBoxItem ************************/

/** \class QgsGrassModuleCheckBox
 *  \brief Checkbox with elided text
 */
class QgsGrassModuleCheckBox : public QCheckBox
{
    Q_OBJECT

  public:
    /** \brief Constructor
     */
    QgsGrassModuleCheckBox( const QString & text, QWidget * parent = 0 );

    //! Destructor
    virtual ~QgsGrassModuleCheckBox();

    void resizeEvent( QResizeEvent * event ) override;

  public slots:
    void setText( const QString & text );

    void setToolTip( const QString & text );

    //! Adjust title size, called on resize
    void adjustText();

  private:
    QString mText;

    QString mTip;
};

/** \class QgsGrassModuleItem
 *  \brief GRASS module option
 */
class QgsGrassModuleItem
{
  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     * \param gnode option node in GRASS module XML description file
     */
    QgsGrassModuleItem( QgsGrassModule *module, QString key,
                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct );

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

    QStringList errors() { return mErrors; }

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

    bool mDirect;

    //! Error messages
    QStringList mErrors;
};

/****************** QgsGrassModuleGroupBoxItem ************************/

/** \class QgsGrassModuleGroupBoxItem
 *  \brief GRASS module option box
 */
class QgsGrassModuleGroupBoxItem : public QGroupBox, public QgsGrassModuleItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     * \param gnode option node in GRASS module XML description file
     */
    QgsGrassModuleGroupBoxItem( QgsGrassModule *module, QString key,
                                QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                                bool direct, QWidget * parent = 0 );

    //! Destructor
    virtual ~QgsGrassModuleGroupBoxItem();

    void resizeEvent( QResizeEvent * event ) override;

  public slots:
    //! Adjust title size, called on resize
    void adjustTitle();

};

/****************** QgsGrassModuleOption ************************/

/** \class QgsGrassModuleOption
 *  \brief  GRASS option
 */
class QgsGrassModuleOption : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleOption( QgsGrassModule *module, QString key,
                          QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                          bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleOption();

    //! Control option
    enum ControlType { NoControl, LineEdit, ComboBox, SpinBox, CheckBoxes };

    //! Control option
    enum ValueType { Double, Integer, String };

    //! Output type
    enum OutputType { None, Vector, Raster };

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

    //! True if this option is output
    bool isOutput() { return mIsOutput; }

    //! Returns output type
    int outputType() { return mOutputType; }

    //! If output, check if current output exists
    // Returns emppty string or name of existing output
    QString outputExists();

    QString ready() override;

    //! Current value
    QString value();

    //! Does this options causes use of region?
    //  Raster input/output uses region by default
    //  Use of region can be forced by 'region' attribute in qgm
    bool usesRegion() { return mUsesRegion; }

    //! Check min/max version and set error if cannot parse
    static bool checkVersion( const QString& version_min, const QString& version_max, QStringList& errors );

  public slots:
    // Add new line edit for multiple options
    void addLineEdit();

    // Remove one line edit for multiple options
    void removeLineEdit();

    // Browse output
    void browse( bool checked );

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
    QList<QString> mValues;

    //! Check boxes
    QList<QgsGrassModuleCheckBox*> mCheckBoxes;

    //! Line
    QList<QLineEdit*> mLineEdits;

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
/** \class QgsGrassModuleFlag
 *  \brief  GRASS flag
 */
class QgsGrassModuleFlag : public QgsGrassModuleCheckBox, public QgsGrassModuleItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleFlag( QgsGrassModule *module, QString key,
                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                        bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleFlag();

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

};

/************************ QgsGrassModuleInput **********************/

/** \class QgsGrassModuleInput
 *  \brief Class representing raster or vector module input
 */
class QgsGrassModuleInput : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleInput( QgsGrassModule *module,
                         QgsGrassModuleStandardOptions *options, QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleInput();

    enum Type { Vector, Raster };

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

    // ! Return vector of attribute fields of current vector
    QgsFields currentFields();

    //! Returns pointer to currently selected layer or null
    QgsMapLayer *currentLayer();

    QString currentMap();

    QString ready() override;

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
    // emitted when value changed/selected
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

    //! Vector of map@mapsestd::vectort in the combobox
    QStringList mMaps;

    //! Type of vector in the combobox
    QStringList mGeometryTypes;

    //! Layer names in the combobox
    QStringList mVectorLayerNames;

    //! Pointers to vector layers in combobox
    QList<QgsMapLayer*> mMapLayers;

    //! Vector of band numbers in combobox for rasters in direct mode
    QList<int> mBands;

    //! Attribute fields of layers in the combobox
    QList< QgsFields > mVectorFields;

    //! The imput map will be updated -> must be from current mapset
    bool mUpdate;

    //! Uses region
    bool mUsesRegion;

    //! Required field
    bool mRequired;
};

/*********************** QgsGrassModuleGdalInput **********************/

/** \class QgsGrassModuleGdalInput
 *  \brief GDAL/OGR module input
 */
class QgsGrassModuleGdalInput : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleGdalInput( QgsGrassModule *module, int type, QString key,
                             QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                             bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleGdalInput();

    enum Type { Gdal, Ogr };

    //! Reimplemented
    QStringList options() override;
    QString ready() override;

  public slots:
    //! Fill combobox with currently available maps in QGIS canvas
    void updateQgisLayers();

    void changed( int );

  private:
    //! Input type
    int mType;

    //! Ogr layer option associated with this input
    QString mOgrLayerOption;

    //! Ogr sql option associated with this input
    QString mOgrWhereOption;

    //! Combobox for QGIS layers
    QComboBox *mLayerComboBox;

    //! Lineedit for postgres password
    QLineEdit *mLayerPassword;

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

/** \class QgsGrassModuleField
 *  \brief GRASS vector attribute column.
 */
class QgsGrassModuleField : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleField( QgsGrassModule *module,
                         QgsGrassModuleStandardOptions *options,
                         QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleField();

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

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

/** \class QgsGrassModuleSelection
 *  \brief List of categories taken from current layer selection.
 */
class QgsGrassModuleSelection : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleSelection( QgsGrassModule *module,
                             QgsGrassModuleStandardOptions *options,
                             QString key,
                             QDomElement &qdesc, QDomElement &gdesc,
                             QDomNode &gnode,
                             bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleSelection();

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

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

/** \class QgsGrassModuleFile
 *  \brief Input/output file.
 */
class QgsGrassModuleFile : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleFile( QgsGrassModule *module,
                        QString key,
                        QDomElement &qdesc, QDomElement &gdesc,
                        QDomNode &gnode,
                        bool direct, QWidget *parent = 0 );

    //! Destructor
    ~QgsGrassModuleFile();

    //! File type
    enum Type { Old, New, Multiple, Directory };

    // Reimplemented methods from QgsGrassModuleOptions
    QStringList options() override;
    QString ready() override;

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

#endif // QGSGRASSMODULEOPTIONS_H
