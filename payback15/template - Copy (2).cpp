/*
	4k Procedual Gfx template code.
	based on "chocolux" codes by alud.
*/

#include <windows.h>

#include <GL/gl.h>
#include "glext.h"
#include <math.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "bass.h"
#include <stdio.h>
#include <stdarg.h>
#include "lib/sync.h"

static const float bpm = 150.0f; /* beats per minute */
static const int rpb = 8; /* rows per beat */
static const double row_rate = (double(bpm) / 60) * rpb;

static double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	return time * row_rate;
}

static void bass_set_row(void *d, int row)
{
	HSTREAM h = *((HSTREAM *)d);
	QWORD pos = BASS_ChannelSeconds2Bytes(h, row / row_rate);
	BASS_ChannelSetPosition(h, pos, BASS_POS_BYTE);
}

#ifndef SYNC_PLAYER

static void bass_pause(void *d, int flag)
{
	HSTREAM h = *((HSTREAM *)d);
	if (flag)
		BASS_ChannelPause(h);
	else
		BASS_ChannelPlay(h, false);
}

static int bass_is_playing(void *d)
{
	HSTREAM h = *((HSTREAM *)d);
	return BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
}

static struct sync_cb bass_cb = {
	bass_pause,
	bass_set_row,
	bass_is_playing
};

#endif /* !defined(SYNC_PLAYER) */

static void die(const char *fmt, ...)
{
	char temp[4096];
	va_list va;
	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp), fmt, va);
	va_end(va);

#if !defined(_WIN32) || defined(_CONSOLE)
	fprintf(stderr, "*** error: %s\n", temp);
#else
	MessageBox(NULL, temp, NULL, MB_OK | MB_ICONERROR);
#endif

	return;
}

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define USE_SOUND_THREAD

#include "4klang.h"

SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES*2];  
HWAVEOUT	hWaveOut;

static const char *vsh =
"varying vec3 v,c;"
"void main(){gl_Position=gl_Vertex;v=vec3(0,0,0);c=normalize(vec3(gl_Vertex.x*1.6,gl_Vertex.y,2));"
"}";

# define VAR_DIR "c"
# define VAR_IGLOBALTIME "y"
# define VAR_IRESOLUTION "k"
# define VAR_ROT "rot"
# define VAR_ORG "v"

const char *fsh =
 "varying vec3 v,c;"
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "void n(in vec3 v,in vec3 s,out vec4 f)"
 "{"
   "int k;"
   "vec3 i=v;"
   "for(int x=0;x<200;x+=1)"
     "{"
       "i+=s;"
       "vec3 c=floor(i/vec3(y*.1)),m=mod(c,1.);"
       "m.x=mod(m.x+floor(y*10.),16.);"
       "float r=(m.z+m.y)*16.+m.x;"
       "vec2 z=vec2(floor(r/512.),mod(r,512.))/vec2(512.);"
	   "vec4 n = vec4(1.0,1.0,1.0,1.0);"
	   "if (y < 30.0) n=vec4(1.*cos(y*1.9),1.*cos(y*1.9),1.*cos(y*1.9),1.*cos(y*1.9));"
       "float l=length(n),b=0.;"
       "b+=sin(c.x*.5523);"
       "b+=sin(c.x*.23+y*5.);"
       "b+=sin(c.x*.0152+y*3.1);"
       "b+=sin(c.z*.7632);"
       "b+=sin(c.z*.25234);"
       "b+=sin(c.z*.45234);"
       "b=b*2.9-6.;"
       "if(c.x==0.&&c.y==0.&&c.z==0.)"
         "{"
           "f=vec4(1.);"
           "break;"
         "}"
       "else"
         " if(c.y<b)"
           "{"
             "f.x=.4+c.y*-.1;"
             "f.y=.4+c.y*-.05;"
             "f.z=.4+c.y*-.03;"
             "break;"
           "}"
         "else"
           " if(l>1.2&&length(c.yz)<10.)"
             "{"
               "f=n;"
               "break;"
             "}"
     "}"
 "}"
 "void main()"
 "{"
   "vec3 x=v;"
   "vec2 c=gl_FragCoord.xy/k.xy;"
   "c.xy-=vec2(.5);"
   "c.xy+=vec2(.5);"
   "float f=1.1,i=sin(y*.432*f)*2.1,r=sin(y*1.21231*f)-.9,m=sin(y*.73223*f)-.6;"
   "mat3 s=mat3(cos(i),0.,-sin(i),0.,1.,0.,sin(i),0.,cos(i)),z=mat3(1.,0.,0.,0.,cos(r),-sin(r),0.,sin(r),cos(r)),b=mat3(cos(m),-sin(m),0.,sin(m),cos(m),0.,0.,0.,1.),o=mat3(1.,0.,0.,0.,.3,0.,0.,0.,1.);"
   "s=mat3(1.)*z*s;"
   "mat3 l=mat3(s[0][0],s[1][0],s[2][0],s[0][1],s[1][1],s[2][1],s[0][2],s[1][2],s[2][2]);"
   "vec3 u=vec3(sin(y*1.9)*.5,.3,-14.5+rot),g=vec3((c.x-.5)*1.6,c.y-.5,1.);"
   "g=normalize(g);"
   "u=s*u;"
   "g=s*g;"
   "u.y+=10.;"
   "vec4 p;"
   "n(u,g,p);"
   "\n#if 0\n"
   "gl_FragColor=p;"
   "\n#else\n"
   "vec4 e=vec4(pow(length(c-vec2(.5)),5.))*vec4(1.);"
   "e=e*18.4*(.02+e);"
   "e*=vec4(.3,.6,1.,1.);"
   "if (p.r < 0.1) discard;"
   "gl_FragColor=clamp(p*vec4(1.),vec4(0.),vec4(1.));"
   "\n#endif\n"
 "}";

