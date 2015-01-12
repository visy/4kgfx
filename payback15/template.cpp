/*
	4k Procedual Gfx template code.
	based on "chocolux" codes by alud.
*/

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <windows.h>

#include <iostream>

#include <GL/gl.h>
#include "glext.h"
#include <math.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "bass.h"
#include <stdio.h>
#include <stdarg.h>
#include "lib/sync.h"

#include "Effect.hpp"
#include "tiny_obj_loader.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

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

// orig by mankeli
const char *fsh =
 "varying vec3 v,c;"
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "uniform float warpfactor;"
 "void n(in vec3 v,in vec3 s,out vec4 f)"
 "{"
   "int k;"
   "vec3 i=v;"
   "for(int x=0;x<200;x++)"
     "{"
	 "i+=s;"
       "vec3 c=floor(i/vec3(y*.1)),m=mod(c,1.);"
       "m.x=mod(m.x+floor(y*10.),16.);"
       "float r=(m.z+m.y)*16.+m.x;"
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
		   "f.x=.4+c.y*-.1+cos(c.xyz*1000.05+y*100.0)*0.05;"
             "f.y=.4+c.y*-.05;"
             "f.z=.4+c.y*-.03;"
			 "f-=vec4(1000.0*abs(sin((cos(y*0.00001)*0.002)*0.1)*(cos(c.x)*0.7)));"
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
   "float f=1.1,i=sin(y*.432*f)*2.1,r=sin(y*1.21231*f+(c.x*warpfactor))-.9,m=sin(y*.73223*f)-.6;"
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
"	float d = 3.0;"
"	d = min(d,Q(uv-vec2(-2.0,0.5)));"
"	d = min(d,U(uv-vec2(-1.5,0.5)));"
"	d = min(d,A(uv-vec2(-1.0,0.5)));"
"	d = min(d,D(uv-vec2(-0.5,0.5)));"
"	d = min(d,T(uv-vec2( 0.0,0.5)));"
"	d = min(d,R(uv-vec2( 0.5,0.5)));"
"	d = min(d,I(uv-vec2( 1.0,0.5)));"
"	d = min(d,P(uv-vec2( 1.5,0.5)));"
"	float w = fwidth(d*3.0);"
"	gl_FragColor = vec4(vec3(abs((d*7.1+cos(uv.y*uv.x*0.07*y*1000.0)*0.1)+tan(y)*0.2)),1.0);"
"}";

// xTibor / 2014
const char *fsh3 =
 "varying vec3 v,c;"
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "vec2 C(float a, float b) {"
 "return vec2(float(a)/8.0,float(b)/8.0);"
 "}"
