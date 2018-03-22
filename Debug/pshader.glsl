#version 130




uniform float un[6];


float u_time = un[0];
float u_alpha = un[1];
float u_x = un[2];
float u_y = un[3];
float u_z = un[4];
float u_scene = un[5];


#define WIDTH 1366
#define HEIGHT 768

#define MAX_ITER 1200
#define MAX_DIST 400
#define AMBIENCE 10

#define PI 3.141592




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

vec3 hash33(vec3 p)
{
    p = fract(p * vec3(443.8975,397.2973, 491.1871));
    p += dot(p.zxy, p.yxz+19.27);
    return fract(vec3(p.x * p.y, p.z*p.x, p.y*p.z));
}

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

vec3 tex3D( sampler2D tex, in vec3 p, in vec3 n ){
  
    n = max((abs(n) - 0.2)*7., 0.001);
    n /= (n.x + n.y + n.z );  
    
	return (texture(tex, p.yz)*n.x + texture(tex, p.zx)*n.y + texture(tex, p.xy)*n.z).xyz;
}

float getGrey(vec3 p) {
	return p.x*0.299 + p.y*0.587 + p.z*0.114;
}

vec3 doBumpMap( sampler2D tex, in vec3 p, in vec3 nor, float bumpfactor){
   
    const float eps = 0.001;
    float ref = getGrey(tex3D(tex,  p , nor));
    vec3 grad = vec3( getGrey(tex3D(tex, vec3(p.x-eps, p.y, p.z), nor))-ref,
                      getGrey(tex3D(tex, vec3(p.x, p.y-eps, p.z), nor))-ref,
                      getGrey(tex3D(tex, vec3(p.x, p.y, p.z-eps), nor))-ref )/eps;
             
    grad -= nor*dot(nor, grad);
    
    return normalize( nor + grad*bumpfactor );
	
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

float sceneSDF(vec3 p)
{
	pMod1(p.x, 20.0);
	pMod1(p.z, 20.0);
	
	float ground = cubeSDF(vec3(p.x, p.y+2.25, p.z), vec3(8,.25,9));

	float w1 = cubeSDF(vec3(p.x+2, p.y, p.z+1), vec3(0.4, 2, 0.4));
	float w2 = cubeSDF(vec3(p.x-2, p.y, p.z-1), vec3(0.4, 2, 0.4));
	
	
	// Solid wall
	float a1 = cubeSDF(vec3(p.x, p.y-3, p.z+9), vec3(8,7,.25));
	float a2 = cubeSDF(vec3(p.x, p.y-3, p.z-9), vec3(8,7,.25));
	
	float a = min(a1, a2);
	
	// Wall cut-out
	float wall = min(w1, w2);
	
	float s = min(wall, ground);	
	
	return s;
}





float raymarch(vec3 ro, vec3 rd, out float itrc)
{
    float depth = 0;
    for (int i = 0; i < MAX_ITER; i++) {
        float dist = sceneSDF(ro + depth * rd);
        if (dist < 0.0001) {
			return depth;
        }
        depth += dist;
        if (depth >= MAX_DIST) {
            return MAX_DIST;
        }
    }
    return MAX_DIST;
}

vec3 rayDirection(float fov, vec2 size)
{
    vec2 xy = gl_FragCoord.xy - size / 2.0;
    float z = size.y / tan(radians(fov) / 2.0);
    return normalize(vec3(xy, -z));
}

vec3 estimateNormal(vec3 p)
{
    return normalize(vec3(
        sceneSDF(vec3(p.x + 0.0001, p.y, p.z)) - sceneSDF(vec3(p.x - 0.0001, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + 0.0001, p.z)) - sceneSDF(vec3(p.x, p.y - 0.0001, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + 0.0001)) - sceneSDF(vec3(p.x, p.y, p.z - 0.0001))
    ));
}

mat4 viewMatrix(vec3 eye, vec3 center, vec3 up)
{
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1)
    );
}