const char *fsh2 =
 "varying vec3 v,c;"
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
"float lineWidth = 0.01;"
"float border = 0.05;"
"float scale = 0.07;"
"float t = 0.0;"
"float line(vec2 p,vec2 s,vec2 e){s*=scale;e*=scale;float l=length(s-e);vec2 d=vec2(e-s)/l;p-=vec2(s.x,-s.y);p=vec2(p.x*d.x+p.y*-d.y,p.x*d.y+p.y*d.x);return length(max(abs(p-vec2(l/2.0,0))-vec2(l/2.0,lineWidth/2.0),0.0))-border;}"
"float A(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(5,8)));return d;}float B(vec2 p){float d=1.0;d=min(d,line(p,vec2(4,5),vec2(4,1.5)));d=min(d,line(p,vec2(4,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(1,5)));return d;}float C(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));return d;}float D(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(4,8)));d=min(d,line(p,vec2(4,8),vec2(4.5,7.5)));d=min(d,line(p,vec2(4.5,7.5),vec2(5,6.25)));d=min(d,line(p,vec2(5,6.25),vec2(5,3.75)));d=min(d,line(p,vec2(5,3.75),vec2(4.5,2)));d=min(d,line(p,vec2(4.5,2),vec2(4,1.5)));d=min(d,line(p,vec2(4,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));return d;}float E(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));return d;}float F(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(1,8)));return d;}float G(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,2.5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(3.5,5)));return d;}float H(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(5,8)));return d;}float I(vec2 p){float d=1.0;d=min(d,line(p,vec2(1.5,1.5),vec2(4.5,1.5)));d=min(d,line(p,vec2(4.5,1.5),vec2(3,1.5)));d=min(d,line(p,vec2(3,1.5),vec2(3,8)));d=min(d,line(p,vec2(3,8),vec2(1.5,8)));d=min(d,line(p,vec2(1.5,8),vec2(4.5,8)));return d;}float J(vec2 p){float d=1.0;d=min(d,line(p,vec2(1.5,8),vec2(3,8)));d=min(d,line(p,vec2(3,8),vec2(4,7)));d=min(d,line(p,vec2(4,7),vec2(4,1.5)));d=min(d,line(p,vec2(4,1.5),vec2(1.5,1.5)));return d;}float K(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(2.5,5)));d=min(d,line(p,vec2(2.5,5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(2.5,5)));d=min(d,line(p,vec2(2.5,5),vec2(5,8)));return d;}float L(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));return d;}float M(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(3,4)));d=min(d,line(p,vec2(3,4),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(5,8)));return d;}float N(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,1.5)));return d;}float O(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,1.5)));return d;}float P(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(1,5)));return d;}float Q(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,8),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(3.5,6.5)));return d;}float R(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,8),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(3.5,5)));d=min(d,line(p,vec2(3.5,5),vec2(5,8)));return d;}float S(vec2 p){float d=1.0;d=min(d,line(p,vec2(5,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(1,5)));d=min(d,line(p,vec2(1,5),vec2(5,5)));d=min(d,line(p,vec2(5,5),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(1,8)));return d;}float T(vec2 p){float d=1.0;d=min(d,line(p,vec2(3,8),vec2(3,1.5)));d=min(d,line(p,vec2(3,1.5),vec2(1,1.5)));d=min(d,line(p,vec2(1,1.5),vec2(5,1.5)));return d;}float U(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,1.5)));return d;}float V(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(3,8)));d=min(d,line(p,vec2(3,8),vec2(5,1.5)));return d;}float W(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(3,6)));d=min(d,line(p,vec2(3,6),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(5,1.5)));return d;}float X(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(5,8)));d=min(d,line(p,vec2(5,8),vec2(3,4.75)));d=min(d,line(p,vec2(3,4.75),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(1,8)));return d;}float Y(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(3,8)));d=min(d,line(p,vec2(3,8),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(5,1.5)));return d;}float Z(vec2 p){float d=1.0;d=min(d,line(p,vec2(1,1.5),vec2(5,1.5)));d=min(d,line(p,vec2(5,1.5),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(1.5,5)));d=min(d,line(p,vec2(1.5,5),vec2(4.5,5)));d=min(d,line(p,vec2(4.5,5),vec2(3,5)));d=min(d,line(p,vec2(3,5),vec2(1,8)));d=min(d,line(p,vec2(1,8),vec2(5,8)));return d;}"

