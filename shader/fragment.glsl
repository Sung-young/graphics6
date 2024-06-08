#version 120                  // GLSL 1.20

varying vec3 v_position_wc;   // position in world coordinates
varying vec3 v_normal_wc;     // normal in world coordinates

uniform vec3 u_light_position;
uniform vec3 u_light_ambient;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

uniform vec3 u_obj_ambient;
uniform vec3 u_obj_diffuse;
uniform vec3 u_obj_specular;
uniform float u_obj_shininess;

uniform vec3 u_camera_position;

void main()
{
    vec3 normal = normalize(v_normal_wc);
    vec3 light_dir = normalize(u_light_position - v_position_wc);

    // Ambient component
    vec3 ambient = u_light_ambient * u_obj_ambient;

    // Diffuse component
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diff * u_light_diffuse * u_obj_diffuse;

    // Specular component
    vec3 view_dir = normalize(u_camera_position - v_position_wc);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_obj_shininess);
    vec3 specular = spec * u_light_specular * u_obj_specular;

    // Combine all components
    vec3 color = ambient + diffuse + specular;

    gl_FragColor = vec4(color, 1.0);
}