"bool split(vec2 p, vec2 sp, vec2 dir) {"
"return dot(p-sp, normalize(vec2(dir.y, -dir.x))) > 0.0;"
"}"
"bool circle(vec2 p, vec2 sp, float r) {"
"return length(p-sp) <= r;	"
"}"
"float frac(vec2 co) {"
"return fract(tan(y+dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453)*vec2(0.008,0.004);"
"}"
"bool LINE2(vec2 p, vec2 a, vec2 b, float w) {"
"vec2 dir = normalize(b - a);"
"vec2 nor = vec2(dir.y, -dir.x)*vec2(2.1)*abs(cos(b.y*0.1+y*0.1+b.x*0.1));"
"return split(p, a - nor * w / 2.0, dir) &&"
"split(p, a + nor * w / 2.0, -dir) &&"
"split(p, a - dir * w / 2.0, -nor) &&"
"split(p, b + dir * w / 2.0, nor);"
"}"
"bool LINE(vec2 p, vec2 a, vec2 b, float w) {"
"b -= a+b*(8.2*frac(a*vec2(y,y)));"
"p -= a+b*(8.2*frac(b*vec2(y,y)));"
"return length (p - clamp (dot (p, b) / dot (b, b), 0.0, 1.0) * b) < w * 1.0;"
"}"
"float beat() {"
"return 1.0;"
"}"
"bool digit(vec2 p, int d, float w) {"
"if (d == 0) return "
"LINE(p, C(1, 2), C(1, 6), w) ||"
"LINE(p, C(1, 6), C(2, 7), w) ||"
"LINE(p, C(2, 7), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(7, 6), w) ||"
"LINE(p, C(7, 6), C(7, 2), w) ||"
"LINE(p, C(7, 2), C(6, 1), w) ||"
"LINE(p, C(6, 1), C(2, 1), w) ||	"
"LINE(p, C(2, 1), C(1, 2), w) ||"
"LINE(p, C(1, 2), C(7, 6), w);"
"if (d == 1) return "
"LINE(p, C(1, 1), C(7, 1), w) ||"
"LINE(p, C(4, 1), C(4, 7), w) ||"
"LINE(p, C(1, 7), C(4, 7), w);	"
"if (d == 2) return "
"LINE(p, C(1, 6), C(2, 7), w) ||"
"LINE(p, C(2, 7), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(7, 6), w) ||						"
"LINE(p, C(7, 6), C(7, 5), w) ||"
"LINE(p, C(7, 5), C(6, 4), w) ||"
"LINE(p, C(6, 4), C(2, 4), w) ||			"
"LINE(p, C(2, 4), C(1, 3), w) ||"
"LINE(p, C(1, 3), C(1, 1), w) ||"
"LINE(p, C(1, 1), C(7, 1), w);"
"if (d == 3) return "
"LINE(p, C(1, 6), C(2, 7), w) ||"
"LINE(p, C(2, 7), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(7, 6), w) ||				"
"LINE(p, C(7, 6), C(7, 5), w) ||"
"LINE(p, C(7, 5), C(6, 4), w) ||"
"LINE(p, C(6, 4), C(7, 3), w) ||"
"LINE(p, C(6, 4), C(3, 4), w) ||"
"LINE(p, C(7, 3), C(7, 2), w) ||"
"LINE(p, C(7, 2), C(6, 1), w) ||"
"LINE(p, C(6, 1), C(2, 1), w) ||				"
"LINE(p, C(2, 1), C(1, 2), w);		"
"if (d == 4) return "
"LINE(p, C(1, 7), C(1, 5), w) ||"
"LINE(p, C(1, 5), C(2, 4), w) ||"
"LINE(p, C(2, 4), C(7, 4), w) ||"
"LINE(p, C(7, 1), C(7, 7), w);"
"if (d == 5) return "
"LINE(p, C(7, 7), C(1, 7), w) ||"
"LINE(p, C(1, 7), C(1, 4), w) ||"
"LINE(p, C(1, 4), C(6, 4), w) ||			"
"LINE(p, C(6, 4), C(7, 3), w) ||			"
"LINE(p, C(7, 3), C(7, 2), w) ||"
"LINE(p, C(7, 2), C(6, 1), w) ||"
"LINE(p, C(6, 1), C(2, 1), w) ||				"
"LINE(p, C(2, 1), C(1, 2), w);		"
"if (d == 6) return "
"LINE(p, C(7, 6), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(2, 7), w) ||			"
"LINE(p, C(2, 7), C(1, 6), w) ||			"
"LINE(p, C(1, 6), C(1, 2), w) ||			"
"LINE(p, C(1, 4), C(6, 4), w) ||			"
"LINE(p, C(6, 4), C(7, 3), w) ||			"
"LINE(p, C(7, 3), C(7, 2), w) ||"
"LINE(p, C(7, 2), C(6, 1), w) ||"
"LINE(p, C(6, 1), C(2, 1), w) ||			"
"LINE(p, C(2, 1), C(1, 2), w);			"
"if (d == 7) return "
"LINE(p, C(1, 7), C(7, 7), w) ||"
"LINE(p, C(7, 7), C(4, 4), w) ||				"
"LINE(p, C(4, 4), C(4, 1), w);"
"if (d == 8) return "
"LINE(p, C(2, 7), C(1, 6), w) ||"
"LINE(p, C(1, 6), C(1, 5), w) ||"
"LINE(p, C(1, 5), C(2, 4), w) ||"
"LINE(p, C(2, 4), C(1, 3), w) ||"
"LINE(p, C(1, 3), C(1, 2), w) ||"
"LINE(p, C(1, 2), C(2, 1), w) ||"
"LINE(p, C(2, 1), C(6, 1), w) ||"
"LINE(p, C(6, 1), C(7, 2), w) ||"
"LINE(p, C(7, 2), C(7, 3), w) ||"
"LINE(p, C(7, 3), C(6, 4), w) ||"
"LINE(p, C(6, 4), C(2, 4), w) ||"
"LINE(p, C(6, 4), C(7, 5), w) ||"
"LINE(p, C(7, 5), C(7, 6), w) ||"
"LINE(p, C(7, 6), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(2, 7), w);		"
"if (d == 9) return "
"LINE(p, C(1, 2), C(2, 1), w) ||"
"LINE(p, C(2, 1), C(6, 1), w) ||			"
"LINE(p, C(6, 1), C(7, 2), w) ||	"
"LINE(p, C(7, 2), C(7, 6), w) ||			"
"LINE(p, C(7, 4), C(2, 4), w) ||			"
"LINE(p, C(2, 4), C(1, 5), w) ||			"
"LINE(p, C(1, 5), C(1, 6), w) ||"
"LINE(p, C(1, 6), C(2, 7), w) ||"
"LINE(p, C(2, 7), C(6, 7), w) ||				"
"LINE(p, C(6, 7), C(7, 6), w);		"
"if (d == 10) return "
"LINE(p, C(1, 1), C(1, 5), w) ||"
"LINE(p, C(1, 5), C(3, 7), w) ||			"
"LINE(p, C(3, 7), C(5, 7), w) ||	"
"LINE(p, C(5, 7), C(7, 5), w) ||			"
"LINE(p, C(7, 5), C(7, 1), w) ||			"
"LINE(p, C(1, 4), C(7, 4), w);		"
"if (d == 11) return "
"LINE(p, C(1, 1), C(1, 7), w) ||"
"LINE(p, C(1, 7), C(6, 7), w) ||			"
"LINE(p, C(1, 4), C(6, 4), w) ||	"
"LINE(p, C(1, 1), C(6, 1), w) ||			"
"LINE(p, C(6, 7), C(7, 6), w) ||			"
"LINE(p, C(7, 6), C(7, 5), w) ||			"
"LINE(p, C(7, 5), C(6, 4), w) ||	"
"LINE(p, C(6, 4), C(7, 3), w) ||			"
"LINE(p, C(7, 3), C(7, 2), w) ||					"
"LINE(p, C(7, 2), C(6, 1), w);		"
"if (d == 12) return "
"LINE(p, C(7, 6), C(6, 7), w) ||"
"LINE(p, C(6, 7), C(2, 7), w) ||			"
"LINE(p, C(2, 7), C(1, 6), w) ||	"
"LINE(p, C(1, 6), C(1, 2), w) ||			"
"LINE(p, C(1, 2), C(2, 1), w) ||			"
"LINE(p, C(2, 1), C(6, 1), w) ||			"
"LINE(p, C(6, 1), C(7, 2), w);		"
"if (d == 13) return "
"LINE(p, C(1, 7), C(1, 1), w) ||"
"LINE(p, C(1, 1), C(5, 1), w) ||			"
"LINE(p, C(5, 1), C(7, 3), w) ||	"
"LINE(p, C(7, 3), C(7, 5), w) ||			"
"LINE(p, C(7, 5), C(5, 7), w) ||			"
"LINE(p, C(5, 7), C(1, 7), w);		"
"if (d == 14) return "
"LINE(p, C(1, 7), C(1, 1), w) ||"
"LINE(p, C(1, 7), C(7, 7), w) ||			"
"LINE(p, C(1, 4), C(5, 4), w) ||			"
"LINE(p, C(1, 1), C(7, 1), w);		"
"if (d == 15) return "
"LINE(p, C(1, 7), C(1, 1), w) ||"
"LINE(p, C(1, 7), C(7, 7), w) ||	"
"LINE(p, C(1, 4), C(5, 4), w);		"
"return false;"
"}"
"void main(void) {"
"vec2 uv = gl_FragCoord.xy / k.xy;	"
"uv.y *= k.y / k.x;"
"uv *= abs(2.0+cos(y*0.3)*0.5)*22.0 + beat() * 6.0;	"
"uv -= y * 1.2;	"
"uv /= dot(uv, vec2(sin(y * 0.34), sin(y * 0.53))) * 0.1 + 2.0;"
//"bool b = digit(fract(uv), int(mod(floor(uv.x+y) * floor(uv.y), 16.0)), 0.1);		"
"bool b = digit(fract(uv), int(mod(floor(uv.x+(y/uv.y)) * floor(uv.y), 16.0)), 0.1);		"
"if (b)	gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);"
"else gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);"
"if (mod(gl_FragCoord.y, 2.0) < 1.0) gl_FragColor *= 0.5;						"
"if ((mod(uv.x, 4.0)*cos(gl_FragCoord.x*0.01+y)) < 2.0) gl_FragColor *= 0.65;	"
"}";