vec3 bgColor(in vec3 p)
{
    vec3 c = vec3(0.);
    float res = WIDTH*0.8;
    
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

float calculateAO(vec3 p, vec3 n){

    const float AO_SAMPLES = 5.0;
    float r = 0.0, w = 1.0, d;
    
    for (float i=1.0; i<AO_SAMPLES+1.1; i++) {
        d = i/AO_SAMPLES;
        r += w*(d - sceneSDF(p + n*d));
        w *= 0.5;
    }
    
    return 1.0-clamp(r,0.0,1.0);
}

float curve(in vec3 p, in float w){

    vec2 e = vec2(-1., 1.)*w;
    
    float t1 = sceneSDF(p + e.yxx), t2 = sceneSDF(p + e.xxy);
    float t3 = sceneSDF(p + e.xyx), t4 = sceneSDF(p + e.yyy);
    
    return 0.125/(w*w) *(t1 + t2 + t3 + t4 - 4.*sceneSDF(p));
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float maxt, float k )
{
    float res = 1.0;
    for( float t=mint; t < maxt; )
    {
        float h = sceneSDF(ro + rd*t);
        if( h<0.001 )
            return 0.0;
        res = min( res, k*h/t );
        t += h;
    }
    return res;
}





void main()
{
	vec2 iResolution = vec2(WIDTH, HEIGHT);
	
	
	vec2 uv = gl_FragCoord.xy / iResolution;
    uv = uv * 2.0 - 1.0;
    uv.x *= WIDTH / HEIGHT;
	
	vec3 viewDir = rayDirection(45.0, iResolution);
    vec3 eye = vec3(u_x, u_y, u_z);
    mat4 viewToWorld = viewMatrix(eye, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
	
	float count = 0;
	float dist = raymarch(eye, worldDir, count);
	
	vec3 rd = normalize(vec3(uv.xy,-2.0));
	rd.z += length(uv) * 0.15;
	
	if (dist > MAX_DIST - 0.0001)
	{
        gl_FragColor = vec4(bgColor(rd) - u_alpha, 0.0);
		return;
	}
	
	
	vec3 sp = eye + dist * worldDir;
	vec3 sn = estimateNormal(sp);
	vec3 color = vec3(sin(u_time), cos(sin(u_time)), cos(sin(u_time))*.5);
	
	
	
	const float tSize0 = 1./1.; 
    const float tSize1 = 1./4.;
	
	
	
	// Material
	
	
	
	
	
	
	// Shading
	
	
	
	
	
	// Light
	vec3 light_pos = vec3(4.0 * sin(u_time) * 3, 2.0, 4.0 * cos(u_time)) * 3;

	vec3 ld = light_pos-sp;
	
	float distlpsp = max(length(ld), 0.001);
	
	ld /= distlpsp;
	
	// Attenuation
	float atten = min(1./(distlpsp), 1);
	
	// Diffuse lighting
	float diff = max( dot(sn, ld), 0.0);
	
	// Specular lighting
	float spec = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 8.);
	
	float crv = clamp(curve(sp, 0.125)*0.5+0.5, .0, 1.);
	
	// Sky
	float sky = clamp( 0.5 + 0.5 * sn.y, 0.0, 1.0 );
	
	// Ambient occlusion
	float ao = calculateAO(sp, sn);
	
	// Shadow
	float shadow = softshadow(sp, light_pos, 0, 10, 8);
	
	// Fresnel
	float fre = pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.);
	
	// Sun
	vec3 sunDir = vec3(0.0, 0.0, 0.0);
	
    float sun = clamp( dot( sn, sunDir ), 0.0, 1.0 );
    float ind = clamp( dot( sn, normalize(sunDir*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
	
	// Final color
	vec3 li = sun*vec3(1.64,1.27,0.99)*pow(vec3(shadow),vec3(1.0,1.2,1.5));
	li += sky*vec3(0.16,0.20,0.28)*ao*(AMBIENCE*2);
	li += ind*vec3(0.40,0.28,0.20)*ao*(AMBIENCE*2);
	
	color = li * color + (spec*color*2);
	color *= fre*atten;
	
	color = pow(color, vec3(1.0/2.2) );
	
	gl_FragColor = vec4(color - u_alpha, 1.0);
}







