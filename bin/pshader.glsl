#version 130




uniform float un[6];


float u_time = un[0];
float u_alpha = un[1];
float u_x = un[2];
float u_y = un[3];
float u_z = un[4];
float u_scene = un[5];


#define WIDTH 1920
#define HEIGHT 1080

#define MAX_ITER 1600
#define MAX_DIST 300
#define AMBIENCE 250

#define PI 3.1415926


float max3(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float opSub( float d1, float d2 ) {
    return max(-d1,d2);
}

float pMod1(inout float p, float size) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	return c;
}

void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

vec2 fOpU(in vec2 obj0, in vec2 obj1)
{
	if (obj0.x < obj1.x)
		return obj0;
	else
		return obj1;
}

float pModInterval1(inout float p, float size, float start, float stop) {
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p+halfsize, size) - halfsize;
	if (c > stop) {
		p += size*(c - stop);
		c = stop;
	}
	if (c <start) {
		p += size*(c - start);
		c = start;
	}
	return c;
}

float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p)
{
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

float fbm(vec3 x, int octaves)
{
	float v = 0.0;
	float a = 0.5;
	vec3 shift = vec3(100);
	for (int i = 0; i < octaves; ++i) {
		v += a * noise(x);
		x = x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}



float sphereSDF(vec3 p, float s)
{
	return length(p)-s;
}

float torusSDF(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz)-t.x,p.y);
	return length(q)-t.y;
}

float cubeSDF(vec3 p, vec3 b)
{
	return max3(abs(p) - b);
}

float roundCubeSDF(vec3 p, vec3 b, float r)
{
  return length(max(abs(p)-b,0.0))-r;
}

vec3 gp;
float gi_x, gi_y, gi_z;
vec2 map(vec3 p)
{
	gp = p;
	gi_z = pMod1(p.z, 17);
	
	// Floor / Ceiling
	vec2 bounds = fOpU(
		vec2(cubeSDF(vec3(p.x, p.y, p.z), vec3(10, 0.1, 8.5)), 1.0),
		vec2(cubeSDF(vec3(p.x, p.y-8, p.z), vec3(10, 0.1, 10)), 1.0)
	);

	// Wall material
	//float wm = mod(gi_z, 2.0) == 0 ? 0.0 : 2.0;
	float wm = 0.0;
	
	bounds = fOpU(
		fOpU(
			bounds,
			// Cubes on the side
			fOpU(
				vec2(cubeSDF(vec3(p.x+10.2, p.y-0.1, p.z), vec3(0.2, 8, 10.0)), 3.0),
				vec2(cubeSDF(vec3(p.x-10.2, p.y-0.1, p.z), vec3(0.2, 8, 10.0)), 3.0)
			)

		),
		//bounds,
		// Walls
		fOpU(
			vec2(cubeSDF(vec3(p.x+10, p.y-4, p.z), vec3(0.1, 4.1, 8.0)), wm),
			vec2(cubeSDF(vec3(p.x-10, p.y-4, p.z), vec3(0.1, 4.1, 8.0)), wm)
		)
	);
	
	vec2 s = bounds;
	
	return s;
}

vec2 trace(vec3 ro, vec3 rd)
{
	float depth = 0;
    for (int i = 0; i < MAX_ITER; i++) {
        vec2 dist = map(ro + depth * rd);
        if (dist.x < 0.0001) {
			return vec2(depth, dist.y);
        }
        depth += dist.x*.5;
        if (depth >= MAX_DIST) {
            return vec2(MAX_DIST, 0.0);
        }
    }
    return vec2(MAX_DIST, 0.0);
}

