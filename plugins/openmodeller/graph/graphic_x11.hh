/**
 * Declarations of GXDrawble, GXPixmap, GXCanvas, GXButton, TBtUnico,
 * TBtUAperta, TBtMult, TBtMExclu, TBtMAperta and GXFrame classes.
 *
 * This is an interface to held different graphic libraries.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-10-25
 * $Id$
 *
 * LICENSE INFORMATION
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _GRAPHIC_X11HH_
#define _GRAPHIC_X11HH_

#include "graph/graphic.hh"
#include "graph/color.hh"

#include <X11/Xlib.h>
#include <list.hh>

class GXFrame;


/**********************************************************************/
/****************************** GXDrawble *****************************/

/*****************************/
class GXDrawble : public GImage
{
public:
  GXDrawble( GXFrame &xframe, int dimx, int dimy );
  virtual ~GXDrawble() {}

  int foreground( GColor c );
  int background( GColor c );

  void clear();

  // Inverte o modo de desenho (Copy x Inverse).
  void copyMode();
  void invMode();

  void text( int x, int y, char *text, GColor c );

  // Draw geometric forms.
  //
  void pixel( int x, int y );
  void line( int x0, int y0, int x1, int y1 );
  void lines( int *pnt, int npnt );
  void polyg( int *pnt, int npnt );
  void fPolyg( int *pnt, int npnt );
  void rect( int x, int y, int w, int h );
  void fRect( int x, int y, int w, int h );
  void circle( int x, int y, int raio );
  void fCircle( int x, int y, int raio );
  void elipse( int x, int y, int rx, int ry );
  void fElipse( int x, int y, int rx, int ry );

  // Draw geometric forms passing temporary color.
  //
  void pixel( int x, int y, GColor c );
  void line( int x0, int y0, int x1, int y1, GColor c );
  void lines( int *pnt, int npnt, GColor c );
  void polyg( int *pnt, int npnt, GColor c );
  void fPolyg( int *pnt, int npnt, GColor c );
  void rect( int x, int y, int w, int h, GColor c );
  void fRect( int x, int y, int w, int h, GColor c );
  void circle( int x, int y, int raio, GColor c );
  void fCircle( int x, int y, int raio, GColor c );
  void elipse( int x, int y, int rx, int ry, GColor c );
  void fElipse( int x, int y, int rx, int ry, GColor c );

  // Draw from GImage.
  void put( GImage *pixmap );
  void put( int x0, int y0, GImage *pixmap );
  void put( int x, int y, int x0, int y0, int dimx, int dimy, GImage *pixmap );


  //  Display *getDisplay()   { return( f_dpy ); }
  //  Pixmap   Draw()         { return( f_draw ); }

  
protected:

  void setDim( int dx, int dy )      { f_dimx = dx; f_dimy = dy; }


  Display *f_dpy;
  Pixmap   f_draw;
  GC       f_gc;
  Colormap f_cm;

  GColor f_fg;
  GColor f_bg;
};




/**********************************************************************/
/******************************* GXPixmap *****************************/

/*******************************/
class GXPixmap : public GXDrawble
{
public:
  GXPixmap( GXFrame &xframe, int dimx, int dimy );
  virtual ~GXPixmap();

  // Redimension this GXPixmap from a loaded pixmap.
  int load( char *nome );

  
private:

  Window   f_orig;   // Parent Window.
  Pixmap   f_pixmap;
  Display *f_dpy;
};

typedef GXPixmap *PGXPixmap;




/**********************************************************************/
/******************************* GXCanvas *****************************/

/*******************************/
class GXCanvas : public GXDrawble
{
  friend class GXFrame;

public:
  GXCanvas( GXFrame &xframe, int posx, int posy, int dimx, int dimy );
  virtual ~GXCanvas();

  void btPress( int x, int y, int button );
  void btRelease( int x, int y, int button );
  void btDrag( int x, int y, int button );

  // Do not redimension GXCanvas.
  int load( char *nome );

  Window  win;


private:

  GFrame::FuncButton bt_press[3];
  GFrame::FuncButton bt_release[3];
  GFrame::FuncButton bt_drag[3];
};
typedef GXCanvas *PGXCanvas;




/**********************************************************************/
/******************************* GXButton *****************************/

/************/
class GXButton
{
public:

  GXButton( GXFrame &xframe, int posx, int posy, int dimx );
  virtual ~GXButton();

  virtual void draw() = 0;
  virtual void set( int x, int y ) = 0;

