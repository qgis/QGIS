/***************************************************************************
                         qgsgrassmoduleparam.h
                             -------------------
    begin                : August, 2015
    copyright            : (C) 2015 by Radim Blazek
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

#ifndef QGSGRASSMODULEPARAM_H
#define QGSGRASSMODULEPARAM_H

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "qgis.h"
#include "qgsfield.h"
#include "qgscoordinatereferencesystem.h"

class QDomNode;
class QDomElement;
class QValidator;

class QgsGrassModule;
class QgsGrassModuleStandardOptions;
class QgsGrassModuleInput;

class QgsMapLayer;
class QgsVectorLayer;

extern "C"
{
#include <grass/gis.h>
}
/****************** QgsGrassModuleCheckBox ************************/

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
class QgsGrassModuleParam
{
  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     * \param gnode option node in GRASS module XML description file
     */
    QgsGrassModuleParam( QgsGrassModule *module, QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct );

    //! Destructor
    virtual ~QgsGrassModuleParam();

    //! Is the item hidden
    bool hidden();

    //! Retruns list of options which will be passed to module
    virtual QStringList options();

    //! Item's key
    QString key() { return mKey; }

    //! Multiple values
    bool multiple() const { return mMultiple; }

    //! Item's id
    QString id() { return mId; }

    //! Check if otpion is ready
    //  Returns empty string or error message
    virtual QString ready() { return QString() ; }

    QStringList errors() { return mErrors; }

    /** Get gisprompt attribute
     * @paream name gisprompt tag attribute name (age, element, prompt)
     */
    static QString getDescPrompt( QDomElement descDomElement, const QString & name );

    //! Find element in GRASS module description by key, if not found, returned element is null
    static QDomNode nodeByKey( QDomElement descDocElement, QString key );

    /** Find list of elements in GRASS module description by option type.
     *  Option type is identified by gisprompt prompt. Only few types are supported */
    static QList<QDomNode> nodesByType( QDomElement descDomElement, STD_OPT optionType, const QString & age = QString() );

  protected:

    //! Pointer to GRASS module
    QgsGrassModule *mModule;

    //! Option key, for flags without '-'
    QString mKey;

    //! Multiple values
    bool mMultiple;

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
class QgsGrassModuleGroupBoxItem : public QGroupBox, public QgsGrassModuleParam
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

  protected:
    //! Line
    QList<QLineEdit*> mLineEdits;

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
class QgsGrassModuleFlag : public QgsGrassModuleCheckBox, public QgsGrassModuleParam
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

/*********************** QgsGrassModuleGdalInput **********************/

/** \class QgsGrassModuleGdalInput
 *  \brief GDAL/OGR module input
 */
class QgsGrassModuleGdalInput : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    enum Type { Gdal, Ogr };

    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleGdalInput( QgsGrassModule *module, QgsGrassModuleGdalInput::Type type, QString key,
                             QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                             bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleGdalInput();

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
 *  \brief GRASS column, not existing column of input vector, may be output column or input column from a table not linked to layer
 */
class QgsGrassModuleField : public QgsGrassModuleOption
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleField( QgsGrassModule *module, QString key,
                         QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                         bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleField();
};

/*********************** QgsGrassModuleVectorField **********************/

/** \class QgsGrassModuleVectorField
 *  \brief GRASS vector attribute column.
 */
class QgsGrassModuleVectorField : public QgsGrassModuleGroupBoxItem
{
    Q_OBJECT

  public:
    /** \brief Constructor
     * \param qdesc option element in QGIS module description XML file
     * \param gdesc GRASS module XML description file
     */
    QgsGrassModuleVectorField( QgsGrassModule *module,
                               QgsGrassModuleStandardOptions *options,
                               QString key,
                               QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                               bool direct, QWidget * parent = 0 );

    //! Destructor
    ~QgsGrassModuleVectorField();

    //! Retruns list of options which will be passed to module
    virtual QStringList options() override;

    void setLayerInput( QgsGrassModuleInput * layerInput ) { mLayerInput = layerInput; }
    QgsGrassModuleInput * layerInput() const { return mLayerInput; }

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

    //! Combobox for QGIS layer fieldsnviz
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

    //! Line
    QLineEdit *mLineEdit;

    //! Browse button
    QPushButton *mBrowseButton;

    //! File filters
    QString mFilters;
};

#endif // QGSGRASSMODULEPARAM_H
