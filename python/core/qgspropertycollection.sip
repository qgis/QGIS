typedef QMap< int, QgsPropertyDefinition > QgsPropertiesDefinition;

class QgsAbstractPropertyCollection
{
%TypeHeaderCode
#include <qgspropertycollection.h>
%End

%ConvertToSubClassCode
  if ( dynamic_cast<QgsPropertyCollection*>(sipCpp) )
	sipType = sipType_QgsPropertyCollection;
  else if ( dynamic_cast<QgsPropertyCollectionStack*>(sipCpp) )
	sipType = sipType_QgsPropertyCollectionStack;
  else
	sipType = sipType_QgsAbstractPropertyCollection;
%End

  public:

    QgsAbstractPropertyCollection( const QString& name = QString() );

    QString name() const;

    void setName( const QString& name );

    virtual QSet<int> propertyKeys() const = 0;

    virtual void clear() = 0;

    virtual bool hasProperty( int key ) const = 0;

    virtual QgsProperty property( int key ) const = 0;

    virtual QVariant value( int key, const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const = 0;

    QString valueAsString( int key, const QgsExpressionContext& context, const QString& defaultString = QString(), bool* ok /Out/ = nullptr ) const;

    QColor valueAsColor( int key, const QgsExpressionContext& context, const QColor& defaultColor = QColor(), bool* ok /Out/ = nullptr ) const;

    double valueAsDouble( int key, const QgsExpressionContext& context, double defaultValue = 0.0, bool* ok /Out/ = nullptr ) const;

    int valueAsInt( int key, const QgsExpressionContext& context, int defaultValue = 0, bool* ok /Out/ = nullptr ) const;

    bool valueAsBool( int key, const QgsExpressionContext& context, bool defaultValue = false, bool* ok /Out/ = nullptr ) const;

    virtual bool prepare( const QgsExpressionContext& context = QgsExpressionContext() ) const = 0;

    virtual QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const = 0;

    virtual bool isActive( int key ) const = 0;

    virtual bool hasActiveProperties() const = 0;

    virtual bool hasDynamicProperties() const = 0;

    virtual bool writeXml( QDomElement& collectionElem, QDomDocument& doc, const QgsPropertiesDefinition& definitions ) const = 0;

    virtual bool readXml( const QDomElement& collectionElem, const QDomDocument& doc, const QgsPropertiesDefinition &definitions ) = 0;

};


class QgsPropertyCollection : QgsAbstractPropertyCollection
{
%TypeHeaderCode
#include <qgspropertycollection.h>
%End

  public:

    QgsPropertyCollection( const QString& name = QString() );

    ~QgsPropertyCollection();

    QgsPropertyCollection( const QgsPropertyCollection& other );

    //QgsPropertyCollection& operator=( const QgsPropertyCollection& other );

    int count() const;

    QSet<int> propertyKeys() const;
    void clear();
    bool hasProperty( int key ) const;
    QgsProperty& property( int key );
    QVariant value( int key, const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;
    bool prepare( const QgsExpressionContext& context = QgsExpressionContext() ) const;
    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;
    bool isActive( int key ) const;
    bool hasActiveProperties() const;
    bool hasDynamicProperties() const;
    bool writeXml( QDomElement& collectionElem, QDomDocument& doc, const QgsPropertiesDefinition& definitions ) const;
    bool readXml( const QDomElement& collectionElem, const QDomDocument& doc, const QgsPropertiesDefinition& definitions );

    void setProperty( int key, const QgsProperty& property );

    void setProperty( int key, const QVariant& value );

};


class QgsPropertyCollectionStack : QgsAbstractPropertyCollection
{
%TypeHeaderCode
#include <qgspropertycollection.h>
%End

  public:

    QgsPropertyCollectionStack();

    ~QgsPropertyCollectionStack();

    QgsPropertyCollectionStack( const QgsPropertyCollectionStack& other );

    //QgsPropertyCollectionStack& operator=( const QgsPropertyCollectionStack& other );

    int count() const;
    void clear();
    void appendCollection( QgsPropertyCollection* collection /Transfer/ );
    QgsPropertyCollection* at( int index );
    //const QgsPropertyCollection* at( int index ) const;
    QgsPropertyCollection* collection( const QString& name );

    bool hasActiveProperties() const;

    bool hasDynamicProperties() const;

    bool isActive( int key ) const;

    QgsProperty property( int key ) const;

    QVariant value( int key, const QgsExpressionContext& context, const QVariant& defaultValue = QVariant() ) const;

    bool prepare( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    QSet< QString > referencedFields( const QgsExpressionContext& context = QgsExpressionContext() ) const;

    QSet<int> propertyKeys() const;
    bool hasProperty( int key ) const;
    bool writeXml( QDomElement& collectionElem, QDomDocument& doc, const QgsPropertiesDefinition& definitions ) const;
    bool readXml( const QDomElement& collectionElem, const QDomDocument& doc, const QgsPropertiesDefinition &definitions );
};

