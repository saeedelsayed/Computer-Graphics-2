//
// "$Id: Fl_Gl_Choice.cxx 6875 2009-09-17 01:57:42Z spitzak $"
//
// OpenGL visual selection code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <config.h>
#if HAVE_GL

#ifndef _WIN32
	#undef USE_GLEW // make sure the GLX headers use function prototypes instead of pointers
#endif
#include "GlChoice.h"
#include <fltk/visual.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
using namespace fltk;

#define DEBUG_PFD (0) // 1 = PFD selection debug output, 0 = no debug output

#ifdef _WIN32
#if DEBUG_PFD

static void debug_print_pfd(int pfd_id, const PIXELFORMATDESCRIPTOR& pfd) {
    printf("pfd #%d:\n", pfd_id);
    printf("- Composition           : %s\n", (pfd.dwFlags & PFD_SUPPORT_COMPOSITION) ? "yes" : "no");
    printf("- Hardware acceleration : %s\n", (pfd.dwFlags & PFD_GENERIC_FORMAT) ? "no" : "yes");
    printf("- Double Buffering      : %s\n", (pfd.dwFlags & PFD_DOUBLEBUFFER) ? "yes" : "no");
    printf("- Overlay Planes        : %u overlay planes & %u underlay planes\n", (pfd.bReserved & 0x0F), (pfd.bReserved & 0xF0));
    printf("- Color Buffer Depth    : %2d (excluding alpha)\n", pfd.cColorBits);
    printf("- RGBA Bits             : %2d, %2d, %2d, %2d\n", pfd.cRedBits, pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits);
    printf("- Alpha Buffer Depth    : %2d\n", pfd.cAlphaBits);
    printf("- Accum Buffer Depth    : %2d\n", pfd.cAccumBits);
    printf("- Accum RGBA Bits       : %2d, %2d, %2d, %2d\n", pfd.cAccumRedBits, pfd.cAccumGreenBits, pfd.cAccumBlueBits, pfd.cAccumAlphaBits);
    printf("- Depth Buffer Depth    : %2d\n", pfd.cDepthBits);
    printf("- Stencil Buffer Depth  : %2d\n", pfd.cStencilBits);
    printf("- Buffer Swap Behavior  : %s\n", (pfd.dwFlags & PFD_SWAP_COPY) ? "copy" : ((pfd.dwFlags & PFD_SWAP_EXCHANGE) ? "exchange" : "unknown"));
    printf("- Aux Buffers Count     : %d\n", pfd.cAuxBuffers);
    printf("- Visible Mask          : %d\n", pfd.dwVisibleMask);
}

#endif // DEBUG_PFD
#endif // _WIN32

static GlChoice* first;

std::vector<int>& GlChoice::ref_gl_context_attrib_list(const Window* window)
{
    static std::vector<int> default_attrib_list;
	static std::map<const Window*, std::vector<int>> attrib_list_map;
    if (window == 0)
        return default_attrib_list;
    return attrib_list_map[window];
}

