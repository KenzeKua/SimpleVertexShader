precision mediump float;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Factor1;

float boxSize = 50.0;
float halfBoxSize = boxSize * 0.5;
float texRes = 256.0;
float blurValue = 1.0 / texRes;

// Gaussian functions
float gaussianFunction(float x)
{
	float variance = 0.15; // x should be 0 - 1.0 with this variance
	
	float alpha = - (x*x / (2.0*variance));
	return exp(alpha);
}
float gaussianFunction2D(float x, float y) // x and y are the distance from your current pixel; normalized
{
	float variance = 0.15; // x and y should be 0 - 1.0 with this variance
	
	float alpha = -((x*x + y*y) / (2.0*variance));
	return exp(alpha);
}

void main()
{
	// Rainbow and flash effect
	vec4 texColor = texture2D(sampler2d, fTexCoord);
	// vec4 combinedColor;
	// combinedColor = fColor * texColor;
	
	// vec4 resultColor;
	// resultColor.r = mod(combinedColor.r + Factor1, 1.0);
	// resultColor.g = mod(combinedColor.g + Factor1, 1.0);
	// resultColor.b = mod(combinedColor.b + Factor1, 1.0);
	// resultColor.a = combinedColor.a;

	// gl_FragColor = resultColor;
	
	
	// Blur
	vec4 tempTexColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec2 tempTexCoord = vec2(0.0, 0.0);
	float samplerCounter = 0.0;
	
	for (float i = -halfBoxSize; i < halfBoxSize; i++)
	{
		for (float j = -halfBoxSize; j < halfBoxSize; j++)
		{
			tempTexCoord = fTexCoord + vec2((blurValue * i), (blurValue * j));
			if (tempTexCoord.x >= 0.0 && tempTexCoord.x <= 1.0 && tempTexCoord.y >= 0.0 && tempTexCoord.y <= 1.0)
			{
				tempTexColor += texture2D(sampler2d, tempTexCoord);
				samplerCounter++;
			}
		}
	}		  
	gl_FragColor = tempTexColor / samplerCounter;
	
	
	// Use texture color only
	// gl_FragColor = texture2D(sampler2d, fTexCoord);
}