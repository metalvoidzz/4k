#version 130




uniform float un[6];


float u_time = un[0];
float u_alpha = un[1];
float u_x = un[2];
float u_y = un[3];
float u_z = un[4];
// account for .000001 difference
float u_scene = round(un[5]);


#define WIDTH 1280
#define HEIGHT 720

#define MAX_ITER 1600
float MAX_DIST;
#define AMBIENCE 300


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

vec2 fOpU(in vec2 obj0, in vec2 obj1)
{
	if (obj0.x < obj1.x)
		return obj0;
	else
		return obj1;
}

float noise(in vec3 p)
{
    const vec3 s = vec3(7, 157, 113);
	vec3 ip = floor(p); // Unique unit cell ID.
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
	p -= ip; // Cell's fractional component.
    p = p*p*(3. - 2.*p);
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
    h.xy = mix(h.xz, h.yw, p.y);
    return mix(h.x, h.y, p.z); // Range: [0, 1].
	
}

mat3 m = mat3( 0.00,  1.60,  1.20, -1.60,  0.72, -0.96, -1.20, -0.96,  1.28 );

float fbm( vec3 p )
{
  float f = 0.5000*noise( p ); p = m*p*1.1;
  f += 0.2500*noise( p ); p = m*p*1.2;
  f += 0.1666*noise( p ); p = m*p;
  f += 0.0834*noise( p );
  return f;
}

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, 0, s),
        vec3(0, 1, 0),
        vec3(-s, 0, c)
    );
}

mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(c, -s, 0),
        vec3(s, c, 0),
        vec3(0, 0, 1)
    );
}



float cubeSDF(vec3 p, vec3 b)
{
	return max3(abs(p) - b);
}

float sphereSDF(vec3 p, float r)
{
	return length(p) - r;
}

vec3 hash33(vec3 p)
{
    p = fract(p * vec3(443.8975,397.2973, 491.1871));
    p += dot(p.zxy, p.yxz+19.27);
    return fract(vec3(p.x * p.y, p.z*p.x, p.y*p.z));
}

vec3 stars(vec3 p)
{
    vec3 c = vec3(0.);
    float res = 70000;
    
	for (float i=0.;i<3;i++)
    {
        vec3 q = fract(p*(.15*res))-0.5;
        vec3 id = floor(p*(.15*res));
        vec2 rn = hash33(id).xy;
        float c2 = 1.-smoothstep(0.,.6,length(q));
        c2 *= step(rn.x,.0005+i*i*0.001);
        c += c2*(mix(vec3(1.0,0.49,0.1),vec3(0.75,0.9,1.),rn.y)*0.25+0.75);
        p *= 1.4;
    }
	return c*c*.7;
}

vec3 gp;
float gi_x, gi_y, gi_z;
vec2 map(vec3 p)
{
	// Rotation
	
	if(u_scene == 4)
		p *= rotateY(radians(90.0));
	
	gp = p;
	
	vec2 s;
	
	// Floating reflective-sphere-thingy
	
	vec3 p1 = p;
	pMod1(p1.x, 24);
	pMod1(p1.z, 24);
	pMod1(p1.y, 24);
	
	p1 = vec3(p1.x, p1.y+1, p1.z);
	
	vec2 fl = fOpU(vec2(sphereSDF(p1, 3.0), 3.0), vec2(cubeSDF(p1, vec3(6, 0.25, 6)), 4.0));
	
	if(u_scene == 0 || u_scene == 2 || u_scene == 4)
	{
		MAX_DIST = 300;
		
		gi_z = pMod1(p.z, 18);
		
		// Ground plane
		
		vec2 gp = fOpU(
			vec2(
				opSub(
					cubeSDF(vec3(p.x, p.y, p.z), vec3(8, 02, 6)),
					cubeSDF(vec3(p.x, p.y, p.z), vec3(10.2, 0.2, 8))
				),
				1.0
			),
			vec2(cubeSDF(vec3(p.x, p.y+0.1, p.z), vec3(10, 0.2, 7)), 2.0)
		);
		
		// Floor / Ceiling
		vec2 bounds = fOpU(
			//vec2(cubeSDF(vec3(p.x, p.y, p.z), vec3(10, 0.1, 8)), 1.0),
			gp,
			vec2(cubeSDF(vec3(p.x, p.y-8, p.z), vec3(10.2, 0.2, 12)), 1.0)
		);

		// Wall material
		
		bounds = fOpU(
			bounds,
			// Walls
			fOpU(
				vec2(cubeSDF(vec3(p.x+10, p.y-4, p.z), vec3(0.2, 4, 7.99)), 0),
				vec2(cubeSDF(vec3(p.x-10, p.y-4, p.z), vec3(0.2, 4, 7.99)), 0)
			)
		);
		
		// "Bridge"
		
		vec2 brid = fOpU(
			vec2(
				cubeSDF(vec3(p.x, p.y, p.z-16), vec3(5.2, 0.2, 8)),
				1.0
			),
			vec2(
				cubeSDF(vec3(p.x, p.y, p.z+16), vec3(5.2, 0.2, 8)),
				1.0
			)
		);
		
		gi_x = pMod1(p.x, 20);
		gi_y = pMod1(p.y, 10);
		
		s = fOpU(brid, bounds);
		
		// The part where the scenes merge together
		
		if(u_scene == 5) s = fOpU(s, fl);
	}else if(u_scene == 1 || u_scene == 3 || u_scene == 5) {
		MAX_DIST = 600;
		s = fl;
	}

	
	return s;
}

