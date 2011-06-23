
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

#ifdef WITH_SOUND
/* includes needed for sound */
#include "sound.h"
#endif

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "line.h"


FILE *usb;


line lines;
char buff[100]="";
int buffcount=0;
bool help = false;	// shows help screen if true

void UpdateSamples()
{
      
      int t,f,s;
      int input[CHANNELNUM+1];
static int lastf=0,lasts=0;
    if (!feof(usb))
    {
      fgets(buff,100,usb);
	      // time channel1 channel2
      sscanf(buff,"%d %d %d",&input[0],&input[1],&input[2]);
      buffcount++;
    }
    else { s = lasts; f = lastf;}
      //printf("%s %d %d\n",buff,f,s);
      lines.AddSample(input);		//FIXME: since we are now getting the timemark as the first parameter we shouldn't add samples everytime, only when a newone is done
   char *ch = buff;
   while(*ch && (*ch!='\n') && (*ch!='\r'))
      ch++;
   if ((*ch=='\r')|| (*ch=='\n'))
      *ch = '\0';

}
void ResetSamples()
{
  lines.Reset();
}

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


/* attributes for a single buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListSgl[] = { GLX_RGBA, GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None
};

/* attributes for a double buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None
};


enum KEYS { ESCAPE=0,F1=1,A=2,UP=3,DOWN=4,LEFT=5,RIGHT=6,SPACE=7,ONE=8,TWO=9,Q=10,W=11,S=12,HELP=13 };

GLWindow GLWin;
const char* title = "Quickie Arduino Scope";
Bool done;
Bool keys[256];
int keyCodes[20];        /* array to hold our fetched keycodes */
Bool scopePause;
Bool antiAliasing;

int loop1, loop2;

GLuint texture[2];
GLuint base;

unsigned int t0;
unsigned int frames;
unsigned int t;
int SampleRate=500;
int SampleStart=0;
unsigned int LastSample;

#ifdef WITH_SOUND
/* sound stuff */
int audioDevice;
waveFile* dieWave;
waveFile* completeWave;
waveFile* freezeWave;
waveFile* hourglassWave;
#endif


unsigned int getMilliSeconds()
{
    struct timeb tb;
    ftime(&tb);
    return tb.time * 1000 + tb.millitm;
}

