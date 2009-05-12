
#ifndef QGSSYMBOLLAYERV2REGISTRY_H
#define QGSSYMBOLLAYERV2REGISTRY_H

#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"

typedef QgsSymbolLayerV2* (*QgsSymbolLayerV2CreateFunc)(const QgsStringMap& );
typedef QgsSymbolLayerV2Widget* (*QgsSymbolLayerV2WidgetFunc)();

/**
 Stores metadata about one symbol layer class.
 */
class QgsSymbolLayerV2Metadata
{
public:
	/** construct invalid metadata */
	QgsSymbolLayerV2Metadata()
	 : mName(), mCreateFunc(NULL), mWidgetFunc(NULL) {}
	
	/** construct metadata */
	QgsSymbolLayerV2Metadata(QString name, QgsSymbolV2::SymbolType type,
                           QgsSymbolLayerV2CreateFunc pfCreate,
                           QgsSymbolLayerV2WidgetFunc pfWidget = NULL)
	 : mName(name), mType(type), mCreateFunc(pfCreate), mWidgetFunc(pfWidget) {}
	
	QString name() const { return mName; }
	QgsSymbolV2::SymbolType type() const { return mType; }
	QgsSymbolLayerV2CreateFunc createFunction() const { return mCreateFunc; }
	QgsSymbolLayerV2WidgetFunc widgetFunction() const { return mWidgetFunc; }
  
  void setWidgetFunction(QgsSymbolLayerV2WidgetFunc f) { mWidgetFunc = f; }
	
protected:
	QString mName;
	QgsSymbolV2::SymbolType mType;
	QgsSymbolLayerV2CreateFunc mCreateFunc;
	QgsSymbolLayerV2WidgetFunc mWidgetFunc;
};

/**
 Registry of available symbol layer classes.
 Implemented as a singleton.
 */
class QgsSymbolLayerV2Registry
{
public:
	
  //! return the single instance of this class (instantiate it if not exists)
	static QgsSymbolLayerV2Registry* instance();
	
  //! return metadata for specified symbol layer
	QgsSymbolLayerV2Metadata symbolLayerMetadata(QString name) const;
	
  //! register a new symbol layer type
	void addSymbolLayerType(const QgsSymbolLayerV2Metadata& metadata);
  
  //! set layer type's widget function 
  bool setLayerTypeWidgetFunction(QString name, QgsSymbolLayerV2WidgetFunc f);
	
  //! create a new instance of symbol layer given symbol layer name and properties
	QgsSymbolLayerV2* createSymbolLayer(QString name, const QgsStringMap& properties = QgsStringMap()) const;
  
  //! return a list of available symbol layers for a specified symbol type
  QStringList symbolLayersForType(QgsSymbolV2::SymbolType type);
	
  //! create a new instance of symbol layer for specified symbol type with default settings
	static QgsSymbolLayerV2* defaultSymbolLayer(QgsSymbolV2::SymbolType type);
	
protected:
	QgsSymbolLayerV2Registry();
	
	static QgsSymbolLayerV2Registry* mInstance;
	QMap<QString, QgsSymbolLayerV2Metadata> mMetadata;
  
};

#endif