const char *fshfb =
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "uniform sampler2D tex;"
 "uniform vec2 zoom;"
"float t = 0.0;"
"float frac(vec2 co) {"
"return fract(tan(y+dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453)*vec2(0.008,0.004);"
"}"
 "void main() {"
"	 t = y;"
"vec2 uv = ((gl_FragCoord.xy - k.x)*vec2(zoom.x,zoom.y)) / k.yy/ 2.0;"
"gl_FragColor = vec4(0.04*cos(t*0.001+gl_FragCoord.y*4.001));"
"float zc = 0.2+cos(t*0.005)*2.5;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.028*zc+frac(uv+t)))*0.0044299121055113265;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.024*zc+frac(uv+t)))*0.00895781211794;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.020*zc+frac(uv+t)))*0.0215963866053;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.016*zc+frac(uv+t)))*0.0443683338718;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.012*zc+frac(uv+t)))*0.0776744219933;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.008*zc+frac(uv+t)))*0.115876621105;"
"    gl_FragColor += texture2D(tex, uv+vec2(-0.004*zc+frac(uv+t)))*0.147308056121;"
"    gl_FragColor *= texture2D(tex, uv         )*4.259576912161+abs(cos(t*y*gl_FragCoord.y*0.0001+gl_FragCoord.x)*1.2*sin(t*0.01));"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.004*zc+frac(uv+t)))*0.147308056121;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.008*zc+frac(uv+t)))*0.115876621105;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.012*zc+frac(uv+t)))*0.0776744219933;"
"   gl_FragColor -= texture2D(tex,  uv+vec2(0.016*zc+frac(uv+t)))*0.0443683338718;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.020*zc+frac(uv+t)))*0.0215963866053;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.024*zc+frac(uv+t)))*0.00895781211794;"
"    gl_FragColor -= texture2D(tex, uv+vec2(0.028*zc+frac(uv+t)))*0.0044299121055113265;"
 "}";