  Window  win;


protected:

  // xpos and width in pixels
  void clear( int xpos, int width );  // Uses default background.
  void clear( int xpos, int width, GColor bg );


  GXFrame *f_frm;
  int      f_dimx, f_dimy;  // x dimension in pixels.

  static GColor f_bg;       // Default background
};
typedef GXButton *PGXButton;


/*****************************/
class TBtUnico : public GXButton
{
public:

  // Apenas um botao.
  TBtUnico( GXFrame &xframe, int posx, int posy, char *label,
	    GFrame::FuncExec func, GColor c );
  virtual ~TBtUnico();

  virtual void draw()                   { label(); }
  virtual void set( int x, int y );


protected:

  void label();             // Default background.
  void label( GColor c );


  GColor   f_cor;
  char     f_label[32];

  GFrame::FuncExec f_func;
};


/*********************************/
class TBtUAperta : public TBtUnico
{
public:

  // Just one button.
  TBtUAperta( GXFrame &xframe, int posx, int posy,
              char *label, GFrame::FuncExec func, GColor c );
  virtual ~TBtUAperta();

  void set( int x, int y );


private:

  int f_estado;
};


/*****************************/
class TBtMult : public GXButton
{
public:

  // Multiple buttons.
  TBtMult( GXFrame &frame, int posx, int posy, char *label,
	   GFrame::FuncNotif func, GColor *ces );
  virtual ~TBtMult();

  virtual void draw();


protected:

  void label( int x );
  void label( int x, GColor c );


  int     f_nbot;
  char    f_label[256];
  GColor *f_cores;

  GFrame::FuncNotif f_func;
};


/******************************/
class TBtMExclu : public TBtMult
{
public:

  // Multiple buttons.
  TBtMExclu( GXFrame &frame, int posx, int posy, char *label,
	     GFrame::FuncNotif func, GColor *cores );
  virtual ~TBtMExclu();

  virtual void draw();
  virtual void set( int x, int y );


protected:

  int f_escolha;
};


/********************************/
class TBtMAperta : public TBtMult
{
public:

  // Multiple buttons.
  TBtMAperta( GXFrame &frame, int posx, int posy, char *label,
	      GFrame::FuncNotif func, GColor *cores );
  virtual ~TBtMAperta();

  virtual void draw();
  virtual void set( int x, int y );


protected:

  int *f_escolha;
};





/**********************************************************************/
/****************************** GXFrame *******************************/

/** 
 * @see GFrame virtual class.
 *
 */

/****************************/
class GXFrame : public GFrame
{
public:
  GXFrame( char *title, int control_height, int dimx, int dimy );
  ~GXFrame();

  void funcAlways( FuncExec f )   { f_falways = f; }
  void funcInit  ( FuncExec f )   { f_finit   = f; }
  void funcShow  ( FuncExec f )   { f_fshow   = f; }

  void funcBtPress( GImage *canvas, FuncButton func, int button );
  void funcBtRelease( GImage *canvas, FuncButton func, int button );
  void funcBtDrag( GImage *canvas, FuncButton func, int button );

  void exec();
  void flush();

  GImage *newCanvas( int posx, int posy, int dimx, int dimy );
  GImage *newPixmap( GImage *canvas, int dimx, int dimy );

  /* Buttons */

  // Press/release
  void button( int x, int y, char *lab, GFrame::FuncExec f,
               GColor c=GColor(30) );

  // Press/stay
  void buttonA ( int x, int y, char *lab, GFrame::FuncExec f,
                 GColor c=GColor(30) );

  // Mult press/release
  void buttonMA( int x, int y, char *lab, GFrame::FuncNotif f,
                 GColor *c=0 );

  // Mult press/stay
  void buttonME( int x, int y, char *lab, GFrame::FuncNotif f,
                 GColor *c=0 );

  Display *dpy;
  int      scr;  // Screen number
  Window   win;
  Window   crt;  // Control window.
  GC       gc;   // Graphic context.


private:

  void setColormap();          // Test.
  void print( Colormap cm );   // Test.

  // Manage frame events.
  int distributeButton( Window win, XEvent &event );

  //  static void exit();

  int f_crt_height;
  int f_winx, f_winy;

  List<PGXCanvas>  f_lcnv;
  List<PGXPixmap>  f_lpix;
  List<PGXButton>  f_lbot;
  PGXButton        f_btsaida;

  FuncExec f_falways;
  FuncExec f_finit;
  FuncExec f_fshow;

  int f_fim;
};



#endif









