
#ifndef QGSSTYLEV2_H
#define QGSSTYLEV2_H

#include <QMap>
#include <QString>

#include "qgssymbollayerv2utils.h" // QgsStringMap

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorColorRampV2;

class QDomDocument;
class QDomElement;

typedef QMap<QString, QgsVectorColorRampV2* > QgsVectorColorRampV2Map;

class CORE_EXPORT QgsStyleV2
{
  public:

    QgsStyleV2();
    ~QgsStyleV2();

    //! return default application-wide style
    static QgsStyleV2* defaultStyle();

    //! remove all contents of the style
    void clear();

    //! add symbol to style. takes symbol's ownership
    bool addSymbol( QString name, QgsSymbolV2* symbol );

    //! remove symbol from style (and delete it)
    bool removeSymbol( QString name );

    //! change symbol's name
    //! @note added in v1.7
    bool renameSymbol( QString oldName, QString newName );

    //! return a NEW copy of symbol
    QgsSymbolV2* symbol( QString name );

    //! return a const pointer to a symbol (doesn't create new instance)
    const QgsSymbolV2* symbolRef( QString name ) const;

    //! return count of symbols in style
    int symbolCount();

    //! return a list of names of symbols
    QStringList symbolNames();


    //! add color ramp to style. takes ramp's ownership
    bool addColorRamp( QString name, QgsVectorColorRampV2* colorRamp );

    //! remove color ramp from style (and delete it)
    bool removeColorRamp( QString name );

    //! change ramp's name
    //! @note added in v1.7
    bool renameColorRamp( QString oldName, QString newName );

    //! return a NEW copy of color ramp
    QgsVectorColorRampV2* colorRamp( QString name );

    //! return a const pointer to a symbol (doesn't create new instance)
    const QgsVectorColorRampV2* colorRampRef( QString name ) const;

    //! return count of color ramps
    int colorRampCount();

    //! return a list of names of color ramps
    QStringList colorRampNames();


    //! load a file into the style
    bool load( QString filename );

    //! save style into a file (will use current filename if empty string is passed)
    bool save( QString filename = QString() );

    //! return last error from load/save operation
    QString errorString() { return mErrorString; }

    //! return current file name of the style
    QString fileName() { return mFileName; }

  protected:

    QgsSymbolV2Map mSymbols;
    QgsVectorColorRampV2Map mColorRamps;

    QString mErrorString;
    QString mFileName;

    static QgsStyleV2* mDefaultStyle;
};


#endif
