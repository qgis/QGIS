class QgsPropertyOverrideButton : QToolButton
{
%TypeHeaderCode
#include <qgspropertyoverridebutton.h>
%End
  public:

    QgsPropertyOverrideButton( QWidget* parent /TransferThis/ = nullptr,
                            const QgsVectorLayer* layer = nullptr );

    void init( int propertyKey,
               const QgsProperty& property,
               const QgsPropertiesDefinition& definitions,
               const QgsVectorLayer* layer = nullptr );

    void init( int propertyKey,
               const QgsAbstractPropertyCollection& collection,
               const QgsPropertiesDefinition& definitions,
               const QgsVectorLayer* layer = nullptr );

    QgsProperty toProperty() const;

    void setToProperty( const QgsProperty& property );

    int propertyKey() const;

    bool isActive() const;

    QgsPropertyDefinition::DataType validDataType() const;

    QString fullDescription() const;

    QString usageInfo() const;

    void setUsageInfo( const QString& info );

    void setVectorLayer( const QgsVectorLayer* layer );

    const QgsVectorLayer* vectorLayer() const;

    void registerCheckedWidget( QWidget* widget );

    void registerExpressionContextGenerator( QgsExpressionContextGenerator* generator );

  public slots:

    void setActive( bool active );

  signals:

    void changed();

    void activated( bool isActive );

  protected:
    void mouseReleaseEvent( QMouseEvent *event );
};

