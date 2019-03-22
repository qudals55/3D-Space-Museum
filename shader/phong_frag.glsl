
uniform vec3 u_light_position;
uniform vec4 u_light_ambient;
uniform vec4 u_light_diffuse;
uniform vec4 u_light_specular;

uniform vec4 u_material_ambient;
uniform vec4 u_material_diffuse;
uniform vec4 u_material_specular;
uniform float u_material_shininess;

uniform sampler2D u_texid;

varying vec2 v_texcoord;
varying vec3 v_vertex;
varying vec3 v_normal;

vec4 directional_light() {
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	color += (u_light_ambient * u_material_ambient);

	vec3 light_vector = normalize(u_light_position);
	vec3 normal = normalize(v_normal);

	float ndotl = max(0.0, dot(normal, light_vector) );
	color += (ndotl * u_light_diffuse * u_material_diffuse);

	vec3 reflect_vector = reflect(-light_vector, normal);

	vec3 view_vector = normalize(-v_vertex);

	float rdotv = dot(reflect_vector, view_vector) ;

	color += (pow(rdotv, u_material_shininess) * u_light_specular * u_material_specular);
	return color;
}
void main() {

gl_FragColor = directional_light()*texture2D(u_texid, v_texcoord) ;
}