"void main()"
"{"	
"	 t = y;"
"    lineWidth = -0.3+cos(y*1.0*(cos(gl_FragCoord.x*0.01+y)+sin(gl_FragCoord.y*0.001))*1.5)*0.05;"
"	vec2 uv = (2.0 * gl_FragCoord.xy - k.xy) / k.yy;"
"	uv.x *= abs(cos(uv.x+t*2.0)*0.5+1.0)+1.0;"
"	uv.y *= abs(cos(uv.y+t*2.0)+1.0)+1.0;"
"	vec3 col = vec3(uv.y)*vec3(0.1,0.1,0.1);"
"	float d = 5.0;"
"	d = min(d,Q(uv-vec2(-2.0,0.5)));"
"	d = min(d,U(uv-vec2(-1.5,0.5)));"
"	d = min(d,A(uv-vec2(-1.0,0.5)));"
"	d = min(d,D(uv-vec2(-0.5,0.5)));"
"	d = min(d,T(uv-vec2( 0.0,0.5)));"
"	d = min(d,R(uv-vec2( 0.5,0.5)));"
"	d = min(d,I(uv-vec2( 1.0,0.5)));"
"	d = min(d,P(uv-vec2( 1.5,0.5)));"
"	float w = fwidth(d*3.0);"
"	gl_FragColor = vec4(vec3(abs(d*7.1+tan(y)*0.2)),1.0);"
"}";

const char *fshfb =
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "uniform sampler2D tex;"
"float t = 0.0;"
"float frac(vec2 co) {"
"return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453)*vec2(0.008,0.004);"
"}"
 "void main() {"
