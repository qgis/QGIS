class QgsDartMeasurement
{
%TypeHeaderCode
#include <qgsdartmeasurement.h>
%End
  public:
    enum Type
    {
      Text,
      ImagePng,
      Integer
    };

    QgsDartMeasurement();

    QgsDartMeasurement( const QString& name, Type type, const QString& value );

    const QString toString() const;

    void send() const;

    static const QString typeToString( QgsDartMeasurement::Type type );
};