/* simple loader for 24bit bitmaps (data is in rgb-format) */
int loadBmp(const char* filename, textureImage* texture)
{
    FILE* file;
    unsigned short int bfType;
    long int bfOffBits;
    short int biPlanes;
    short int biBitCount;
    long int biSizeImage;
    int i;
    unsigned char temp;
    /* make sure the file is there and open it read-only (binary) */
    if ((file = fopen(filename, "rb")) == NULL) {
        printf("File not found : %s\n", filename);
        return 0;
    }
    if (!fread(&bfType, sizeof(short int), 1, file)) {
        printf("Error reading file!\n");
        return 0;
    }
    /* check if file is a bitmap */
    if (bfType != 19778) {
        printf("Not a Bitmap-File!\n");
        return 0;
    }
    long filesize;
    /* get the file size */
    fread(&filesize,4,1,file);
    /* skip file size and reserved fields of bitmap file header */
    fseek(file, 0xa, SEEK_SET);
    bfOffBits = 0;
    /* get the position of the actual bitmap data */
    if (!fread(&bfOffBits, sizeof(4), 1, file)) {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Data at Offset: %ld\n", bfOffBits);
    /* skip size of bitmap info header */
    fseek(file, 4, SEEK_CUR);
    /* get the width of the bitmap */
    fread(&texture->width, sizeof(int), 1, file);
    printf("Width of Bitmap: %d\n", texture->width);
    /* get the height of the bitmap */
    fread(&texture->height, sizeof(int), 1, file);
    printf("Height of Bitmap: %d\n", texture->height);
    /* get the number of planes (must be set to 1) */
    fread(&biPlanes, sizeof(short int), 1, file);
    if (biPlanes != 1) {
        printf("Error: number of Planes not 1!\n");
        return 0;
    }
    /* get the number of bits per pixel */
    if (!fread(&biBitCount, sizeof(short int), 1, file)) {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Bits per Pixel: %d\n", biBitCount);
    if (biBitCount != 24) {
        printf("Bits per Pixel not 24\n");
        return 0;
    }
    /* calculate the size of the image in bytes */
    biSizeImage = texture->width * texture->height * 3;
    printf("Size of the image data: %ld\n", biSizeImage);
    texture->data = (unsigned char *)malloc(biSizeImage);
    /* seek to the actual data */
    fseek(file, bfOffBits, SEEK_SET);
    if (!fread(texture->data, biSizeImage, 1, file)) {
        printf("Error loading file!\n");
        return 0;
    }
    /* swap red and blue (bgr -> rgb) */
    for (i = 0; i < biSizeImage; i += 3) {
        temp = texture->data[i];
        texture->data[i] = texture->data[i + 2];
        texture->data[i + 2] = temp;
    }
    return 1;
}


/* Load Bitmaps And Convert To Textures */
Bool loadGLTextures()
{
    Bool status;
    textureImage* texti;

    status = False;
    texti = (textureImage*) malloc(sizeof(textureImage) * 2);
    if (loadBmp("Data/font.bmp", &texti[0]) &&
        loadBmp("Data/image.bmp", &texti[1])) {
        status = True;
        glGenTextures(2, &texture[0]);	/* create two textures */
            for (loop1 = 0; loop1 < 2; loop1++) {
            glBindTexture(GL_TEXTURE_2D, texture[loop1]);
            /* use linear filtering */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            /* actually generate the texture */
            glTexImage2D(GL_TEXTURE_2D, 0, 3, texti[loop1].width,
                    texti[loop1].height, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, texti[loop1].data);
        }
    }
    /* free the ram we used in our texture generation process */
    for (loop1 = 0; loop1 < 2; loop1++) {
        if (&texti[loop1]) {
            if (texti[loop1].data)
                free(texti[loop1].data);
        }
    }
    free(texti);
    return status;
}

void buildFont(void) {
    GLfloat cx, cy;         /* the character coordinates in our texture */
    base = glGenLists(256);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    for (loop1 = 0; loop1 < 256; loop1++)
    {
        cx = (float) (loop1 % 16) / 16.0f;
        cy = (float) (loop1 / 16) / 16.0f;
        glNewList(base + loop1, GL_COMPILE);
            glBegin(GL_QUADS);
                glTexCoord2f(cx, 1 - cy - 0.0625f);
                glVertex2i(0, 16);
                glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);
                glVertex2i(16, 16);
                glTexCoord2f(cx + 0.0625f, 1 - cy);
                glVertex2i(16, 0);
                glTexCoord2f(cx, 1 - cy);
                glVertex2i(0, 0);
            glEnd();
            glTranslated(15, 0, 0);
        glEndList();
    }
}

void killFont(void) {
    glDeleteLists(base, 256);
}

void printGLf(GLint x, GLint y, int set, const char* fmt, ...) {
    char text[256];
    va_list ap;
    if (fmt == NULL)
        return;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    if (set > 1)
        set = 1;
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    glTranslatef(x, y, 0);
    glListBase(base - 32 + 128 * set);
    if (set == 0)
        glScalef(1.5f, 2.0f, 1.0f);
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
    glDisable(GL_TEXTURE_2D);
}

/* function called when our window is resized (should only happen in window mode) */
void resizeGLScene(unsigned int width, unsigned int height)
{
    if (height == 0)        /* Prevent A Divide By Zero If The Window Is Too Small */
        height = 1;
    glViewport(0, 0, width, height);    /* Reset The Current Viewport And Perspective Transformation */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* general OpenGL initialization function */
int initGL()
{
    if (!loadGLTextures()) {
        return False;
    }
    buildFont();
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* we use resizeGLScene once to set up our initial perspective */
    resizeGLScene(GLWin.width, GLWin.height);
    glFlush();
    return True;
}
void DrawLine(GLint x1,GLint y1,GLint x2,GLint y2)
{
  glBegin(GL_LINES);
  glVertex2d(x1,y1); glVertex2d(x2,y2);
  glEnd();
}
/* Here goes our drawing code */

int drawGLLine(int line, int yoffset) 
{
      if (lines.IsVisible(line))
      {
	glBegin(GL_LINES);
	if (line == 0) glColor3f(1.0f, 1.0f, 0.0f);
	else glColor3ub(0,255,0);
	int top = lines.GetEnd();
	if (top<SampleRate)top=SampleRate;
	for(int x=0;x<top;x++)
	{
		glVertex2d(20+(int)((400.0*x)/top),yoffset - lines.GetChSample(SampleStart+x, line));
	}
	glEnd();
      }      

  
}

int drawGLScene()
{
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTranslatef(0.0f, 0.0f, -5.0f);
    glColor3f(1.0f, 0.5f, 1.0f);
    if (help)
    {
      printGLf(20,4,1,"1 and 2 toggle channels 1 and 2");
      printGLf(20,20,1,"q and a increases and decreases y scale for channel 1");
      printGLf(20,40,1,"w and s increases and decreases y scale for channel 2");
      printGLf(20,60,1,"space pauses");
      printGLf(20,80,1,"left/right arrows shift the sample time frame");
      printGLf(20,100,1,"up/down arrows change the sample rate");    
      glXSwapBuffers(GLWin.dpy, GLWin.win);
      return true;
    }	
    printGLf(20, 4, 1, "Press ? for help");
    glColor3f(1.0f, 1.0f, 0.0f);

    printGLf(20,20,1,"scale 1 = 1/%d, 2 = 1/%d",lines.GetScale(0),lines.GetScale(1));
    printGLf(20,40,1,"Sample %0.3f secs offset %d" ,SampleRate/5000.0,SampleStart);
    printGLf(20,60,1,"%d %s ",buffcount,buff);
    if (scopePause) {
        glColor3ub(rand() % 256, rand() % 256, rand() % 256);
        printGLf(472, 20, 1, "PAUSED");
    }
    glLineWidth(2.0f);
    glEnable(GL_LINE_SMOOTH);
    glColor3f(1.0f, 1.0f, 0.0f);
    glLoadIdentity();
    /*
    {
      int top = lines[0].GetEnd();
      if (lines[0].IsVisible())
      {
      if (top<SampleRate) top = SampleRate;
      glBegin(GL_LINES);
      glColor3f(1.0f, 1.0f, 0.0f);
      for(int x=0;x<top;x++)
      {
	glVertex2d(20+(int)((400.0*x)/top),270 - lines[0].GetSample(SampleStart,x));
//	printf("%d %d\n",x,70 + lines[0].GetSample(x));
      }
      glEnd();
      }
      */
      drawGLLine(0, 270);
      {
      unsigned int bt = lines.GetBaseTime(),mx = lines.GetMaxTime();
      unsigned int tt = mx - bt; // total time
      
      int top = lines.GetEnd();
      if (top<SampleRate) top = SampleRate;
      DrawLine(20,300,400,300);
      if (tt>0)
      {
      for(int x=0;x<top;x++)
      {
	int st = lines.GetTime(SampleStart+x);
	GLint tx =  st - bt;
	  tx = 20+tx;
	 DrawLine(tx,290,tx,310);
      
      }
      drawGLLine(1, 470);
      }      
    }
   
    glLineWidth(1.0f);
    if (antiAliasing)
        glEnable(GL_LINE_SMOOTH);
    glXSwapBuffers(GLWin.dpy, GLWin.win);
    frames++;
    GLfloat fps;
    t = getMilliSeconds();
    GLfloat sec = (t - t0) / 1000.0;
    fps = frames / sec;
    if (!scopePause)
    {
      UpdateSamples();
    }
    if (t - t0 >= 5000) {
        printf("%g FPS\n", fps);
        t0 = t;
        frames = 0;
    }
    return True;
}

/* function to release/destroy our resources and restoring the old desktop */
GLvoid killGLWindow()
{
    if (GLWin.ctx) {
    if (!glXMakeCurrent(GLWin.dpy, None, NULL)) {
        printf("Could not release drawing context.\n");
    }
    glXDestroyContext(GLWin.dpy, GLWin.ctx);
    GLWin.ctx = NULL;
    }
    /* switch back to original desktop resolution if we were in fs */
    if (GLWin.fs) {
        XF86VidModeSwitchToMode(GLWin.dpy, GLWin.screen, &GLWin.deskMode);
        XF86VidModeSetViewPort(GLWin.dpy, GLWin.screen, 0, 0);
    }
    killFont();
    XCloseDisplay(GLWin.dpy);
}

/* this function creates our window and sets it up properly */
Bool createGLWindow(const char *title, int width, int height, Bool fullscreenflag)
{
    XVisualInfo *vi;
    Colormap cmap;
    int dpyWidth, dpyHeight;
    int i;
    int glxMajorVersion, glxMinorVersion;
    int vidModeMajorVersion, vidModeMinorVersion;
    XF86VidModeModeInfo **modes;
    int modeNum;
    int bestMode;
    Atom wmDelete;
    Window winDummy;
    unsigned int borderDummy;

    GLWin.fs = fullscreenflag;
    /* set best mode to current */
    bestMode = 0;
    /* get a connection */
    GLWin.dpy = XOpenDisplay(0);
    GLWin.screen = DefaultScreen(GLWin.dpy);
    XF86VidModeQueryVersion(GLWin.dpy, &vidModeMajorVersion,
            &vidModeMinorVersion);
    printf("XF86VidModeExtension-Version %d.%d\n", vidModeMajorVersion,
       vidModeMinorVersion);
    XF86VidModeGetAllModeLines(GLWin.dpy, GLWin.screen, &modeNum, &modes);
    /* save desktop-resolution before switching modes */
    GLWin.deskMode = *modes[0];
    /* look for mode with requested resolution */
    for (i = 0; i < modeNum; i++) {
        if ((modes[i]->hdisplay == width)
                && (modes[i]->vdisplay == height)) {
            bestMode = i;
        }
    }
    /* get an appropriate visual */
    vi = glXChooseVisual(GLWin.dpy, GLWin.screen, attrListDbl);
    if (vi == NULL) {
        vi = glXChooseVisual(GLWin.dpy, GLWin.screen, attrListSgl);
        printf("Only Singlebuffered Visual!\n");
    } else {
        printf("Got Doublebuffered Visual!\n");
    }
    glXQueryVersion(GLWin.dpy, &glxMajorVersion, &glxMinorVersion);
    printf("glX-Version %d.%d\n", glxMajorVersion, glxMinorVersion);
    /* create a GLX context */
    GLWin.ctx = glXCreateContext(GLWin.dpy, vi, 0, GL_TRUE);
    /* create a color map */
    cmap = XCreateColormap(GLWin.dpy, RootWindow(GLWin.dpy, vi->screen),
               vi->visual, AllocNone);
    GLWin.attr.colormap = cmap;
    GLWin.attr.border_pixel = 0;

    if (GLWin.fs) {
        XF86VidModeSwitchToMode(GLWin.dpy, GLWin.screen, modes[bestMode]);
        XF86VidModeSetViewPort(GLWin.dpy, GLWin.screen, 0, 0);
        dpyWidth = modes[bestMode]->hdisplay;
        dpyHeight = modes[bestMode]->vdisplay;
        printf("Resolution %dx%d\n", dpyWidth, dpyHeight);
        XFree(modes);

        /* create a fullscreen window */
        GLWin.attr.override_redirect = True;
        GLWin.attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
                                KeyReleaseMask | StructureNotifyMask;
        GLWin.win = XCreateWindow(GLWin.dpy, RootWindow(GLWin.dpy, vi->screen),
                    0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput,
                    vi->visual, CWBorderPixel | CWColormap |
                    CWEventMask | CWOverrideRedirect, &GLWin.attr);
        XWarpPointer(GLWin.dpy, None, GLWin.win, 0, 0, 0, 0, 0, 0);
        XMapRaised(GLWin.dpy, GLWin.win);
        XGrabKeyboard(GLWin.dpy, GLWin.win, True, GrabModeAsync,
              GrabModeAsync, CurrentTime);
        XGrabPointer(GLWin.dpy, GLWin.win, True, ButtonPressMask,
             GrabModeAsync, GrabModeAsync, GLWin.win, None,
             CurrentTime);
    } else {
        /* create a window in window mode */
        GLWin.attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
                                KeyReleaseMask | StructureNotifyMask;
        GLWin.win = XCreateWindow(GLWin.dpy,
                    RootWindow(GLWin.dpy, vi->screen), 0,
                    0, width, height, 0, vi->depth,
                    InputOutput, vi->visual,
                    CWBorderPixel | CWColormap | CWEventMask,
                    &GLWin.attr);
        /* only set window title and handle wm_delete_events if in windowed mode */
        wmDelete = XInternAtom(GLWin.dpy, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(GLWin.dpy, GLWin.win, &wmDelete, 1);
        XSetStandardProperties(GLWin.dpy, GLWin.win, title,
                   title, None, NULL, 0, NULL);
        XMapRaised(GLWin.dpy, GLWin.win);
    }
    /* connect the glx-context to the window */
    glXMakeCurrent(GLWin.dpy, GLWin.win, GLWin.ctx);
    XGetGeometry(GLWin.dpy, GLWin.win, &winDummy, &GLWin.x, &GLWin.y,
                 &GLWin.width, &GLWin.height, &borderDummy, &GLWin.depth);
    printf("Depth %d\n", GLWin.depth);
    if (glXIsDirect(GLWin.dpy, GLWin.ctx))
        printf("Congrats, you have Direct Rendering!\n");
    else
        printf("Sorry, no Direct Rendering possible!\n");
    initGL();
    return True;
}

void initKeys()
{
    printf("Initializing keys...\n");
    /* get keycode for escape-key */
    keyCodes[ESCAPE] = XKeysymToKeycode(GLWin.dpy, XK_Escape);
    /* get keycode for F1 */
    keyCodes[F1] = XKeysymToKeycode(GLWin.dpy, XK_F1);
    /* get keycode for 'a' */
    keyCodes[A] = XKeysymToKeycode(GLWin.dpy, XK_a);
    keyCodes[Q] = XKeysymToKeycode(GLWin.dpy, XK_q);
    keyCodes[W] = XKeysymToKeycode(GLWin.dpy, XK_w);
    keyCodes[S] = XKeysymToKeycode(GLWin.dpy, XK_s);
    /* get keycode for Up-Arrow */
    keyCodes[UP] = XKeysymToKeycode(GLWin.dpy, XK_Up);
    /* get keycode for Down-Arrow */
    keyCodes[DOWN] = XKeysymToKeycode(GLWin.dpy, XK_Down);
    /* get keycode for Left-Arrow */
    keyCodes[LEFT] = XKeysymToKeycode(GLWin.dpy, XK_Left);
    /* get keycode for Right-Arrow */
    keyCodes[RIGHT] = XKeysymToKeycode(GLWin.dpy, XK_Right);
    /* get keycode for spacebar */
    keyCodes[SPACE] = XKeysymToKeycode(GLWin.dpy, XK_space);
    keyCodes[ONE] = XKeysymToKeycode(GLWin.dpy,XK_1);
    keyCodes[TWO] = XKeysymToKeycode(GLWin.dpy,XK_2);
    keyCodes[HELP] = XKeysymToKeycode(GLWin.dpy,XK_question);
}

void keyAction()
{
    if (keys[keyCodes[ESCAPE]])
        done = True;
    if (keys[keyCodes[F1]]) {
        killGLWindow();
        GLWin.fs = !GLWin.fs;
        createGLWindow(title, 640, 480, GLWin.fs);
        keys[keyCodes[F1]] = False;
    }
    if (keys[keyCodes[A]]) {
	lines.DecScale(0);
        keys[keyCodes[A]] = False;
    }
    if (keys[keyCodes[Q]]) {
	lines.IncScale(0);
        keys[keyCodes[Q]] = False;
    }
    if (keys[keyCodes[W]]) {
	lines.IncScale(1);
        keys[keyCodes[W]] = False;
    }
    if (keys[keyCodes[S]]) {
	lines.DecScale(1);
        keys[keyCodes[S]] = False;
    }   
    if (keys[keyCodes[HELP]]) {
      help = !help;
      keys[keyCodes[HELP]] = False;
    }
}

int main(int argc, char **argv)
{
    XEvent event;
    unsigned int start;
    int width=640;
    int height = 480;
    done = False;
const char* dev = "/dev/ttyUSB0";
  help = false;
  argc--;
  while(argc)
  {
    argv++;
    if (strcmp(*argv,"-d")==0)
    {
      argv++;
      argc--;
      dev = *argv;
    }
    if ((strcmp(*argv,"-h")==0) || help)
    {
      printf("Usage is scope [-h] [-d dev] [-w W H]\nwhere: -h is help\n  -d assign device\n  -w Width height (eg -w 640 480)\n");
      return 0;
    }
    if (strcmp(*argv,"-w")==0)
    {
      argv++;
      argc--;
      width = atoi(*argv++);
      argc--;
      height = atoi(*argv++);
      argc--;
      help |= (width==0 || height==0);
    }
  }
    usb = fopen(dev,"r");
    buff[0] = '\0';
    if (usb==0)
    {
      printf("could not open %s",dev);
      return 1;
    }
    
    
    /* default to fullscreen */
    GLWin.fs = False;
    createGLWindow(title, width, height, GLWin.fs);
    initKeys();
#ifdef WITH_SOUND
    dieWave = (waveFile *)malloc(sizeof(waveFile));
    loadWave("Data/die.wav", dieWave);
    freezeWave = (waveFile *)malloc(sizeof(waveFile));
    loadWave("Data/freeze.wav", freezeWave);
    completeWave = (waveFile *)malloc(sizeof(waveFile));
    loadWave("Data/complete.wav", completeWave);
    hourglassWave = (waveFile *)malloc(sizeof(waveFile));
    loadWave("Data/hourglass.wav", hourglassWave);
    /* we setup the sound device according to the format of our wave-file */
    initSound(dieWave->header->bitsPerSample,
        dieWave->header->numberOfChannels,
        dieWave->header->sampleRate);
#endif
    antiAliasing = True;
    scopePause = False;
    t0 = getMilliSeconds();
    /* wait for events */
    while (!done) {
        /* handle the events in the queue */
        while (XPending(GLWin.dpy) > 0) {
            XNextEvent(GLWin.dpy, &event);
            switch (event.type) {
                case Expose:
                    if (event.xexpose.count != 0)
                        break;
                    drawGLScene();
                    break;
                case ConfigureNotify:
                /* call resizeGLScene only if our window-size changed */
                    if ((event.xconfigure.width != GLWin.width) ||
                        (event.xconfigure.height != GLWin.height)) {
                        GLWin.width = event.xconfigure.width;
                        GLWin.height = event.xconfigure.height;
                        printf("Resize event\n");
                        resizeGLScene(event.xconfigure.width,
                                      event.xconfigure.height);
                    }
                    break;
                /* exit in case of a mouse button press */
                case ButtonPress:
                  //  done = True;
                    break;
                case KeyPress:
                    keys[event.xkey.keycode] = True;
                    break;
                case KeyRelease:
                    keys[event.xkey.keycode] = False;
                    break;
                case ClientMessage:
                    if (*XGetAtomName(GLWin.dpy,
                        event.xclient.message_type) ==
                        *"WM_PROTOCOLS") {
                        printf("Exiting sanely...\n");
                        done = True;
                    }
                    break;
                default:
                    break;
            }
        }
        start = getMilliSeconds();
        drawGLScene();
        keyAction();
        {
#ifdef WITH_SOUND
                    /* play die sound */
//                    playSound(dieWave);
#endif
                
            if (keys[keyCodes[ONE]]){ keys[keyCodes[ONE]] = false; lines.ToggleVisible(0); }
	    if (keys[keyCodes[TWO]]){ keys[keyCodes[TWO]] = false; lines.ToggleVisible(1); }
	  
            if (keys[keyCodes[LEFT]]) {
	        keys[keyCodes[LEFT]] = false;
		SampleStart-=100;
		if (SampleStart<0)SampleStart=0;
            }
            if (keys[keyCodes[RIGHT]]) {
		keys[keyCodes[RIGHT]] = false;
		SampleStart+=100;
		if (SampleStart>TRACELENGTH) SampleStart = TRACELENGTH;
            }
	  
            if (keys[keyCodes[UP]]) {
	        keys[keyCodes[UP]] = false;
		SampleRate+=10;
            }
            if (keys[keyCodes[DOWN]]) {
		keys[keyCodes[DOWN]] = false;
		if (SampleRate>10)
		SampleRate-=10;
		
              
            }
	    if (keys[keyCodes[SPACE]])
	    {
		keys[keyCodes[SPACE]] = false;
		scopePause = !scopePause;
	    }
        }
    }
#ifdef WITH_SOUND
    killsound();
#endif
    killGLWindow();
    fclose(usb);
    return 0;
}