const char *fshtex =
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "uniform sampler2D tex;"
 "uniform vec2 zoom;"
 "uniform float picalpha;"
 "void main() {"
"vec2 uv = (gl_FragCoord.xy / k.xy * 2.0)*vec2(zoom.x,zoom.y);"
"   vec4 color = texture2D(tex, vec2(uv.x,1.0-uv.y));"
 "     float alpha = picalpha;"
 "     if (alpha > 665.0) alpha = alpha-665.0;"
 "    color = vec4(color.rgb*vec3((2.0-alpha)),alpha);"
 "   if (color.r > 0.99) discard;"
 "  if (color.g <= -0.2) discard;"
 "   gl_FragColor = color;"
 "}";

const char *fshtex2 =
 "uniform float y;"
 "uniform float rot;"
 "uniform vec2 k;"
 "uniform sampler2D tex;"
 "uniform vec2 zoom;"
 "uniform float contrast;"
 "uniform float brightness;"
 "uniform float multiple;"
"float FXAA_REDUCE_MIN =  (1.0/ 128.0);"
"float FXAA_REDUCE_MUL =  (1.0 / 8.0);"
"float FXAA_SPAN_MAX =    8.0;"
"vec4 applyFXAA(vec2 fragCoord, sampler2D tex1)"
"{"
"    vec4 color1;"
"    vec4 color2;"
"    vec2 inverseVP = vec2(1.0 / k.x, 1.0 / k.y);"
"    vec3 rgbNW = texture2D(tex1, (fragCoord + vec2(-1.0, -1.0)) * inverseVP).xyz;"
"    vec3 rgbNE = texture2D(tex1, (fragCoord + vec2(1.0, -1.0)) * inverseVP).xyz;"
"    vec3 rgbSW = texture2D(tex1, (fragCoord + vec2(-1.0, 1.0)) * inverseVP).xyz;"
"    vec3 rgbSE = texture2D(tex1, (fragCoord + vec2(1.0, 1.0)) * inverseVP).xyz;"
"    vec3 rgbM  = texture2D(tex1, fragCoord  * inverseVP).xyz;"
"    vec3 luma = vec3(0.299, 0.587, 0.114);"
"    float lumaNW = dot(rgbNW, luma);"
"    float lumaNE = dot(rgbNE, luma);"
"    float lumaSW = dot(rgbSW, luma);"
"    float lumaSE = dot(rgbSE, luma);"
"    float lumaM  = dot(rgbM,  luma);"
"    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));"
"    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));"
"    vec2 dir;"
"    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));"
"    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));"
"    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);"
"    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);"
"    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseVP;"
"    vec3 rgbA = 0.5 * (texture2D(tex1, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz + texture2D(tex1, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);"
"    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture2D(tex1, fragCoord * inverseVP + dir * -0.5).xyz + texture2D(tex1, fragCoord * inverseVP + dir * 0.5).xyz);"
"    float lumaB = dot(rgbB, luma);"
"	 float inten = 0.04+abs(0.04*cos(fragCoord.y*lumaM*lumaB+lumaMin));"
//"    if ((lumaB < lumaMin) || (lumaB > lumaMax)) color = vec4(rgbA*vec3(1.0-(lumaMax*lumaB+abs(cos(fragCoord.y*1.1)))*0.08), 1.0);"
"    if ((lumaB < lumaMin) || (lumaB > lumaMax))  color2 = vec4(vec3(rgbA.r*1.0-(lumaMax*lumaM+abs(cos(fragCoord.y*1.1)))*inten,rgbA.g*1.0-(lumaMax*lumaM+abs(cos(fragCoord.y*1.1)))*inten,rgbA.b*1.0-(lumaM+abs(cos(fragCoord.y*1.1)))*inten), 1.0);"
"    else color2 = vec4(vec3(rgbB.r*1.0-(lumaMax*lumaB+abs(cos(fragCoord.y*1.1)))*inten,rgbB.g*1.0-(lumaMax*lumaB+abs(cos(fragCoord.y*1.1)))*inten,rgbB.b*1.0-(lumaB+abs(cos(fragCoord.y*1.1)))*inten), 1.0);"
"    if ((lumaB < lumaMin) || (lumaB > lumaMax)) color1 = vec4(rgbA, 1.0);"
"    else color1 = vec4(rgbB,1.0);"
"    return color2;"
"}"
" void main() {"
"	vec2 uv = (gl_FragCoord.xy / k.xy)*vec2(zoom.x,zoom.y);"
"   vec4 color = vec4(applyFXAA(gl_FragCoord.xy, tex).rgb,1.0);"
"   color *= vec4(multiple,multiple,multiple,1.0);"
"   color = mix( vec4(1.0,1.0,1.0,1.0),color,contrast);"
"   gl_FragColor =  vec4(color.r , color.g, color.b, 1.0);"
 "}";





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

