varying vec3 org,dir;
float iGlobalTime = 2.0;
vec2 iResolution = vec2(1280.0,720.0);

void trace(in vec3 ro, in vec3 rd, out vec4 outcol)
{
    int i;
    vec3 rp = ro;

      //  outcol = vec4(0.0);
  
    //outcol = textureCube(iChannel1, rd-cos(iGlobalTime*2.0)*iGlobalTime*0.01);

    for (int i = 0; i < 400; i+=1)
    {
        rp += rd * 1.0;
        
        vec3 cell = floor(rp / vec3(iGlobalTime*0.1));
        
        vec3 tcell = mod(cell, 1.0);
		tcell.x = mod(tcell.x + floor(iGlobalTime * 10.0), 16.0);
        float idx = ((tcell.z * 1.0 + tcell.y) * 16.0) + tcell.x;
        vec2 idxuv = vec2(floor(idx/512.0), mod(idx, 512.0)) / vec2(512.0);
        //vec4 celltex = texture2D(iChannel0, idxuv);
	vec4 celltex = vec4(1.0,1.0,1.0,1.0);
        float val = length(celltex);
        
        float fy = 0.0;
        fy += sin(cell.x * 0.5523);
        fy += sin(cell.x * 0.23 + iGlobalTime * 5.0);
        fy += sin(cell.x * 0.0152 + iGlobalTime * 3.1);
        fy += sin(cell.z * 0.7632);
        fy += sin(cell.z * 0.25234);
        fy += sin(cell.z * 0.45234);
            
        fy = fy * 2.9 - 6.0;
        if (cell.x == 0.0 && cell.y == 0.0 && cell.z == 0.0)
        {
            outcol = vec4(1.0);
            break;
        }
        else
        if (cell.y < fy)
        {
             outcol.r = 0.4 + cell.y * -0.1;
             outcol.g = 0.4 + cell.y * -0.05;
             outcol.b = 0.4 + cell.y * -0.03;
             break;
        }
        else
        if (val > 1.2 && length(cell.yz) < 10.0)
        {
            outcol = celltex;
            break;
        }
    }
//    outcol.r = length(rp-ro) * 0.1;
}

void main(void)
{
    vec3 p = org;

    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
    uv.xy -= vec2(0.5);
//    uv.xy *= 0.1;
//    uv.xy -= (iMouse.xy / iResolution.xy) - vec2(0.5);
    uv.xy += vec2(0.5);

    float speed = 1.1;
    float ang = sin(iGlobalTime * 0.432 * speed) * 2.1;
    float ang2 = sin(iGlobalTime * 1.21231 * speed) - 0.9;
    float ang3 = sin(iGlobalTime * 0.73223 * speed) - 0.6;

//    ang = 1.0;
    
    mat3 rotinv = mat3(
        cos(ang), 0.0, -sin(ang),
        0.0,      1.0, 0.0,
        sin(ang), 0.0, cos(ang));

    mat3 rotinv2 = mat3(
          1.0, 0.0, 0.0,
          0.0, cos(ang2), -sin(ang2),
          0.0, sin(ang2), cos(ang2));

        mat3 rotinv3 = mat3(
          cos(ang3), -sin(ang3), 0.0,
          sin(ang3), cos(ang3), 0.0,
          0.0, 0.0, 1.0);


    mat3 scale = mat3(
    1.0, 0.0, 0.0,
    0.0, 0.3, 0.0,
    0.0, 0.0, 1.0);
 
    rotinv = mat3(1.0) * rotinv2 * rotinv;// * rotinv3 * rotinv;

    mat3 rot = mat3(
        rotinv[0][0],rotinv[1][0],rotinv[2][0],
        rotinv[0][1],rotinv[1][1],rotinv[2][1],
        rotinv[0][2],rotinv[1][2],rotinv[2][2]);

    
    
    vec3 ro = vec3(0.0 + sin(iGlobalTime*1.9) * 0.5, 0.3, -14.5 + sin(iGlobalTime) * 10.0);
    vec3 rd = vec3((uv.x - 0.5) * 1.6, uv.y - 0.5, 1.0);

    rd = normalize(rd);

  //  ro += rd * ((0.5)/rd.z);
    
    ro = rotinv * ro;
    rd = rotinv * rd;
    
    ro.y += 10.0;
    
    vec4 outcol;
    trace(ro, rd, outcol);

#if 0
    gl_FragColor = outcol;
#else
    vec4 vignet = vec4(pow(length(uv - vec2(0.5)), 5.0)) * vec4(1.0);
    
    vignet = vignet * 18.4 * (0.02 + vignet);
    vignet *= vec4(0.3, 0.6, 1.0, 1.0);
    gl_FragColor = clamp(outcol * vec4(1.0) * 1.0, vec4(0.0), vec4(1.0)) - vignet;
#endif
}
