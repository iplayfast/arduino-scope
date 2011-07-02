#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>
#include <math.h>
#include <sys/timeb.h>
/* stuff about our window grouped together */
typedef struct {
    Display *dpy;
    int screen;
    Window win;
    GLXContext ctx;
    XSetWindowAttributes attr;
    Bool fs;
    XF86VidModeModeInfo deskMode;
    int x, y;
    unsigned int width, height;
    unsigned int depth;
} GLWindow;

typedef struct {
    int width;
    int height;
    unsigned char* data;
} textureImage;

enum KEYS { ESCAPE=0,F1=1,A=2,UP=3,DOWN=4,LEFT=5,RIGHT=6,SPACE=7,ONE=8,TWO=9,Q=10,W=11,S=12,HELP=13 };

extern GLWindow GLWin;
extern Bool keys[256];
extern int keyCodes[20];  
GLvoid killGLWindow();
int loadBmp(const char* filename, textureImage* texture);
Bool createGLWindow(const char *title, int width, int height, Bool fullscreenflag);
void initKeys();
void DrawLine(GLint x1,GLint y1,GLint x2,GLint y2);
int initGL();


/* Load Bitmaps And Convert To Textures */
Bool loadGLTextures();
void ClearScreen();
void buildFont(void);

void killFont(void);

void glSwap();
void printGLf(GLint x, GLint y, int set, const char* fmt, ...);

/* function called when our window is resized (should only happen in window mode) */
void resizeGLScene(unsigned int width, unsigned int height);