"	 t = y;"
"vec2 uv = (gl_FragCoord.xy - k.x) / k.yy/ 2.0;"
"gl_FragColor = vec4(0.04*cos(t*0.001+gl_FragCoord.y*4.001));"
"float zc = 0.2+cos(t*0.01)*0.1;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.028*zc+frac(uv+t)))*0.0044299121055113265;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.024*zc+frac(uv+t)))*0.00895781211794;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.020*zc+frac(uv+t)))*0.0215963866053;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.016*zc+frac(uv+t)))*0.0443683338718;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.012*zc+frac(uv+t)))*0.0776744219933;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.008*zc+frac(uv+t)))*0.115876621105;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.004*zc+frac(uv+t)))*0.147308056121;"
"    gl_FragColor *= texture2D(tex, uv         )*3.659576912161+cos(t*0.01)*0.1;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.004*zc+frac(uv+t)))*0.147308056121;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.008*zc+frac(uv+t)))*0.115876621105;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.012*zc+frac(uv+t)))*0.0776744219933;"
"   gl_FragColor -= texture2D(tex,  uv+vec2(0.016*zc+frac(uv+t)))*0.0443683338718;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.020*zc+frac(uv+t)))*0.0215963866053;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.024*zc+frac(uv+t)))*0.00895781211794;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.028*zc+frac(uv+t)))*0.0044299121055113265;"
 "}";


int	SCREEN_WIDTH = 1920;
int	SCREEN_HEIGHT =	1080;

static const PIXELFORMATDESCRIPTOR pfd =
{
	0,1,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 32, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0
};

#pragma data_seg(".wavefmt")
WAVEFORMATEX WaveFMT =
{
#ifdef FLOAT_32BIT	
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif		
    2, // channels
    SAMPLE_RATE, // samples per sec
    SAMPLE_RATE*sizeof(SAMPLE_TYPE)*2, // bytes per sec
    sizeof(SAMPLE_TYPE)*2, // block alignment;
    sizeof(SAMPLE_TYPE)*8, // bits per sample
    0 // extension not needed
};

GLuint color_tex;
GLuint fb;

#pragma data_seg(".wavehdr")
WAVEHDR WaveHDR = 
{
	(LPSTR)lpSoundBuffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0, 
	0, 
	0, 
	0, 
	0, 
	0
};

MMTIME MMTime = 
{ 
	TIME_SAMPLES,
	0
};

/////////////////////////////////////////////////////////////////////////////////
// crt emulation
/////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	int _fltused = 1;
}

#pragma code_seg(".initsnd")
void  InitSound()
{
/*
#ifdef USE_SOUND_THREAD
	// thx to xTr1m/blu-flame for providing a smarter and smaller way to create the thread :)
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
#else
	_4klang_render(lpSoundBuffer);
#endif
	waveOutOpen			( &hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader( hWaveOut, &WaveHDR, sizeof(WaveHDR) );
	waveOutWrite		( hWaveOut, &WaveHDR, sizeof(WaveHDR) );	
*/
}

#define EFF_WIDTH  SCREEN_WIDTH
#define EFF_HEIGHT SCREEN_HEIGHT

