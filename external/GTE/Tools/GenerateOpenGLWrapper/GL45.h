// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.11

#pragma once

#if defined(__gl_h_) || defined(__GL_H__)
#error gl.h included before OpenGL4.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error glext.h included before OpenGL4.h
#endif
#define __gl_h_
#define __GL_H__
#define __glext_h_
#define __GLEXT_H_

#if defined(GTE_USE_MSWINDOWS)

// Define GLAPI for use by glcorearb.h. The GTGraphics library is static,
// so there is no need to define GLAPI to be __declspec(dllimport) or
// __declspec(dllexport).
#define GLAPI

// Microsoft Windows supports OpenGL 1.1 types and functions.  Prevent these
// from being defined in glcorearb.h.
#define GL_VERSION_1_0
#define GL_VERSION_1_1
extern "C"
{
// OpenGL 1.0
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
__declspec(dllimport) void __stdcall glCullFace(GLenum mode);
__declspec(dllimport) void __stdcall glFrontFace(GLenum mode);
__declspec(dllimport) void __stdcall glHint(GLenum target, GLenum mode);
__declspec(dllimport) void __stdcall glLineWidth(GLfloat width);
__declspec(dllimport) void __stdcall glPointSize(GLfloat size);
__declspec(dllimport) void __stdcall glPolygonMode(GLenum face, GLenum mode);
__declspec(dllimport) void __stdcall glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
__declspec(dllimport) void __stdcall glTexParameterf(GLenum target, GLenum pname, GLfloat param);
__declspec(dllimport) void __stdcall glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
__declspec(dllimport) void __stdcall glTexParameteri(GLenum target, GLenum pname, GLint param);
__declspec(dllimport) void __stdcall glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
__declspec(dllimport) void __stdcall glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
__declspec(dllimport) void __stdcall glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
__declspec(dllimport) void __stdcall glDrawBuffer(GLenum mode);
__declspec(dllimport) void __stdcall glClear(GLbitfield mask);
__declspec(dllimport) void __stdcall glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
__declspec(dllimport) void __stdcall glClearStencil(GLint s);
__declspec(dllimport) void __stdcall glClearDepth(GLdouble depth);
__declspec(dllimport) void __stdcall glStencilMask(GLuint mask);
__declspec(dllimport) void __stdcall glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
__declspec(dllimport) void __stdcall glDepthMask(GLboolean flag);
__declspec(dllimport) void __stdcall glDisable(GLenum cap);
__declspec(dllimport) void __stdcall glEnable(GLenum cap);
__declspec(dllimport) void __stdcall glFinish(void);
__declspec(dllimport) void __stdcall glFlush(void);
__declspec(dllimport) void __stdcall glBlendFunc(GLenum sfactor, GLenum dfactor);
__declspec(dllimport) void __stdcall glLogicOp(GLenum opcode);
__declspec(dllimport) void __stdcall glStencilFunc(GLenum func, GLint ref, GLuint mask);
__declspec(dllimport) void __stdcall glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
__declspec(dllimport) void __stdcall glDepthFunc(GLenum func);
__declspec(dllimport) void __stdcall glPixelStoref(GLenum pname, GLfloat param);
__declspec(dllimport) void __stdcall glPixelStorei(GLenum pname, GLint param);
__declspec(dllimport) void __stdcall glReadBuffer(GLenum mode);
__declspec(dllimport) void __stdcall glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
__declspec(dllimport) void __stdcall glGetBooleanv(GLenum pname, GLboolean *data);
__declspec(dllimport) void __stdcall glGetDoublev(GLenum pname, GLdouble *data);
__declspec(dllimport) GLenum __stdcall glGetError(void);
__declspec(dllimport) void __stdcall glGetFloatv(GLenum pname, GLfloat *data);
__declspec(dllimport) void __stdcall glGetIntegerv(GLenum pname, GLint *data);
__declspec(dllimport) const GLubyte * __stdcall glGetString(GLenum name);
__declspec(dllimport) void __stdcall glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
__declspec(dllimport) void __stdcall glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
__declspec(dllimport) void __stdcall glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
__declspec(dllimport) void __stdcall glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
__declspec(dllimport) void __stdcall glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
__declspec(dllimport) GLboolean __stdcall glIsEnabled(GLenum cap);
__declspec(dllimport) void __stdcall glDepthRange(GLdouble near, GLdouble far);
__declspec(dllimport) void __stdcall glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

// OpenGL 1.1
typedef float GLclampf;
typedef double GLclampd;
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_VIEWPORT                       0x0BA2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_DOUBLE                         0x140A
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F
#define GL_TEXTURE                        0x1702
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064
#define GL_REPEAT                         0x2901
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_VERTEX_ARRAY                   0x8074
__declspec(dllimport) void __stdcall glDrawArrays(GLenum mode, GLint first, GLsizei count);
__declspec(dllimport) void __stdcall glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
__declspec(dllimport) void __stdcall glGetPointerv(GLenum pname, void **params);
__declspec(dllimport) void __stdcall glPolygonOffset(GLfloat factor, GLfloat units);
__declspec(dllimport) void __stdcall glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
__declspec(dllimport) void __stdcall glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
__declspec(dllimport) void __stdcall glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
__declspec(dllimport) void __stdcall glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
__declspec(dllimport) void __stdcall glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
__declspec(dllimport) void __stdcall glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
__declspec(dllimport) void __stdcall glBindTexture(GLenum target, GLuint texture);
__declspec(dllimport) void __stdcall glDeleteTextures(GLsizei n, const GLuint *textures);
__declspec(dllimport) void __stdcall glGenTextures(GLsizei n, GLuint *textures);
__declspec(dllimport) GLboolean __stdcall glIsTexture(GLuint texture);
}

// Microsoft Windows OpenGL extensions (WGL).
extern "C"
{
extern int __stdcall wglSwapIntervalEXT(int interval);
extern int __stdcall wglGetSwapIntervalEXT(void);
}
extern void InitializeWGL();
#endif

// Use the prototypes provided by glcorearb.h.
#define GL_GLEXT_PROTOTYPES
#if defined(GTE_USE_MSWINDOWS) && !defined(NOMINMAX)
// Disable the Windows.h min and max macros.
#define NOMINMAX
#endif
#include <Graphics/GL45/GL/glcorearb.h>

// Call this function before using OpenGL in your code.  To see what your
// driver supports, pass in a file name to which the information will be
// stored.  For no information, pass in nullptr.
extern void InitializeOpenGL(int& major, int& minor, char const* infofile);
