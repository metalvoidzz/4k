#version 120

uniform float u_time;

#define WIDTH 800
#define HEIGHT 600

#define PI 3.141592
#define MAX_ITER 255
#define MAX_DIST 100.0
#define EPSILON 0.0001
#define OMEGA 1.2
#define WH vec2(WIDTH, HEIGHT)




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
	return max3(abs(p) - b);
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


#define Repeat(p, c) \
	mod(p, c)-0.5*c
	
float sceneSDF(vec3 p)
{
	float s = cos(sin(u_time * 3)) * 0.5;
	
	float box = cubeSDF(p, vec3(0.2,0.2,0.2));
	
	p.y -= sin(u_time) * 2;
	
	float torus = torusSDF(p, vec2(cos(sin(u_time))*2,0.1));
	
	return min(Combine(box, torus, 0.2), sphereSDF(p, 0.3));
	//return cubeSDF(Repeat(p, 2), vec3(.2, .2, .2));
}



float raymarch(vec3 eye, vec3 dir)
{
    float depth = 0.0;
    for (int i = 0; i < MAX_ITER; i++)
	{
        float dist = sceneSDF(eye + depth * dir);
        if (dist < EPSILON)
			return depth;
        depth += dist;
        if (depth >= MAX_DIST)
            break;
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
        sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
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
    const vec3 ambientLight = 0.2 * vec3(1.0, 1.0, 1.0);
    vec3 color = ambientLight * k_a;
    
    vec3 light1Pos = vec3(4.0 * sin(u_time) * 3,
                          2.0,
                          4.0 * cos(u_time)) * 3;
    vec3 light1Intensity = vec3(0.4, 0.4, 0.4);
    
    color += phongContribForLight(k_d, k_s, alpha, p, eye,
                                  light1Pos,
                                  light1Intensity);
    
    vec3 light2Pos = vec3(2.0 * sin(0.37 * u_time) * 3,
                          2.0 * cos(0.37 * u_time) * 3,
                          2.0);
    vec3 light2Intensity = vec3(0.4, 0.4, 0.4);
    
    color += phongContribForLight(k_d, k_s, alpha, p, eye,
                                  light2Pos,
                                  light2Intensity);    
    return color;
}



void main()
{
	vec2 uv = gl_FragCoord.xy / WH;
    uv = uv * 2.0 - 1.0;
    uv.x *= WIDTH / HEIGHT;
	
	vec3 viewDir = rayDirection(45.0, WH);
    vec3 eye = vec3(8.0, 5.0, u_time);
    mat4 viewToWorld = viewMatrix(eye, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    vec3 worldDir = (viewToWorld * vec4(viewDir, 0.0)).xyz;
	
    float dist = raymarch(eye, worldDir);
    
	vec3 dir = normalize(vec3(uv.xy,-2.0));
	dir.z += length(uv) * 0.15;
	
    if (dist > MAX_DIST - EPSILON)
	{
        gl_FragColor = vec4(SkyColor(dir), 0.0);
		return;
    }
	
    vec3 p = eye + dist * worldDir;
    
    vec3 K_a = vec3(0.2, 0.2, 0.2);
    vec3 K_d = vec3(0.7, 0.2, 0.2);
    vec3 K_s = vec3(1.0, 1.0, 1.0);
    float shininess = 10.0;
    
    vec3 color = phongIllumination(K_a, K_d, K_s, shininess, p, eye);
	
    gl_FragColor = vec4(color, 1.0);	
}