GlChoice* GlChoice::find(int mode) {
  GlChoice* g;
  
  for (g = first; g; g = g->next) if (g->mode == mode) return g;

#ifdef _WIN32

  // STR #3119: select pixel format with composition support
  // ... and no more than 32 color bits (8 bits/color)
  // Ref: PixelFormatDescriptor Object
  // https://msdn.microsoft.com/en-us/library/cc231189.aspx
#if !defined(PFD_SUPPORT_COMPOSITION)
# define PFD_SUPPORT_COMPOSITION (0x8000)
#endif

  // Replacement for ChoosePixelFormat() that finds one with an overlay
  // if possible:
  HDC dc = getDC();
  int pixelFormat = 0;
  PIXELFORMATDESCRIPTOR chosen_pfd;
  for(int i = 1; ; i++) {
      PIXELFORMATDESCRIPTOR pfd;
      if(!DescribePixelFormat(dc, i, sizeof(pfd), &pfd)) break;
      // continue if it does not satisfy our requirements:
      if(~pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL)) continue;
      if(pfd.iPixelType != ((mode & INDEXED_COLOR) ? PFD_TYPE_COLORINDEX : PFD_TYPE_RGBA)) continue;
      if((mode & ALPHA_BUFFER) && !pfd.cAlphaBits) continue;
      if((mode & ACCUM_BUFFER) && !pfd.cAccumBits) continue;
      if((!(mode & DOUBLE_BUFFER)) != (!(pfd.dwFlags & PFD_DOUBLEBUFFER))) continue;
      if((!(mode & STEREO)) != (!(pfd.dwFlags & PFD_STEREO))) continue;
      if((mode & DEPTH_BUFFER) && !pfd.cDepthBits) continue;
      if((mode & STENCIL_BUFFER) && !pfd.cStencilBits) continue;

#if DEBUG_PFD
      debug_print_pfd(i, pfd);
#endif // DEBUG_PFD

      // see if better than the one we have already:
      if(pixelFormat) {
          // offering non-generic rendering is better (read: hardware acceleration)
          if(!(chosen_pfd.dwFlags & PFD_GENERIC_FORMAT) &&
              (pfd.dwFlags & PFD_GENERIC_FORMAT))
              continue;
          // offering overlay is better:
          else if(!(chosen_pfd.bReserved & 15) && (pfd.bReserved & 15)) {}
          // otherwise prefer a format that supports composition (STR #3119)
          else if((chosen_pfd.dwFlags & PFD_SUPPORT_COMPOSITION) &&
              !(pfd.dwFlags & PFD_SUPPORT_COMPOSITION)) continue;
          // otherwise more bit planes is better, but no more than 32 (8 bits per channel):
          else if(pfd.cColorBits > 32 || chosen_pfd.cColorBits > pfd.cColorBits) continue;
          else if(chosen_pfd.cDepthBits > pfd.cDepthBits) continue;
      }
      pixelFormat = i;
      chosen_pfd = pfd;
  }

#if DEBUG_PFD
  static int bb = 0;
  if(!bb) {
      bb = 1;
      printf("PFD_SUPPORT_COMPOSITION = 0x%x\n", PFD_SUPPORT_COMPOSITION);
  }
  printf("\nChosen pixel format is %d\n", pixelFormat);
  debug_print_pfd(pixelFormat, chosen_pfd);
#endif // DEBUG_PFD

  if(!pixelFormat) return 0;

#elif defined(__APPLE__)

  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  const int *blist;
  int list[32];
   
  int n = 0;
  if (mode & INDEXED_COLOR) {
    list[n++] = AGL_BUFFER_SIZE;
    list[n++] = 8; // glut tries many sizes, but this should work...
  } else {
    list[n++] = AGL_RGBA;
    list[n++] = AGL_GREEN_SIZE;
    list[n++] = (mode & RGB24_COLOR) ? 8 : 1;
    if (mode & ALPHA_BUFFER) {
      list[n++] = AGL_ALPHA_SIZE;
      list[n++] = (mode & RGB24_COLOR) ? 8 : 1;
    }
    if (mode & ACCUM_BUFFER) {
      list[n++] = AGL_ACCUM_GREEN_SIZE;
      list[n++] = 1;
      if (mode & ALPHA_BUFFER) {
        list[n++] = AGL_ACCUM_ALPHA_SIZE;
        list[n++] = 1;
      }
    }
  }
  if (mode & DOUBLE_BUFFER) {
    list[n++] = AGL_DOUBLEBUFFER;
  }
  if (mode & DEPTH_BUFFER) {
    list[n++] = AGL_DEPTH_SIZE; list[n++] = 24;
  }
  if (mode & STENCIL_BUFFER) {
    list[n++] = AGL_STENCIL_SIZE; list[n++] = 1;
  }
# ifdef AGL_STEREO
  if (mode & STEREO) {
    list[n++] = AGL_STEREO;
  }
# endif
  list[n] = AGL_NONE;
  blist = list;

  open_display();
  AGLPixelFormat fmt = aglChoosePixelFormat(NULL, 0, (GLint*)blist);
  if (!fmt) return 0;