GLuint color_tex, color_tex2;
GLuint fb, fb2;

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

int	SCREEN_WIDTH = 1920;
int	SCREEN_HEIGHT =	1080;

#define EFF_WIDTH  1080
#define EFF_HEIGHT 1080

std::string subtitles[64] = {
"*** accessing qpr instructional presentation archive ***",
"a once-in-a-lifetime-of-a-star opportunity.",
"did you know: you could already be a planet-owner!",
"in this short demonstration, we'll take a look -",
"at planet reconfiguration options and",
"examine essential features of\nthe latest must-have home kit.",
"haven't you always wished: \"Oh, if I just -",
"had the perfect kit ready for\nmy intelligent life experiments!\"",
"and don't you hate when your\nfledgling proto-star collapses on itself",
"before you even get solid continents\nforming on the outer crust?",
"this is why the\nquadtrip planet reconfigurator was dreamed up.",
"our tireless hordes of\ninsectspawn drones tap away in -",
"hive-centralized programming pods\nto bring you the latest -",
"in planet-grooming technology.",
"* $$$ £££ ¥¥¥ > we accept most stardard units of currency < ¥¥¥ £££ $$$ *",
"if you order the \nquadtrip planet reconfigurator -",
"during the next five\nstandard sixth moon orbits -",
"we'll throw in this eye-catching\nhomestead for your brood -",
"no extra charge!",
"this program is brought to you by\nthe eternally gnawing insect in your brain -",
"in co-operation with uncle rand(51223) corporation\nand the <attempting translation>oqsbxxy7qqq44 group.",
"subtitles made possible by a grant\nfrom the unending hivemother.",
"blessed be the hivemother.\nsweet flow her nectar.",
"it is now safe to turn off your neural implant."
};

int prevtext = 0;

int frames = 0;

const struct sync_track *rotatex, *rotatey, *rotatez;
const struct sync_track *logox, *logoy, *logoz, *logocolor;

std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;

static void RenderObject(int shapeindex, int colore, double row)
{
//  std::cout << "# of shapes    : " << shapes.size() << std::endl;
//  std::cout << "# of materials : " << materials.size() << std::endl;

	int i = shapeindex; // shape to render

	float scale = 0.4;

	glPushMatrix();
	glLoadIdentity();

	glTranslatef(float(sync_get_val(logox, row)),float(sync_get_val(logoy, row)),float(sync_get_val(logoz, row)));
	glScalef(0.1,0.15,0.1);
	glRotatef(float(sync_get_val(rotatez, row)),0.0,0.0,1.0);
	glRotatef(float(sync_get_val(rotatey, row)),0.0,1.0,0.0);
	glRotatef(float(sync_get_val(rotatex, row)),1.0,0.0,0.0);
	if (colore == 0) glColor3f(0.5*float(sync_get_val(logocolor, row)),0.5*float(sync_get_val(logocolor, row)),0.5*float(sync_get_val(logocolor, row)));
	else if (colore == 1) glColor3f(0.8*float(sync_get_val(logocolor, row)),0.8*float(sync_get_val(logocolor, row)),0.8*float(sync_get_val(logocolor, row)));

	glBegin(GL_TRIANGLES);
	const size_t face_count = shapes[i].mesh.indices.size() / 3;
    for (size_t f = 0; f < face_count; f++)
    {
        const int vi0 = shapes[i].mesh.indices[f*3+0];
        const int vi1 = shapes[i].mesh.indices[f*3+1];
        const int vi2 = shapes[i].mesh.indices[f*3+2];



        {
				std::vector<float>& mp = shapes[i].mesh.positions;
				glVertex3f(
					mp[vi0 * 3 + 0] * scale, 
					mp[vi0 * 3 + 1] * scale, 
					mp[vi0 * 3 + 2] * scale
				);
				glVertex3f(
					mp[vi1 * 3 + 0] * scale, 
					mp[vi1 * 3 + 1] * scale, 
					mp[vi1 * 3 + 2] * scale
				);
				glVertex3f(
					mp[vi2 * 3 + 0] * scale, 
					mp[vi2 * 3 + 1] * scale, 
					mp[vi2 * 3 + 2] * scale
				);
        }
/*
        if (!shapes[i].mesh.normals.empty())
        {
            std::vector<float>& mn = shapes[i].mesh.normals;
            trin0 = Imath::V3d(mn[vi0 * 3 + 0], mn[vi0 * 3 + 1], mn[vi0 * 3 + 2]);
            trin1 = Imath::V3d(mn[vi1 * 3 + 0], mn[vi1 * 3 + 1], mn[vi1 * 3 + 2]);
            trin2 = Imath::V3d(mn[vi2 * 3 + 0], mn[vi2 * 3 + 1], mn[vi2 * 3 + 2]);
        }

*/
    }
    
    glEnd();

	glPopMatrix();
}




