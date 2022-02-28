precision mediump float;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Factor1;


void main()
{
	gl_FragColor = fColor;
}