int frames = 0;
void main()
{
	//int msgboxID=MessageBox(NULL, "Press OK for 1920x1080   OR   Cancel for 1280x720","Quadtrip", MB_OKCANCEL | MB_DEFBUTTON2);
	int msgboxID = 0;
	switch(msgboxID){
		case IDCANCEL:
		{
			break;
		}

		default:
			break;	
	}

	DEVMODE dmScreenSettings =
	{
	"",0,0,sizeof(dmScreenSettings),0,DM_PELSWIDTH|DM_PELSHEIGHT,
	0,0,0,0,0,0,0,0,0,0,0,0,0,"",0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0,0,0,0,0,0,0,0,0,0
	};

	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN); 
	HDC hDC = GetDC( CreateWindow("edit", 0, WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0) );
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd) , &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	ShowCursor(0);
	// Uncomment for 4klang InitSound();

	const GLuint p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint p2 = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint pfb = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();

	GLuint s = ((PFNGLCREATESHADERPROC)(wglGetProcAddress("glCreateShader")))(GL_VERTEX_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &vsh,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fsh,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fsh2,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p2,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fshfb,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(pfb,s);


	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p2);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(pfb);
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);

	GLint loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IGLOBALTIME);
	GLint loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_ROT);
	GLint loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IRESOLUTION);

	((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);

	// render to texture


   glGenTextures(1, &color_tex);
   glBindTexture(GL_TEXTURE_2D, color_tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //NULL means reserve texture memory, but texels are undefined
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, EFF_WIDTH, EFF_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   //-------------------------
	glBindTexture(GL_TEXTURE_2D, 0);

   ((PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT"))(1, &fb);
   ((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, fb);
   ((PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT"))(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex, 0);

   ((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, 0);

   GLenum status;
  status = ((PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT"))(GL_FRAMEBUFFER_EXT);
  switch(status)
  {
     case GL_FRAMEBUFFER_COMPLETE_EXT:
     break;
  default:
     return;
  }

   HSTREAM stream;

	const struct sync_track *clear_r, *clear_g, *clear_b;
	const struct sync_track *cam_rot, *cam_dist;

	/* init BASS */
	if (!BASS_Init(-1, 44100, 0, 0, 0))
		die("failed to init bass");
	stream = BASS_StreamCreateFile(false, "kuu2.mp3", 0, 0,
	    BASS_STREAM_PRESCAN);
	if (!stream)
		die("failed to open tune");

	sync_device *rocket = sync_create_device("sync");
	if (!rocket)
		die("out of memory?");

#ifndef SYNC_PLAYER
	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
		die("failed to connect to host");
#endif

	/* get tracks */
	clear_r = sync_get_track(rocket, "clear.r");
	clear_g = sync_get_track(rocket, "clear.g");
	clear_b = sync_get_track(rocket, "clear.b");
	cam_rot = sync_get_track(rocket, "cam.rot"),
	cam_dist = sync_get_track(rocket, "cam.dist");

	/* let's roll! */

	BASS_Start();
	BASS_ChannelPlay(stream, false);

	glEnable(GL_TEXTURE_2D);
	//bass_set_row(&stream,4872);
loop:
	((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, fb);
	glViewport( 0, 0, EFF_WIDTH, EFF_HEIGHT );
	



	// Uncomment for 4klang waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));

	//const float t = 700.0+(float)(MMTime.u.sample >> 8)*0.05;

	double row = bass_get_row(stream);
	
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pfb);
			
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, color_tex);
		((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
		loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IGLOBALTIME);
		loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_ROT);
		loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IRESOLUTION);
		GLint loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,"tex");

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,(float)frames);
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,EFF_WIDTH,EFF_HEIGHT);
		((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
		glRecti(1,1,-1,-1);


#ifndef SYNC_PLAYER
		if (sync_update(rocket, (int)floor(row), &bass_cb, (void *)&stream))
			sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif
			
//	sync_update(rocket, (int)floor(row), &bass_cb, (void *)&stream);

	frames++;

	if (row < 1280 || row > 1360) {
		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);
		GLint loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IGLOBALTIME);
		GLint loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_ROT);
		GLint loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IRESOLUTION);

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,float(sync_get_val(cam_dist, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,EFF_WIDTH,EFF_HEIGHT);
		glRecti(1,1,-1,-1);
	}
	else if (row >= 1280 && row <= 1360) {
		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p2);

		GLint loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_IGLOBALTIME);
		GLint loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_ROT);
		GLint loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_IRESOLUTION);

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,float(sync_get_val(cam_dist, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,EFF_WIDTH,EFF_HEIGHT);
		glRecti(1,1,-1,-1);
	}

	glFinish();

	((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, 0);
	glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	glClear(GL_COLOR_BUFFER_BIT);
	
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pfb);

			
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, color_tex);
		((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
		loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IGLOBALTIME);
		loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_ROT);
		loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IRESOLUTION);
		loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,"tex");

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,(float)frames);
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);
		((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
		glRecti(1,1,-1,-1);


	SwapBuffers(hDC);

	BASS_Update(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Uncomment for klang if (GetAsyncKeyState(VK_ESCAPE) || MMTime.u.sample >= MAX_SAMPLES) ExitProcess(0);
	if (GetAsyncKeyState(VK_ESCAPE)) goto quit;
	goto loop;
quit:
	#ifndef SYNC_PLAYER
	sync_save_tracks(rocket);
#endif

	sync_destroy_device(rocket);

	BASS_StreamFree(stream);
	BASS_Free();

	ExitProcess(0);
}
