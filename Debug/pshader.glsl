#version 120


uniform float un[6];


float u_time = un[0];
float u_alpha = un[1];
float u_x = un[2];
float u_y = un[3];
float u_z = un[4];
float u_scene = un[5];


#define WIDTH 1366
#define HEIGHT 768

#define PI 3.141592
#define MAX_ITER 400
#define MAX_DIST 200.0
#define EPSILON 0.0001
#define OMEGA 1.2



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




float sphereSDF(vec3 p, float s)
{
	return length(p)-s;
}

float max3(vec3 v)
{
	return max(max (v.x, v.y), v.z);
}

float cubeSDF(vec3 p, vec3 b)
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float torusSDF(vec3 p, vec2 t)
{
	vec2 q = vec2(length(p.xz)-t.x,p.y);
	return length(q)-t.y;
}

float Union(float d1, float d2)
{
    return min(d1,d2);
}

float Sub(float d1, float d2)
{
    return max(-d1,d2);
}

float Inter(float d1, float d2)
{
    return max(d1,d2);
}

float Combine(float d1, float d2, float r)
{
	float m = min(d1, d2);
	if((d1 < r) && (d2 < r))
	{
		return min(m, r - sqrt((r - d1) * (r - d1) + (r - d2) * (r - d2)));
	}else {
		return m;
	}
}
	
// rotate space
void sR(inout vec2 p, float a)
{
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot(p,n.xyz) + n.w;
}

float cylinderSDF(vec3 p, float r, float height)
{
	float d = length(p.xz) - r;
	d = max(d, abs(p.y) - height);
	return d;
}

float combine(float a, float b, float r)
{
	vec2 u = max(vec2(r - a,r - b), vec2(0));
	return max(r, min (a, b)) - length(u);
}

float pMod1(inout float p, float size)
{
	float halfsize = size*0.5;
	float c = floor((p + halfsize)/size);
	p = mod(p + halfsize, size) - halfsize;
	return c;
}

float sceneSDF(vec3 p)
{
	if(u_scene == 0)
	{
		float scene;
		
		sR(p.yz, 90);
		float i = pMod1(p.x, 1);
		
		
		float box = cubeSDF(p, vec3(.15,1.6,0.6));
		float wall = cubeSDF(p, vec3(0.03, 3.0, 1.1));
		
		p.y -= 1.6;
		float c1 = cylinderSDF(p.yxz, 0.6, .15);
		p.y += 3.2;
		float c2 = cylinderSDF(p.yxz, 0.6, .15);

		
		float cylinders = min(c1, c2);
		
		
		float mountedPart = min(box, cylinders);
		
		
		float w = combine(mountedPart, wall, 0.04);
		
		scene = w;
		
		return scene;
	}else if(u_scene == 1) {
		return MAX_DIST;
	}
}

float raymarch(vec3 eye, vec3 dir)
{
	float t = 0.0;
	float d = 1.0;
	float pd = 10.0;
	float os = 0.0;
	
	for(int i = 0; i < MAX_ITER; i++)
	{
		d=sceneSDF(eye + t * dir);
		if(d > os)
		{
			os = d * min(1.0, 0.5 * d / pd);
			t += d + os;
			pd = d;
		}else {
			t -= os;
			pd = 100000.0;
			d = 0.1;
			os = 0.0;
		}
		if(t > MAX_DIST || d < 0.0) return MAX_DIST;
	}
	
	return t;
}

vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
    vec2 xy = fragCoord - size / 2.0;
    float z = size.y / tan(radians(fieldOfView) / 2.0);
    return normalize(vec3(xy, -z));
}

vec3 estimateNormal(vec3 p) {
    return normalize(vec3(
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

vec3 SkyColor(vec3 e)
{
    e.y = max(e.y, 0.0);
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, .6+(1.0-e.y) * .4);
}

vec3 phongContribForLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye, vec3 lightPos, vec3 lightIntensity)
{
    vec3 N = estimateNormal(p);
    vec3 L = normalize(lightPos - p);
    vec3 V = normalize(eye - p);
    vec3 R = normalize(reflect(-L, N));
    
    float dotLN = dot(L, N);
    float dotRV = dot(R, V);
    
    if (dotLN < 0.0) {
        return vec3(0.0, 0.0, 0.0);
    } 
    
    if (dotRV < 0.0) {
        return lightIntensity * (k_d * dotLN);
    }
    return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye)
{
    const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
    vec3 color = ambientLight * k_a;
    
    vec3 lightPos = vec3(4, 2, 1);
    vec3 lightIntensity = vec3(0.4, 0.4, 0.4);
    
    color += phongContribForLight(k_d, k_s, alpha, p, eye, lightPos, lightIntensity);
								  
    return color;
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

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    uv = uv * 2.0 - 1.0;
    uv.x *= WIDTH / HEIGHT;
	
	vec3 dir = normalize(vec3(uv.xy,-2.0));
	dir.z += length(uv) * 0.15;
	
	vec3 viewDir = rayDirection(45.0, vec2(WIDTH, HEIGHT), gl_FragCoord.xy);
    
	vec3 eye = vec3(u_x, u_y, u_z);
    mat4 viewToWorld = viewMatrix(eye, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
	
    float dist = raymarch(eye, worldDir);
	
    if (dist > MAX_DIST - EPSILON)
	{
        gl_FragColor = vec4(SkyColor(dir), 0.0);
		return;
    }

    vec3 p = eye + dist * worldDir;
    
    vec3 K_a = vec3(0.2, 0.2, 0.2);
    vec3 K_d = vec3(0.7, 0.2, 0.2);
    vec3 K_s = vec3(1.0, 1.0, 1.0);
    float shininess = 5.0;
	
	vec3 color = phongIllumination(K_a, K_d, K_s, shininess, p, eye);
	
    gl_FragColor = vec4(color, 1.0);	
}