#else

  int list[32];
  int n = 0;
  if (mode & INDEXED_COLOR) {
    list[n++] = GLX_BUFFER_SIZE;
    list[n++] = 8; // glut tries many sizes, but this should work...
  } else {
    list[n++] = GLX_RGBA;
    list[n++] = GLX_GREEN_SIZE;
    const int bits = (mode & RGB24_COLOR) ? 8 : 1;
    list[n++] = bits;
    if (mode & ALPHA_BUFFER) {
      list[n++] = GLX_ALPHA_SIZE;
      list[n++] = bits;
    }
    if (mode & ACCUM_BUFFER) {
      list[n++] = GLX_ACCUM_GREEN_SIZE;
      list[n++] = bits;
      if (mode & ALPHA_BUFFER) {
	list[n++] = GLX_ACCUM_ALPHA_SIZE;
	list[n++] = bits;
      }
    }
  }
  if (mode & DOUBLE_BUFFER) {
    list[n++] = GLX_DOUBLEBUFFER;
  }
  if (mode & DEPTH_BUFFER) {
    list[n++] = GLX_DEPTH_SIZE; list[n++] = 1;
  }
  if (mode & STENCIL_BUFFER) {
    list[n++] = GLX_STENCIL_SIZE; list[n++] = 1;
  }
  if (mode & STEREO) {
    list[n++] = GLX_STEREO;
  }
#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
  if (mode & MULTISAMPLE) {
    list[n++] = GLX_SAMPLES_SGIS;
    list[n++] = 4; // value Glut uses
  }
#endif
  list[n] = 0;
    
  open_display();
#if 0 // force it to use a specific visual number, for testing
  XVisualInfo templt;
  templt.visualid = 0x34; // use glxinfo to list these numbers
  int num;
  XVisualInfo *vis=XGetVisualInfo(xdisplay, VisualIDMask, &templt, &num);
#else
  XVisualInfo* vis = glXChooseVisual(xdisplay, xscreen, list);
#endif
  if (!vis) {
# if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
    if (mode&MULTISAMPLE) return find(mode&~MULTISAMPLE);
# endif
    return 0;
  }

#endif

  g = new GlChoice;
  g->mode = mode;
  g->next = first;
  first = g;

#ifdef _WIN32
  g->pixelFormat = pixelFormat;
  g->pfd = chosen_pfd;
#elif defined(__APPLE__)
  g->pixelformat = fmt;
#else
  g->vis = vis;

  if (/*MaxCmapsOfScreen(ScreenOfDisplay(xdisplay,xscreen))==1 && */
      vis->visualid == xvisual->visualid &&
      !getenv("MESA_PRIVATE_CMAP"))
    g->colormap = xcolormap;
  else
    g->colormap = XCreateColormap(xdisplay, RootWindow(xdisplay,xscreen),
				  vis->visual, AllocNone);
#endif

  return g;
}

static GLContext first_context;

#if USE_X11

GLXFBConfig findFBConfigFromVisual(Display* display, XVisualInfo* visualInfo) 
{
    if (!display || !visualInfo) {
        fprintf(stderr, "Invalid display or visualInfo\n");
        return NULL;
    }

    int fbCount;
    GLXFBConfig* fbConfigs = glXGetFBConfigs(display, DefaultScreen(display), &fbCount);
    if (!fbConfigs || fbCount == 0) {
        fprintf(stderr, "Failed to get framebuffer configurations\n");
        return NULL;
    }

    GLXFBConfig matchingFBConfig = NULL;
    for (int i = 0; i < fbCount; i++) {
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbConfigs[i]);
        if (vi) {
            if (vi->visualid == visualInfo->visualid) {
                matchingFBConfig = fbConfigs[i];
                XFree(vi);
                break;
            }
            XFree(vi);
        }
    }
    XFree(fbConfigs);
    return matchingFBConfig;
}

// Define this to destroy all OpenGL contexts at exit to try to fix NVidia crashes
#define DESTROY_ON_EXIT 0

#if DESTROY_ON_EXIT
static struct Contexts {
  GLContext context;
  struct Contexts* next;
} * context_list;

