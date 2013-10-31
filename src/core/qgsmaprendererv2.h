#ifndef QGSMAPRENDERERV2_H
#define QGSMAPRENDERERV2_H

#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

class QPainter;

class QgsScaleCalculator;
class QgsMapRendererJob;


#include "qgsscalecalculator.h"

struct QgsMapSettings
{
  // TODO

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

  // TODO: utility functions


protected:

  double mDpi;

  QSize mSize;

  QgsRectangle mExtent;

  QStringList mLayers;

  bool mProjectionsEnabled;
  QgsCoordinateReferenceSystem mDestCRS;

  // derived properties
  bool mValid; //!< whether the actual settings are valid (set in updateDerived())
  QgsRectangle mVisibleExtent; //!< extent with some additional white space that matches the output aspect ratio
  double mMapUnitsPerPixel;
  double mScale;


  // utiity stuff
  QgsScaleCalculator mScaleCalculator;
};


#endif // QGSMAPRENDERERV2_H