float calculateAO(vec3 p, vec3 n)
{
    const float AO_SAMPLES = 5.0;
    float r = 0.0, w = 1.0, d;
    
    for (float i=1.0; i<AO_SAMPLES+1.1; i++) {
        d = i/AO_SAMPLES;
        r += w*(d - map(p + n*d).x);
        w *= 0.5;
    }
    
    return 1.0-clamp(r,0.0,1.0);
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float maxt, float k )
{
    float res = 1.0;
    for( float t=mint; t < maxt; )
    {
        float h = map(ro + rd*t).x;
        if( h<0.001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h;
    }
    return res;
}

vec3 getNormal( in vec3 p ){
    return normalize(vec3(
        map(vec3(p.x + 0.0001, p.y, p.z)).x - map(vec3(p.x - 0.0001, p.y, p.z)).x,
        map(vec3(p.x, p.y + 0.0001, p.z)).x - map(vec3(p.x, p.y - 0.0001, p.z)).x,
        map(vec3(p.x, p.y, p.z  + 0.0001)).x - map(vec3(p.x, p.y, p.z - 0.0001)).x
    ));
}

vec3 doColor(in vec3 sp, in vec3 rd, in vec3 sn, in vec3 lp, vec3 base)
{
	/*vec3 ld = lp-sp;
    float lDist = max(length(ld), 0.001);
    ld /= lDist;
    
    float atten = 1. / (1.0 + lDist*0.2 + lDist*lDist*0.1);
    float diff = max(dot(sn, ld), 0.);
    float spec = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.0);
    
	vec3 sceneCol = (base*(diff + 0.15) + vec3(1., .6, .2)*spec*2.) * atten;

    return sceneCol*AMBIENCE*.25;*/
	
	vec3 ld = lp-sp;
	float distlpsp = max(length(ld), 0.001);
	ld /= distlpsp;
	
	vec3 color = base;

	// Attenuation
	float atten = min(1./(distlpsp), 1);
	// Diffuse lighting
	float diff = max( dot(sn, ld), 0.0);
	// Specular lighting
	float spec = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.);
	// Sky
	float sky = clamp( 0.5 + 0.5 * sn.y, 0.0, 1.0 );
	// Ambient occlusion
	float ao = calculateAO(sp, sn);
	// Shadow
	float shadow = softshadow(sp, ld, 0.002, 12, 128);
	// Fresnel
	float fre = pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.);
	// Sun
	vec3 sunDir = vec3(0.0, 0.0, 0.0);
    float sun = clamp( dot( sn, sunDir ), 0.0, 1.0 );
    float ind = clamp( dot( sn, normalize(sunDir*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
	// Final color
	vec3 li = sky*vec3(0.16,0.20,0.28)*ao + ind*vec3(0.40,0.28,0.20)*ao;
	color = li * base + (spec*color*.4);
	color *= fre*diff*atten*AMBIENCE;
	
	return color;
}

void main()
{
	vec2 uv = (gl_FragCoord.xy - vec2(WIDTH, HEIGHT)*.5) / HEIGHT;
    vec3 rd = normalize(vec3(uv, 1.0));
    vec3 ro = vec3(u_x, u_y, u_z);
    //vec3 lp = vec3(sin(u_time), cos(sin(u_time)), cos(sin(u_time))*.5);
	vec3 lp = vec3(ro.x, ro.y+20, ro.z);
	
    vec2 t = trace(ro, rd);

	vec3 color;
	float shiny;
	if(t.y == 0.0)
	{
		// Wall material
		
		//vec3 c1 = vec3(cos(sin(u_time)), 0.1, clamp(sin(u_time)*.2, 0.0, 1.0));
		vec3 c1 = vec3(0.6, 0.1, 0.2);
		vec3 c2 = vec3(1.0, 1.0, 1.0);
		
		color = mix(
			c1,
			c2,
			step(
				1.,
				abs(gp.y + 0.75 * -5.0
				+ sin(gp.z * 0.20 + u_time * .8) - fbm(gp * 1.05, 6) * 2)
			)
		);
		
		shiny = 0.04;
	}else if(t.y == 1.0) {
		color = vec3(0.2, 0.2, 0.2);
		shiny = 1.4;
	}else if(t.y == 2.0) {
		color = vec3(1.0, 1.0, 1.0);
		shiny = 0.04;
	}else if(t.y == 3.0) {
		color = vec3(0.2, 0.2, 0.2);
		shiny = 0.02;
	}
	
	float fog = smoothstep(0., .95, t.x/MAX_DIST);
    ro += rd*t.x;
    vec3 sn = getNormal(ro);
    vec3 sceneColor = doColor(ro, rd, sn, lp, color);
	
	// Reflection //
    
    rd = reflect(rd, sn);
    t.x = trace(ro +  rd*.01, rd).x;
    ro += rd*t.x;
    sn = getNormal(ro);
    sceneColor += doColor(ro, rd, sn, lp, color)*shiny;
   
	// Final color //
   
    sceneColor = mix(sceneColor, vec3(0), fog); 
    
	gl_FragColor = vec4(sqrt(clamp(sceneColor, 0.0, 1.0))-u_alpha, 1.0);
}
