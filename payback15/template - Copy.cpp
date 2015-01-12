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

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define USE_SOUND_THREAD

#include "4klang.h"

SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES*2];  
HWAVEOUT	hWaveOut;

static const char *vsh =
"varying vec3 org,dir;void main(){gl_Position=gl_Vertex;org=vec3(0,0,0);dir=normalize(vec3(gl_Vertex.x*1.6,gl_Vertex.y,2));}"
;
static const char *fsh =
	"uniform float time;varying vec3 org,dir;float flr(vec3 p,float f){return abs(f-p.y);}float sph(vec3 p,vec4 spr){return length(spr.xyz-p)-spr.w;}float cly(vec3 p,vec4 cld){return length(vec2(cld.x+0.5*sin(p.x+p.z*2.0+time)*cos(p.y+p.z*2.0),cld.z)-p.xz)-cld.w;}float scene(vec3 p){float d=flr(p,-5.0);d=min(d,flr(p,5.0));for(float ii=0.0;ii<8.0;ii+=1.0){float x = cos(time*0.2+ii*(time*0.0004))*2.0; d=min(d,cly(p,vec4(x*6.0,0,10+ii*4.0,1.1)));}return d;}vec3 getN(vec3 p){float eps=0.01;return normalize(vec3(scene(p+vec3(eps,0,0))-scene(p-vec3(eps,0,0)),scene(p+vec3(0,eps,0))-scene(p-vec3(0,eps,0)),scene(p+vec3(0,0,eps))-scene(p-vec3(0,0,eps))));}float AO(vec3 p,vec3 n){float dlt=0.5;float oc=0.0,d=1.0;for(int i=0;i<6;i++){oc+=(float(i)*dlt-scene(p+n*float(i)*dlt))/d;d*=2.0;}return 1.0-oc;}void main(){float g,d=0.0;vec3 p=org;for(int i=0;i<64;i++){d=scene(p);p=p+d*dir;}if(d>1.0){gl_FragColor=vec4(0,0,0,1);return;}vec3 n=getN(p);float a=AO(p,n);vec3 s=vec3(0,0,0);vec3 lp[3],lc[3];lp[0]=vec3(-4,0,4);lp[1]=vec3(2,3,8);lp[2]=vec3(4,-2,24);lc[0]=vec3(1.0,0.5,0.4);lc[1]=vec3(0.4,0.5,1.0);lc[2]=vec3(0.2,1.0,0.5);for(int i=0;i<3;i++){vec3 l,lv;lv=lp[i]-p;l=normalize(lv);g=length(lv);g=max(0.0,dot(l,n))/g*float(10);s+=g*lc[i];}float fg=min(0.8+cos(time*0.05)*0.5,0.7+(abs(15.0*sin(time*0.1+gl_FragCoord.y*0.1)*cos(time*50.1)))/length(p-org));gl_FragColor=vec4(s*a,1)*fg*fg;}"
;

#define	SCREEN_WIDTH	1920
#define	SCREEN_HEIGHT	1080

static const PIXELFORMATDESCRIPTOR pfd =
{
	0,1,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, 32, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0
};
static DEVMODE dmScreenSettings =
{
	"",0,0,sizeof(dmScreenSettings),0,DM_PELSWIDTH|DM_PELSHEIGHT,
	0,0,0,0,0,0,0,0,0,0,0,0,0,"",0,0,SCREEN_WIDTH,SCREEN_HEIGHT,0,0,0,0,0,0,0,0,0,0
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
#ifdef USE_SOUND_THREAD
	// thx to xTr1m/blu-flame for providing a smarter and smaller way to create the thread :)
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
#else
	_4klang_render(lpSoundBuffer);
#endif
	waveOutOpen			( &hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader( hWaveOut, &WaveHDR, sizeof(WaveHDR) );
	waveOutWrite		( hWaveOut, &WaveHDR, sizeof(WaveHDR) );	
}


void entrypoint()
{
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN); 
	HDC hDC = GetDC( CreateWindow("edit", 0, WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0) );
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd) , &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	ShowCursor(0);
	InitSound();

	const GLuint p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	GLuint s = ((PFNGLCREATESHADERPROC)(wglGetProcAddress("glCreateShader")))(GL_VERTEX_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &vsh,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);
	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fsh,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);

	GLchar n[15] = "time";
	GLint loc = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,n);

loop:
	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));

	const float t = 700.0+(float)(MMTime.u.sample >> 8)*0.05;
	((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc,t);

	glRecti(1,1,-1,-1);
	SwapBuffers(hDC);
	if (GetAsyncKeyState(VK_ESCAPE) || MMTime.u.sample >= MAX_SAMPLES)
		ExitProcess(0);
	goto loop;
}
