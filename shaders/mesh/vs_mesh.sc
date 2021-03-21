$input a_position, a_normal
$output v_pos, v_normal

#include <bgfx_shader.sh>

void main() {
  v_normal = mul(u_modelView, vec4(a_normal, 0.0)).xyz;
  gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
  v_pos = gl_Position.xyz;
}