static void destructor() {
  if (xdisplay && first_context) {
    first_context = 0;
    for (Contexts* p = context_list; p; p = p->next) {
      glXDestroyContext(xdisplay, p->context);
    }
    context_list = 0;
    XFlush(xdisplay);
  }
}
#endif

GLContext fltk::create_gl_context(const Window* window, XVisualInfo* vis) {
  GLContext context;
#if 1 // enable OpenGL3 support if possible
  // This is disabled because it does not work on SUSE11 with NVidia cards.
  // I tried all the visuals and none worked. Error is returned when attempts
  // are made to use the context:
  // XRequest.144: BadAlloc (insufficient resources for operation) 0x1e00006
  // XRequest.144: GLXBadContext 0x1e00006
  // XRequest.144: GLXBadDrawable 0x1e00004
  //typedef GLXFBConfig (*PFNGLXGETFBCONFIGFROMVISUALSGIXPROC)(
	//	Display *dpy,
	//	XVisualInfo *vis );
  typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARB)(
		Display *dpy,
		GLXFBConfig config,
		GLXContext share_context,
		Bool direct,
		const int *attrib_list);

  //PFNGLXGETFBCONFIGFROMVISUALSGIXPROC glXGetFBConfigFromVisualSGIX = (PFNGLXGETFBCONFIGFROMVISUALSGIXPROC)
	//	glXGetProcAddress((const GLubyte *) "glXGetFBConfigFromVisualSGIX");

  PFNGLXCREATECONTEXTATTRIBSARB glXCreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARB)  glXGetProcAddress((const GLubyte *) "glXCreateContextAttribsARB");

  if (!GlChoice::ref_gl_context_attrib_list(window).empty() && glXCreateContextAttribsARB) {
    //printf("A\n");
    // printf("Success!\n");
    GLXFBConfig c = findFBConfigFromVisual(xdisplay, vis);
//    printf("B\n");
//#   define GLX_CONTEXT_MAJOR_VERSION_ARB		0x2091
//#   define GLX_CONTEXT_MINOR_VERSION_ARB		0x2092
//    int arr2[] = {  GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
//                    GLX_CONTEXT_MINOR_VERSION_ARB, 0, None };
    context = glXCreateContextAttribsARB(xdisplay, c, first_context, True, GlChoice::ref_gl_context_attrib_list(window).data());
//    printf("C\n");
  } else
#endif
      context = glXCreateContext(xdisplay, vis, first_context, 1);
#if DESTROY_ON_EXIT
  Contexts* p = new Contexts;
  p->context = context;
  p->next = context_list;
  context_list = p;
#endif
  if (!first_context) {
    first_context = context;
#if DESTROY_ON_EXIT
    atexit(::destructor);
#endif
  }
  return context;
}

