#version 400 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColors;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragColors;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	fragPos = vec3(model * vec4(aPos, 1.0)); // here we are transforming from vertex space to world space.
	fragNormal = mat3(transpose(inverse(model))) * aNormal; // this line ensures that the normals are set up correctly for lighting
	fragColors = aColors; // pretty self explanitory

	gl_Position = projection * view * model * vec4(aPos, 1.0); // final clip-space position
}