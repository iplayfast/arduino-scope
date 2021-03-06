
#include <stdio.h>
#include <stdlib.h>


#include "display.h"
#ifdef WITH_SOUND
/* includes needed for sound */
#include "sound.h"
#endif

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "line.h"
#include "randomseriesproducer.h"
#include "fileseriesproducer.h"

FILE *usb;
int width=640;
int height = 480;


char buff[100]="";
int buffcount=0;
bool help = false;	// shows help screen if true
bool Visible[2];		// TODO: needs to be moved into a display class
int Scale[2];		// TODO: needs to be moved into a display class
SeriesProducer *Producer;

void ResetSamples()
{
    Producer->Clear();
}







const char* title = "Quickie Arduino Scope";
Bool done;

Bool scopePause;
Bool antiAliasing;

int loop2;

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

/* Here goes our drawing code */

int drawGLLine(int line, int yoffset)
{
    if (Visible[line])
    {
        glBegin(GL_LINES);
        if (line == 0) glColor3f(1.0f, 1.0f, 0.0f);
        else glColor3ub(0,255,0);
	int top = Producer->GetEnd(line);
        if (top<SampleRate)top=SampleRate;
        for (int x=0;x<top;x++)
        {
	int y = Producer->GetSample(line,x);
            glVertex2d(20+(int)((400.0*x)/top),yoffset - y);
        }
        glEnd();
    }
}
int DrawScreen()
{

  ClearScreen();
    if (help)
    {
        printGLf(20,4,1,"1 and 2 toggle channels 1 and 2");
        printGLf(20,20,1,"q and a increases and decreases y scale for channel 1");
        printGLf(20,40,1,"w and s increases and decreases y scale for channel 2");
        printGLf(20,60,1,"space pauses");
        printGLf(20,80,1,"left/right arrows shift the sample time frame");
        printGLf(20,100,1,"up/down arrows change the sample rate");
	glSwap();
        return true;
    }
    printGLf(20, 4, 1, "Press ? for help");
    glColor3f(1.0f, 1.0f, 0.0f);

    printGLf(20,20,1,"scale 1 = 1/%d, 2 = 1/%d",Scale[0],Scale[1]);
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
      if (Visible[0])
      {
      if (top<SampleRate) top = SampleRate;
      glBegin(GL_LINES);
      glColor3f(1.0f, 1.0f, 0.0f);
      for(int x=0;x<top;x++)
      {
    glVertex2d(20+(int)((400.0*x)/top),270 - lines[0].GetSample(SampleStart,x));
    //    printf("%d %d\n",x,70 + lines[0].GetSample(x));
      }
      glEnd();
      }
      */
      drawGLLine(0, 270);
    {
	int numberOfChannels = Producer->GetNumChannels();
	if (numberOfChannels==0) return 0; // shouldn't happen 
	int div = height / numberOfChannels;
	int YLine=0;
	for(int i=0;i<numberOfChannels;i++)
	{
	  YLine += div; // move to bottom of channel
	  DrawLine(0,YLine,width,YLine); // draw the channel marker
	  int numsamples = Producer->SampleCount(i);	// number of samples this channel received
	  for(int s=0;s<numsamples;s++)
	  {
	    int st = Producer->SampleTime(i,s);
	    DrawLine(st,YLine,st,YLine - Producer->SampleValue(i,s)); // draw a tick up to the sample
	  }
	}
    }

    glLineWidth(1.0f);
    if (antiAliasing)
        glEnable(GL_LINE_SMOOTH);
    glSwap();
    frames++;
    GLfloat fps;
    t = getMilliSeconds();
    GLfloat sec = (t - t0) / 1000.0;
 static GLfloat Lastsec = 0;  // a cheat... I know
    if (Lastsec!=sec)
    {
      Lastsec = sec;
      Producer->Clear();
    }
    fps = frames / sec;
    if (!scopePause)
    {
       Producer->readSeries();
    }
    if (t - t0 >= 5000) {
        printf("%g FPS\n", fps);
        t0 = t;
        frames = 0;
    }
    return True;
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
	Scale[0]--;
        keys[keyCodes[A]] = False;
    }
    if (keys[keyCodes[Q]]) {
	Scale[0]++;
        keys[keyCodes[Q]] = False;
    }
    if (keys[keyCodes[W]]) {
	Scale[1]++;
        keys[keyCodes[W]] = False;
    }
    if (keys[keyCodes[S]]) {
	Scale[1]--;
        keys[keyCodes[S]] = False;
    }
    if (keys[keyCodes[HELP]]) {
        help = !help;
        keys[keyCodes[HELP]] = False;
    }
}

void initSound()
{
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
}

SeriesProducer *initArgs(int argc,char **argv)
{
  SeriesProducer *Result = 0;
  const char* dev = "/dev/stdin";
  help = false;
  argc--;
  argv++;
  while (argc ||help)
  {
    if (help || (argc && (strcmp(*argv,"--help")==0) || (strcmp(*argv,"-h")==0)))
    {
      printf("Usage is scope [-h] [-d dev] [-s simulation] [-w W H]\nwhere: -h is help\n  -d assign device\n-s simulation, of type random or sine\n  -w Width height (eg -w 640 480)\n");
      return 0;
    }
    if (argc && strcmp(*argv,"-d")==0)
    {
      argv++;
      argc--;
      argc--;
      dev = *argv++;
      continue;
    }
    if (argc && strcmp(*argv,"-s")==0)	// simulation
	{
	  argc--; argv++;
	  if (strcmp(*argv,"random")==0)
	  {
	    argc--; argv++;
	    Result = new RandomSeriesProducer(0,512,2);
	    // TODO need to setup parameters for RandomSeriesProducer
	  }
	  if (strcmp(*argv,"sine")==0)
	  {
	    printf("Sorry sine isn't implemented yet\n");
	    return 0;
	    //TODO need to setup sine producer
	  }
	  continue;
	}
	if (argc && strcmp(*argv,"-w")==0)
	{
	  argv++;
	  argc--;
	  width = atoi(*argv++);
	  argc--;
	  height = atoi(*argv++);
	  argc--;
	  help |= (width==0 || height==0);
	  continue;
	}
	help = true; // if got to here then entered an argument we don't understand
  }
  if (Result==0)	// not assigned to a simulation, so must be a file
    {
      try
      {
      FileSeriesProducer *Result = new FileSeriesProducer(dev,2); // TODO: assuming 2 channels make more generic
      return Result;
      }
      catch(const char* error)
      {
	printf("%s %s\n",error,dev);
	return 0;
      }
    }
    return Result;
}

int main(int argc, char **argv)
{
    XEvent event;
    unsigned int start;
        buff[0] = '\0';
    done = False;
    Producer = initArgs(argc,argv);
    if (Producer==0) return 1; // no producer we must die
    /* default to fullscreen */
    GLWin.fs = False;
    createGLWindow(title, width, height, GLWin.fs);
    initKeys();
    initSound();

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
                DrawScreen();
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
        DrawScreen();
        keyAction();
        {
#ifdef WITH_SOUND
            /* play die sound */
//                    playSound(dieWave);
#endif

            if (keys[keyCodes[ONE]]) {
                keys[keyCodes[ONE]] = false;
                Visible[0] = !Visible[0];
            }
            if (keys[keyCodes[TWO]]) {
                keys[keyCodes[TWO]] = false;
                Visible[1] = !Visible[1];
            }

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