#elif defined(_WIN32)
GLContext fltk::create_gl_context(const Window* window, const GlChoice* g, int layer) {
  CreatedWindow* i = CreatedWindow::find(window);
  SetPixelFormat(i->dc, g->pixelFormat, &g->pfd);

  GLContext context = 0;
  if (!GlChoice::ref_gl_context_attrib_list(window).empty()) {
	  HGLRC tempRC = wglCreateContext(i->dc);
	  wglMakeCurrent(i->dc, tempRC);
	  GLenum err = glewInit();
	  if (err != GLEW_OK) {
		  std::cerr << "GLEW init error: " << glewGetErrorString(err) << std::endl;
	  }
	  else {
		// attempt to choose another pixel format
		  
		//// try to choose better pixel format
		//std::vector<int> attribList;
		//attribList.push_back(WGL_DRAW_TO_WINDOW_ARB); attribList.push_back(GL_TRUE);
		//attribList.push_back(WGL_SUPPORT_OPENGL_ARB); attribList.push_back(GL_TRUE);
		//attribList.push_back(WGL_DOUBLE_BUFFER_ARB);  attribList.push_back((g->get_mode() & DOUBLE_BUFFER) != 0 ? GL_TRUE : GL_FALSE);
		//attribList.push_back(WGL_STEREO_ARB);         attribList.push_back((g->get_mode() & STEREO) != 0 ? GL_TRUE : GL_FALSE);
		//attribList.push_back(WGL_DEPTH_BITS_ARB);     attribList.push_back((g->get_mode() & DEPTH_BUFFER) != 0 ? 24 : 0);
		//attribList.push_back(WGL_STENCIL_BITS_ARB);   attribList.push_back((g->get_mode() & STENCIL_BUFFER) != 0 ? 8 : 0);
		//attribList.push_back(WGL_ACCUM_BITS_ARB);     attribList.push_back((g->get_mode() & ACCUM_BUFFER) != 0 ? 32 : 0);
		//attribList.push_back(WGL_ACCUM_BITS_ARB);     attribList.push_back(0);
		//if ((g->get_mode() & INDEXED_COLOR) != 0) {
		//	attribList.push_back(WGL_PIXEL_TYPE_ARB);     attribList.push_back(WGL_TYPE_COLORINDEX_ARB);
		//	attribList.push_back(WGL_COLOR_BITS_ARB);     attribList.push_back(8);
		//}
		//else if ((g->get_mode() & ALPHA_BUFFER) != 0) {
		//	attribList.push_back(WGL_PIXEL_TYPE_ARB);     attribList.push_back(WGL_TYPE_RGBA_ARB);
		//	attribList.push_back(WGL_COLOR_BITS_ARB);     attribList.push_back(64);
		//	attribList.push_back(WGL_ALPHA_BITS_ARB);     attribList.push_back(8);
		//}
		//else {
		//	attribList.push_back(WGL_COLOR_BITS_ARB);     attribList.push_back((g->get_mode() & RGB24_COLOR) != 0 ? 24 : 32);
		//}
		//if (g->get_mode() & MULTISAMPLE) {
		//	attribList.push_back(WGL_SAMPLE_BUFFERS_ARB); attribList.push_back(1);
		//	attribList.push_back(WGL_SAMPLES_ARB); attribList.push_back(4);
		//}
		//attribList.push_back(0);
		//int formats[200];
		//UINT nr_formats;
		//if (wglChoosePixelFormatARB(i->dc, &attribList[0], 0, 200, formats, &nr_formats)) {
		//	if (nr_formats > 0) {
		//		PIXELFORMATDESCRIPTOR pfd;
		//		int format = formats[0];
		//		//format = g->pixelFormat;
		//		DescribePixelFormat(i->dc, format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		//		if (SetPixelFormat(i->dc, format, &pfd) == TRUE) {
		//			std::cout << "FL_Gl_Choice::successfully chose pixel format " << format << std::endl;
		//			const_cast<GlChoice*>(g)->pixelFormat = format;
		//			const_cast<GlChoice*>(g)->pfd = pfd;
		//		}
		//		else {
		//			LPVOID lpMsgBuf;
		//			DWORD dw = GetLastError();

		//			FormatMessage(
		//				FORMAT_MESSAGE_ALLOCATE_BUFFER |
		//				FORMAT_MESSAGE_FROM_SYSTEM |
		//				FORMAT_MESSAGE_IGNORE_INSERTS,
		//				NULL,
		//				dw,
		//				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		//				(LPTSTR)&lpMsgBuf,
		//				0, NULL);
		//			std::cerr << (LPCTSTR)lpMsgBuf << std::endl;
		//			LocalFree(lpMsgBuf);
		//			std::cerr << "FL_Gl_Choice::could not set pixel format to " << format << " (keep " << g->pixelFormat << ")" << std::endl;
		//		}
		//	}
		//	else
		//		std::cerr << "FL_Gl_Choice::no pixel formats found" << std::endl;
		//}
		//else
		//	std::cerr << "FL_Gl_Choice::wglChoosePixelFormatARB failed" << std::endl;

		  bool is_first_context = first_context == 0;
          auto& attrib_list = GlChoice::ref_gl_context_attrib_list(window);
          if (layer != 0) {
              attrib_list.back() = WGL_CONTEXT_LAYER_PLANE_ARB;
              attrib_list.push_back(layer);
              attrib_list.push_back(0);
			  context = wglCreateContextAttribsARB(i->dc, first_context, attrib_list.data());
		  }
		  else
			  context = wglCreateContextAttribsARB(i->dc, first_context, attrib_list.data());
		  if (!context) {
			  int err = glGetError();
			  std::cerr << "WGL error: " << glewGetErrorString(err) << std::endl;
			  context = tempRC;
		  }
		  else {
			  wglMakeCurrent(NULL, NULL);
			  wglDeleteContext(tempRC);
		  }
		  if (is_first_context)
			  first_context = context;
	  }
  }
  else {
	  context = layer ? wglCreateLayerContext(i->dc, layer) : wglCreateContext(i->dc);
	  if (context) {
		  if (first_context) 
			  wglShareLists(first_context, context);
		  else 
			  first_context = context;
	  }
  }

  return context;
}

