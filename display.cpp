#include "display.h"
GLWindow GLWin;
Bool keys[256];
int keyCodes[20];        /* array to hold our fetched keycodes */
int loop1;
GLuint texture[2];
GLuint base;


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
    glVertex2d(x1,y1);
    glVertex2d(x2,y2);
    glEnd();
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
void glSwap()
{
  glXSwapBuffers(GLWin.dpy, GLWin.win);
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
void ClearScreen()
{
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTranslatef(0.0f, 0.0f, -5.0f);
    glColor3f(1.0f, 0.5f, 1.0f);
}