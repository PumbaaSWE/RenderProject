#version 450

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 lightDir = normalize(vec3(0,1,0 ));
	vec3 norm = normalize(normal);
outColor = vec4(1.0,1.0,1.0, 1.0);
outColor *= dot(lightDir , norm)*0.5+0.5;
//outColor = vec4(normalize(normal)*0.5+0.5, 1.0);
	//outColor = vec4(1.0,1.0,1.0 1.0);
}