static bool
LoadObj(
  const char* filename,
  const char* basepath = NULL)
{
  std::cout << "Loading " << filename << std::endl;

  std::string err = tinyobj::LoadObj(shapes, materials, filename, basepath);

  if (!err.empty()) {
    std::cerr << err << std::endl;
    return false;
  }

  return true;
}













void main()
{
	LoadObj("logo_draft1.obj", ".");
	
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	sf::RenderWindow  window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "quadtripping window", sf::Style::Fullscreen, settings);
	window.setVerticalSyncEnabled(true);

	settings = window.getSettings();

	std::cout << "Quadtrip demo system startup" << std::endl;
	std::cout << "depth bits:" << settings.depthBits << std::endl;
	std::cout << "stencil bits:" << settings.stencilBits << std::endl;
	std::cout << "antialiasing level:" << settings.antialiasingLevel << std::endl;
	std::cout << "version:" << settings.majorVersion << "." << settings.minorVersion << std::endl;

	sf::Font font;
	sf::Font font2;
	font.loadFromFile("gfx/CAMCORDER_INV.ttf");
	font2.loadFromFile("gfx/CAMCORDER_REG.ttf");
	sf::Text text("", font);
	sf::Text text2("", font2);
	text.setCharacterSize(40);
	text.setColor(sf::Color::Black);

	text2.setCharacterSize(40);
	text2.setColor(sf::Color::White);

	sf::FloatRect f = text.getLocalBounds();
	text.setOrigin(f.width/2,f.height/2);
	text.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT-50);

	sf::FloatRect f2 = text2.getLocalBounds();
	text2.setOrigin(f2.width/2,f2.height/2);
	text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT-50-6);

	ShowCursor(0);
	// Uncomment for 4klang InitSound();

	const GLuint p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint p2 = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint p3 = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint pfb = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint ptex = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	const GLuint ptex2 = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();

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
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fsh3,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p3,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fshfb,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(pfb,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fshtex,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(ptex,s);

	s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1, &fshtex2,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(ptex2,s);

	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p2);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p3);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(pfb);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(ptex);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(ptex2);
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

    glGenTextures(1, &color_tex2);
   glBindTexture(GL_TEXTURE_2D, color_tex2);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //NULL means reserve texture memory, but texels are undefined
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   //-------------------------
	glBindTexture(GL_TEXTURE_2D, 0);

   ((PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT"))(1, &fb2);
   ((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, fb2);
   ((PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT"))(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, color_tex2, 0);

   ((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, 0);

   status;
  status = ((PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT"))(GL_FRAMEBUFFER_EXT);
  switch(status)
  {
     case GL_FRAMEBUFFER_COMPLETE_EXT:
     break;
  default:
     return;
  }


  // load textures

  sf::Texture texture[12];
	texture[11].loadFromFile("gfx/quadtrip-jekku.png");
	texture[10].loadFromFile("gfx/title.png");
	texture[9].loadFromFile("gfx/credut.png");
	texture[8].loadFromFile("gfx/nopee.png");
	texture[7].loadFromFile("gfx/martini.png");
	texture[6].loadFromFile("gfx/luckytext.png");
	texture[5].loadFromFile("gfx/jakki.png");
	texture[3].loadFromFile("gfx/jekku.png");
	texture[4].loadFromFile("gfx/luckystrike.png");
	texture[1].loadFromFile("gfx/mallu.png");
	texture[2].loadFromFile("gfx/orska.png");
	texture[0].loadFromFile("gfx/pfizer.png");
	
	texture[0].setSmooth(true);
	texture[1].setSmooth(true);
	texture[2].setSmooth(true);
	texture[3].setSmooth(true);
	texture[4].setSmooth(true);
	texture[5].setSmooth(true);
	texture[6].setSmooth(true);
	texture[7].setSmooth(true);
	texture[8].setSmooth(true);
	texture[9].setSmooth(true);
	texture[10].setSmooth(true);
	texture[11].setSmooth(true);
	 
		
	HSTREAM stream;

	const struct sync_track *clear_r, *clear_g, *clear_b, *contrast, *brightness, *multiple;
	const struct sync_track *cam_rot, *cam_dist, *zoomx, *zoomy, *pic, *picalpha, *txtind, *warpfactor;

	/* init BASS */
	if (!BASS_Init(-1, 44100, 0, 0, 0))
		die("failed to init bass");
	stream = BASS_StreamCreateFile(false, "neuron-nycomp.mp3", 0, 0,
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
	cam_rot = sync_get_track(rocket, "cam.rot"),
	cam_dist = sync_get_track(rocket, "cam.dist");
	zoomx = sync_get_track(rocket, "zoomx");
	zoomy = sync_get_track(rocket, "zoomy");
	pic = sync_get_track(rocket, "pic");
	picalpha = sync_get_track(rocket, "picalpha");
	txtind = sync_get_track(rocket, "textindex");
	warpfactor = sync_get_track(rocket, "warpfactor");

	contrast = sync_get_track(rocket, "contrast");
	brightness = sync_get_track(rocket, "brightness");
	multiple = sync_get_track(rocket, "multiple");

	rotatex = sync_get_track(rocket, "rotatex");
	rotatey = sync_get_track(rocket, "rotatey");
	rotatez = sync_get_track(rocket, "rotatez");

	logox = sync_get_track(rocket, "logox");
	logoy = sync_get_track(rocket, "logoy");
	logoz = sync_get_track(rocket, "logoz");
	logocolor = sync_get_track(rocket, "logocolor");

	/* let's roll! */

	BASS_Start();
	BASS_ChannelPlay(stream, true);

	glEnable(GL_TEXTURE_2D);
	//bass_set_row(&stream,4872);

	sf::Clock clock;

while (window.isOpen())
{
	window.clear();

	((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, fb);
	glViewport( 0, 0, EFF_WIDTH, EFF_HEIGHT );
	



	// Uncomment for 4klang waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));

	//const float t = 700.0+(float)(MMTime.u.sample >> 8)*0.05;

	double row = bass_get_row(stream);
	if (row == 0 || (row >= 860 && row <= 889) || (row >= 3560 && row <= 3589)) glClear(GL_COLOR_BUFFER_BIT);
	
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pfb);
			
		glEnable(GL_TEXTURE_2D);
		  glBindTexture(GL_TEXTURE_2D, color_tex);


		((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
		loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IGLOBALTIME);
		loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_ROT);
		loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IRESOLUTION);
		GLint loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,"tex");
		GLint loc_zoom = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,"zoom");

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,row);
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,EFF_WIDTH,EFF_HEIGHT);
		((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_zoom,float(sync_get_val(zoomx, row)),float(sync_get_val(zoomy, row)));
		glRecti(1,1,-1,-1);


#ifndef SYNC_PLAYER
		if (sync_update(rocket, (int)floor(row), &bass_cb, (void *)&stream))
			sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif
			
//	sync_update(rocket, (int)floor(row), &bass_cb, (void *)&stream);

	frames++;


	if (row < 1200 || row > 1360) {
		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);
		GLint loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IGLOBALTIME);
		GLint loc_warpfactor = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,"warpfactor");
		GLint loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_ROT);
		GLint loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p,VAR_IRESOLUTION);

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,float(sync_get_val(cam_dist, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_warpfactor,float(sync_get_val(warpfactor, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,EFF_WIDTH,EFF_HEIGHT);
		glRecti(1,1,-1,-1);

//		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		sf::Texture::bind(&texture[0]);
		//glBindTexture(GL_TEXTURE_2D, color_tex);
		((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
				
	//	glDisable(GL_BLEND);


	}



	((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, fb2);
	glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	glClear(GL_COLOR_BUFFER_BIT);

	if (row >= 1200 && row <= 1360) {

		// quadtrip logo

		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p2);

		GLint loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_IGLOBALTIME);
		GLint loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_ROT);
		GLint loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(p2,VAR_IRESOLUTION);

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,float(sync_get_val(cam_dist, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);
		glRecti(1,1,-1,-1);
	} else {

		// framebuffer effect

		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pfb);
			
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, color_tex);
		((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
		loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IGLOBALTIME);
		loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_ROT);
		loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,VAR_IRESOLUTION);
		loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(pfb,"tex");

		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,row);
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
		((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);
		((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
		glRecti(1,1,-1,-1);



		int picindex = (int)(sync_get_val(pic, row));
		if (picindex != 0 || picindex != 9) {
			glEnable(GL_BLEND);

			if (float(sync_get_val(picalpha, row)) < 665.0) {
				glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
				((PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation"))(GL_MIN);
			}
			else {
				glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
				((PFNGLBLENDEQUATIONPROC)wglGetProcAddress("glBlendEquation"))(GL_FUNC_ADD);
			}

			sf::Texture::bind(&texture[picindex-1]);
			((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(ptex);
			 loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,VAR_IGLOBALTIME);
			 loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,VAR_ROT);
			 loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,VAR_IRESOLUTION);
			loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,"tex");
			GLint loc_zoom = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,"zoom");
			GLint loc_picalpha = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex,"picalpha");

			((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_time,float(sync_get_val(cam_dist, row)));
			((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_rot,float(sync_get_val(cam_rot, row)));
			((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
			((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_zoom,0.5,0.5);
			((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_picalpha,float(sync_get_val(picalpha, row)));

			glRecti(1,1,-1,-1);
			glDisable(GL_BLEND);
		}
	}

	glFinish();


	BASS_Update(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Uncomment for klang if (GetAsyncKeyState(VK_ESCAPE) || MMTime.u.sample >= MAX_SAMPLES) ExitProcess(0);
	if (GetAsyncKeyState(VK_ESCAPE)) goto quit;

    sf::Event event;
    while (window.pollEvent(event))
    {
		if (event.type == sf::Event::Closed) {
			goto quit;
        }
		else if (event.type == sf::Event::Resized) {
			// adjust the viewport when the window is resized
			glViewport(0, 0, event.size.width, event.size.height);
			SCREEN_WIDTH = event.size.width;
			SCREEN_HEIGHT= event.size.height;
		}
    }


	window.pushGLStates();

	int textindex = (int)(sync_get_val(txtind, row));

	if (prevtext != textindex && textindex > 0 && textindex < sizeof( subtitles ) / sizeof( subtitles[ 0 ])) {
		text.setString(subtitles[textindex-1]);
		
		text2.setString(subtitles[textindex-1]);
		
		sf::FloatRect f = text.getLocalBounds();
		text.setOrigin(f.width/2,f.height/2);
		text.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT-50);

		sf::FloatRect f2 = text2.getLocalBounds();
		text2.setOrigin(f2.width/2,f2.height/2);
		text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT-50-6);


	} else if (prevtext != textindex && textindex == 0) {
		text.setString("");
		text2.setString("");
	}

	if (textindex == 15 || textindex == 24) {
		sf::FloatRect f = text.getLocalBounds();
		text.setOrigin(f.width/2,f.height/2);
		text.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

		sf::FloatRect f2 = text2.getLocalBounds();
		text2.setOrigin(f2.width/2,f2.height/2);
		text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT/2-6);
		window.draw(text2);
		text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT/2-2);
		window.draw(text2);
	
		window.draw(text);
	} else {
		sf::FloatRect f2 = text2.getLocalBounds();
		text2.setOrigin(f2.width/2,f2.height/2);
		text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT-50-6);
		window.draw(text2);
		text2.setPosition(SCREEN_WIDTH/2-2, SCREEN_HEIGHT-50-2);
		window.draw(text2);
	
		window.draw(text);
	}

	prevtext = textindex;

	window.popGLStates();



	((PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT"))(GL_FRAMEBUFFER_EXT, 0);

		glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,color_tex2);
			((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(ptex2);
			 loc_time = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,VAR_IGLOBALTIME);
			 loc_rot = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,VAR_ROT);
			 loc_resolution = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,VAR_IRESOLUTION);
			loc_tex = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,"tex");
			 loc_zoom = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,"zoom");

			 GLuint loc_contrast = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,"contrast");
			 GLuint loc_brightness = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,"brightness");
			 GLuint loc_multiple = ((PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation"))(ptex2,"multiple");

			((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_resolution,SCREEN_WIDTH,SCREEN_HEIGHT);
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(loc_tex,0);
			((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(loc_zoom,1.0,1.0);
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_contrast,float(sync_get_val(contrast, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_brightness,float(sync_get_val(brightness, row)));
		((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(loc_multiple,float(sync_get_val(multiple, row)));

			glRecti(1,1,-1,-1);

			glFinish();


	window.pushGLStates();
		  RenderObject(0,0, row);
	window.popGLStates();
	window.pushGLStates();
		  RenderObject(1,0, row);
	window.popGLStates();
	window.pushGLStates();
		  RenderObject(2,1, row);
	window.popGLStates();


	window.display();


}

quit:
	window.close();			

#ifndef SYNC_PLAYER
	sync_save_tracks(rocket);
#endif

	sync_destroy_device(rocket);

	BASS_StreamFree(stream);
	BASS_Free();

	ExitProcess(0);
}
