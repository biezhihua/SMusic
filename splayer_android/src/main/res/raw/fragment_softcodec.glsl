// 片段着色器

precision mediump float;
varying vec2 v_texPosition;
uniform sampler2D texture_y;
uniform sampler2D texture_u;
uniform sampler2D texture_v;
void main() {
    float y, u, v;
    y = texture2D(texture_y, v_texPosition).r;
    u = texture2D(texture_u, v_texPosition).r- 0.5;
    v = texture2D(texture_v, v_texPosition).r- 0.5;

    vec3 rgb;
    rgb.r = y + 1.403 * v;
    rgb.g = y - 0.344 * u - 0.714 * v;
    rgb.b = y + 1.770 * u;

    gl_FragColor = vec4(rgb, 1);
}