vec2 trace(vec3 ro, vec3 rd)
{
	float depth = 0;
	vec2 dist;
    for (int i = 0; i < MAX_ITER; i++)
	{
        dist = map(ro + depth * rd);
        if (dist.x < 0.0001) {
			return vec2(depth, dist.y);
        }
        depth += dist.x*.5;
        if (depth >= MAX_DIST) {
            return vec2(MAX_DIST, 0);
        }
    }
	return vec2(MAX_DIST, 0);
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

vec3 getNormal( in vec3 p ){
    return normalize(vec3(
        map(vec3(p.x + 0.0001, p.y, p.z)).x - map(vec3(p.x - 0.0001, p.y, p.z)).x,
        map(vec3(p.x, p.y + 0.0001, p.z)).x - map(vec3(p.x, p.y - 0.0001, p.z)).x,
        map(vec3(p.x, p.y, p.z  + 0.0001)).x - map(vec3(p.x, p.y, p.z - 0.0001)).x
    ));
}

vec3 doColor(in vec3 sp, in vec3 rd, in vec3 sn, in vec3 lp, vec3 base)
{
	vec3 ld = lp-sp;
	float distlpsp = max(length(ld), 0.001);
	ld /= distlpsp;
	vec3 color;
	float ao = calculateAO(sp, sn);
	vec3 sunDir = vec3(0.0, 0.0, 0.0);
    float ind = clamp( dot( sn, normalize(sunDir*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
	vec3 li = clamp( 0.5 + 0.5 * sn.y, 0.0, 1.0 )*vec3(0.16,0.20,0.28)*ao + ind*vec3(0.40,0.28,0.20)*ao;
	color = li * base + (pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.)*color*.4);
	color *= pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.)*max( dot(sn, ld), 0.0)*min(1./(distlpsp), 1)*AMBIENCE;
	
	return color;
}

void main()
{
	vec2 uv = (gl_FragCoord.xy - vec2(WIDTH, HEIGHT)*.5) / HEIGHT;
    vec3 rd = normalize(vec3(uv, 1.0));
    vec3 ro = vec3(u_x, u_y, u_z);
	vec3 lp = vec3(ro.x, ro.y, ro.z);
	
    vec2 t = trace(ro, rd);
	
	if (t.x > MAX_DIST - 0.0001)
	{
		gl_FragColor = vec4(stars(rd)-u_alpha, 0.0);
		return;
	}

	ro += rd*t.x;
    vec3 sn = getNormal(ro);
	
	vec3 color;
	float shiny;
	if(t.y == 0.0)
	{
		// Wall material
		
		//vec3 c1 = vec3(cos(sin(u_time))*2, 0.0, clamp(sin(u_time)*.2, 0.0, 1.0));
		vec3 c1 = vec3(0.4, 0.1, 0.2);
		vec3 c2 = vec3(0.6);
		
		color = mix(
			c1,
			c2,
			step(
				1.,
				abs(gp.y + 0.75 * -5.0
				+ sin(gp.z * 0.20 + (u_scene == 2 || u_scene == 4 ? u_time : 0) * .8) - fbm(gp * 1.5))
			)
		);
		
		shiny = 0.1;
	}else if(t.y == 1.0) {
		color = vec3(0.2, 0.2, 0.2);
		shiny = 0.6;
	}else if(t.y == 2.0) {
		color = vec3(0.1, 0.3, 0.2);
		shiny = 0.09;
	}else if(t.y == 3.0) {
		color = vec3(0.2, 0.1, 0.5);
		shiny = 2.5;
	}else if(t.y == 4.0) {
		color = vec3(0.01);
		shiny = 1;
	}
	
	float fog = smoothstep(0., 1.1, t.x/MAX_DIST);
    vec3 sceneColor = doColor(ro, rd, sn, lp, color);
	
	// Reflection //
    
    rd = reflect(rd, sn);
    t = trace(ro +  rd*.01, rd);
    ro += rd*t.x;
    sn = getNormal(ro);
    sceneColor += doColor(ro, rd, sn, lp, color)*shiny;
   
	// Post processing //
  
	// Scanlines
	sceneColor -= sin(uv.y*1100)*0.007;
	// Fog
    sceneColor = mix(sceneColor, vec3(0), fog);
	// Grain
	sceneColor = mix(sceneColor, vec3(fract(sin(dot(uv.xy, vec2(17.0,180.)+u_time))* 2500)), 0.004);
	
	gl_FragColor = vec4(sqrt(clamp(sceneColor, 0.0, 1.0))-u_alpha, 1.0);
}
