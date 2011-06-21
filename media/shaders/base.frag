// Fragment Shader – file "base.frag"

#version 150 core

#pragma include scattering.glsl

in vec2 texCoord0;
in vec3 fragmentNormal;
in vec3 worldSpacePosition;
in vec3 shadowUV;

uniform sampler2D sDiffuse;
uniform sampler2D sSpecular;
uniform sampler2D sNormal;
uniform sampler2D sShadow;

uniform mat3 normalMatrix;
uniform vec3 cameraPosition;
uniform vec3 sunDirection;
uniform float reileighCoefficient;
uniform float mieCoefficient;
uniform float mieDirectionalG;
uniform float turbidity;

out vec4 fragmentColor0;
out float fragmentColor1;

void main(void)
{
	
	float sunE = sunIntensity(dot(sunDirection, vec3(0.0f, 1.0f, 0.0f)));
	
	const float ambientIntensity = sunE * 0.1f;
	

	vec3 diffuseColor = vec3(0.5, 0.5, 0.5);
	vec3 ambientTerm = diffuseColor * ambientIntensity;
	float diffuseFactor =  sunE * max(dot(sunDirection, fragmentNormal.xyz), 0);
	vec3 diffuseTerm = diffuseColor * diffuseFactor;

	vec4 L0 = vec4(diffuseTerm + ambientTerm, 1);


	// --- atmospheric scattering starts here ---


	// extinction (absorbtion + out scattering)
	// rayleigh coefficients
	vec3 betaR = totalRayleigh(lambda) * reileighCoefficient;

	// mie coefficients
	vec3 betaM = totalMie(lambda, K, turbidity) * mieCoefficient;

		
	float distance = length(worldSpacePosition - cameraPosition);
	vec3 Fex = exp(-((betaR + betaM) * distance));


	// in scattering
	float cosTheta = dot(normalize(worldSpacePosition - cameraPosition), sunDirection);
	
	float rPhase = rayleighPhase(cosTheta);
	vec3 betaRTheta = betaR * rPhase;

	float mPhase = hgPhase(cosTheta, mieDirectionalG);
	vec3 betaMTheta = betaM * mPhase;

	vec3 Lin = sunE * ((betaRTheta + betaMTheta) / (betaR + betaM)) * (1.0f - Fex);

	// composition
	fragmentColor0 = L0 * vec4(Fex, 1) + vec4(Lin, 0);
	fragmentColor1 = log(fragmentColor0.r * 0.2126f + fragmentColor0.g * 0.7152f + fragmentColor0.b * 0.0722f);  
}