#elif defined(__APPLE__)

// warning: the Quartz version should probably use Core GL (CGL) instead of AGL
GLContext fltk::create_gl_context(const Window* window, const GlChoice* g, int layer) {
  GLContext context;
  context = aglCreateContext(g->pixelformat, first_context);
  if (!context) return 0;
  if (!first_context) first_context = context;
  return context;
}

#endif

GLContext fl_current_glcontext;
static const Window* cached_window;

void fltk::set_gl_context(const Window* window, GLContext context) {
  if (context != fl_current_glcontext || window != cached_window) {
    fl_current_glcontext = context;
    cached_window = window;
#if USE_X11
    if (first_context) glXMakeCurrent(xdisplay, xid(window), context);
#elif defined(_WIN32)
    wglMakeCurrent(CreatedWindow::find(window)->dc, context);
#elif defined(__APPLE__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
    if ( window->parent() ) {
      // Calculate rectangle for the subwindow
      // Unfortunatly this controls the origin as well as the drawable
      // area, so only clipping on the top and right is supported.
      //++ this gets called a lot if we have more than one GL buffer... .
      fltk::Rectangle r(*window);
      Widget* p = window->parent();
      for (;; p = p->parent()) {
	if (r.y() < 0) r.set_y(0); // clip top
	if (r.r() > p->w()) r.set_r(p->w()); // clip right
	if (!p->parent()) break;
	r.move(p->x(), p->y());
      }
      GLint rect[] = { r.x(), p->h()-r.b(), r.w(), r.h() };
      aglSetInteger( context, AGL_BUFFER_RECT, rect );
      aglEnable( context, AGL_BUFFER_RECT );
    }
    aglSetDrawable(context, GetWindowPort( xid(window) ) );
    aglSetCurrentContext(context);
#endif
# if USE_GLEW
    static bool beenhere = false;
    if (!beenhere) {
      beenhere = true;
      glewExperimental = GL_TRUE;
      glewInit();
    }
# endif
  }
}

void fltk::no_gl_context() {
#if USE_X11
  glXMakeCurrent(xdisplay, 0, 0);
#elif defined(_WIN32)
  wglMakeCurrent(0, 0);
#elif defined(__APPLE__)
  // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
  if (fl_current_glcontext) aglSetCurrentContext(0);
#endif
  fl_current_glcontext = 0;
  cached_window = 0;
}

void fltk::delete_gl_context(GLContext context) {
  if (fl_current_glcontext == context) no_gl_context();
  if (context != first_context) {
#if USE_X11
    if (first_context) {
      glXDestroyContext(xdisplay, context);
#if DESTROY_ON_EXIT
      Contexts** p = &context_list;
      Contexts* q = *p;
      while (q && q->context != context) {
        p = &(q->next);
        q = *p;
      }
      if (q) {*p = q->next; delete q;}
#endif
    }
#elif defined(_WIN32)
    wglDeleteContext(context);
#elif defined(__APPLE__)
    // warning: the Quartz version should probably use Core GL (CGL) instead of AGL
    aglSetDrawable( context, NULL );
    aglDestroyContext( context );
#endif
  }
}

#endif

//
// End of "$Id: Fl_Gl_Choice.cxx 6875 2009-09-17 01:57:42Z spitzak $".
//
