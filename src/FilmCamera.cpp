#include "FilmCamera.hpp"

#include <gtc/matrix_projection.hpp> 

FilmCamera::FilmCamera(vec3 _eyePosition, vec3 _targetPosition, vec3 _upVector) :
	eyePosition(_eyePosition),
	targetPosition(_targetPosition),
	upVector(glm::normalize(_upVector))
{
	// TODO Auto-generated constructor stub
}

FilmCamera::~FilmCamera()
{
	// TODO Auto-generated destructor stub
}

mat4 FilmCamera::getProjectionMatrix()
{
	return projectionMatrix;
}

mat4 FilmCamera::getViewMatrix()
{
	return viewMatrix;
}

mat4 FilmCamera::getUntraslatedViewMatrix()
{
	return untranslatedViewMatrix;
}


void FilmCamera::updateMatrixes() 
{
	vec3 f = glm::normalize(targetPosition - eyePosition);
	vec3 s = glm::cross(f, upVector);
	vec3 u = glm::cross(s, f);

	untranslatedViewMatrix = mat4(vec4(s, 0), vec4(upVector, 0), vec4(-f, 0), vec4(0, 0, 0, 1.0f));
	viewMatrix = glm::translate(untranslatedViewMatrix, eyePosition);
	projectionMatrix = glm::perspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
}
