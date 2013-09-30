#ifndef QGSMAPRENDERERV2_H
#define QGSMAPRENDERERV2_H

#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

class QPainter;

class QgsScaleCalculator;
class QgsMapRendererJob;




struct QgsMapRendererSettings
{
  // TODO

  double dpi;

  QSize size;

  QgsRectangle extent;

  QStringList layers;

  bool projectionsEnabled;
  QgsCoordinateReferenceSystem destCRS;

  // derived properties
  bool valid; //!< whether the actual settings are valid (set in updateDerived())
  QgsRectangle visibleExtent; //!< extent with some additional white space that matches the output aspect ratio
  double mapUnitsPerPixel;
  double scale;


  // TODO: utility functions


};


class QgsMapRendererV2 : public QObject
{
  Q_OBJECT
public:

  QgsMapRendererV2();
  ~QgsMapRendererV2();

  //
  // getters/setters for rendering settings
  //

  QgsRectangle extent() const;
  void setExtent(const QgsRectangle& rect);

  QSize outputSize() const;
  void setOutputSize(const QSize& size);

  double outputDpi() const;
  void setOutputDpi(double dpi);

  QStringList layers() const;
  void setLayers(const QStringList& layers);

  void updateDerived(); // TODO: should be protected, called automatically

  bool hasValidSettings() const;
  QgsRectangle visibleExtent() const;
  double mapUnitsPerPixel() const;
  double scale() const;

  //! Access all map renderer settings at once
  const QgsMapRendererSettings& settings() const { return mSettings; }

  //
  // rendering control
  //

  //! start rendering to a QImage with the current settings
  bool start(bool parallel = false);

  //! start rendering with a custom painter (
  bool startWithCustomPainter(QPainter* painter);

  //! cancel the rendering job and wait until it stops
  bool cancel();

  //! block until the rendering is done
  void waitForFinished();

  bool isRendering() const { return mActiveJob != 0; }

  const QgsMapRendererJob* activeJob() const { return mActiveJob; }

signals:
  void finished();

protected slots:
  void onJobFinished();

protected:
  void updateScale();
  void adjustExtentToSize();

protected:

  QgsMapRendererSettings mSettings;

  QgsScaleCalculator* mScaleCalculator;

  //! currently running renderer job (null if there is none)
  QgsMapRendererJob* mActiveJob;
};


#endif // QGSMAPRENDERERV2_H
