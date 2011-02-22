#include "RenderMesh.hpp"

#include "Shader.hpp"

#include <scene/PolyMesh.hpp>
#include <scene/LineGizmo.hpp>
#include <camera/Camera.hpp>
#include <util/util.hpp>

#include <gtx/inverse_transpose.hpp>

#include <vector>

using namespace kocmoc::renderer;
using kocmoc::scene::PolyMesh;
using kocmoc::scene::LineGizmo;
using kocmoc::camera::Camera;

using glm::mat4;
using glm::mat3;
using glm::dvec3;
using glm::gtx::inverse_transpose::inverseTranspose;

using std::vector;


RenderMesh::RenderMesh(PolyMesh* _mesh, Shader* _shader)
	: mesh(_mesh)
	, shader(_shader)
	, modelMatrix(mat4(1))
	, isUploaded(false)
{
	originGizmo = util::generator::generateOriginGizmo();
	boundingBox = util::generator::generateUnitCube();

	PolyMesh::BoundingBox bb = mesh->calculateBoundingBox();
	dvec3 center = (bb.max + bb.min) / 2.0;
	dvec3 scale = bb.max - bb.min;

	bbTransform = glm::scale(glm::translate(center), scale);
};

void RenderMesh::setModelMatrix(mat4 _modelMatrix)
{
	modelMatrix = _modelMatrix;
	
}

void RenderMesh::draw(mat4 parentTransform, Camera *camera)
{
	if (shader->getIsUploaded() == false)
		shader->upload();

	if (isUploaded == false)
		uploadData();

	mat4 leafTransform = parentTransform * modelMatrix;
	mat3 normalMatrix = inverseTranspose(mat3(modelMatrix));


	shader->bind();

	GLint location;
	if ((location = shader->get_uniform_location("cameraPosition")) >= 0)
		glUniform3fv(location, 1, glm::value_ptr(camera->getPosition()));

	if ((location = shader->get_uniform_location("projectionMatrix")) >= 0)		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));
	if ((location = shader->get_uniform_location("viewMatrix")) >= 0)		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
	if ((location = shader->get_uniform_location("modelMatrix")) >= 0)		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(leafTransform));

	if (location = shader->get_uniform_location("normalMatrix") >= 0)
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(normalMatrix));	
	

	for (TextureList::const_iterator ci = textures.begin();
		ci != textures.end();
		ci++)
	{
		glActiveTexture(ci->textureUnit);
		glBindTexture(GL_TEXTURE_2D, ci->handle);
	}


	glBindVertexArray(vaoHandle);
	glDrawElements(GL_TRIANGLES, triangulatedVertexIndexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	shader->unbind();

	// extras, debug etc...
	originGizmo->draw(leafTransform, camera);
	boundingBox->draw(leafTransform * bbTransform, camera);
}

void RenderMesh::uploadData()
{
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);


	// reindex vertex data and convert to float
	// brute force expansion, (for now...)
	Shader::VertexAttributeSemanticList semantics = shader->getSemantics();

	for (Shader::VertexAttributeSemanticList::const_iterator ci = semantics.begin();
		ci != semantics.end();
		ci++)
	{
		// TODO: make find error proof!
		const PolyMesh::VertexAttribute& attribute = mesh->vertexAttributes.find(ci->attributeName)->second;

		uint indexCount = mesh->vertexIndexCount;
		uint dataLength = indexCount * attribute.stride;

		float* reindexedData = new float[dataLength];

		for (uint i = 0; i < indexCount; i++)
		{
			for (uint j = 0; j < attribute.stride; j++)
			{
				reindexedData[i*attribute.stride + j] = (float)attribute.attributeData[attribute.indices[i] * attribute.stride + j];
			}
		}

		GLuint handle = 0;
		glGenBuffers(1, &handle);
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		glBufferData(GL_ARRAY_BUFFER, dataLength * sizeof(float), reindexedData, GL_STATIC_DRAW);
		glVertexAttribPointer(ci->attributeIndex, attribute.stride, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(ci->attributeIndex);

		delete[] reindexedData;
	}

	
	
	// triangulate indices
	// upload

	uint* reindexedIndices = new uint[mesh->vertexIndexCount];
	for (uint i = 0; i < mesh->vertexIndexCount; i++)
		reindexedIndices[i] = i;

	vector<unsigned int> triangulatedIndices;

	for (uint i = 0; i < mesh->primitiveCount; i++)
	{
		uint count = mesh->firstIndexArray[i+1] - mesh->firstIndexArray[i];

		if (count == 3)
		{
			// just upload
			triangulatedIndices.push_back(reindexedIndices[mesh->firstIndexArray[i]]);
			triangulatedIndices.push_back(reindexedIndices[mesh->firstIndexArray[i]+1]);
			triangulatedIndices.push_back(reindexedIndices[mesh->firstIndexArray[i]+2]);
		}
		else if (count > 3)
		{
			// triangulate
			unsigned int firstIndex = mesh->firstIndexArray[i];
			for (unsigned int offset = 1; offset < (count - 1); offset++)
			{
				triangulatedIndices.push_back(reindexedIndices[firstIndex]);
				triangulatedIndices.push_back(reindexedIndices[firstIndex + offset]);
				triangulatedIndices.push_back(reindexedIndices[firstIndex + offset + 1]);
			}
		}
	}
	
	triangulatedVertexIndexCount = triangulatedIndices.size();
	
	// upload
	GLuint handle;
	glGenBuffers(1, &handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangulatedVertexIndexCount * sizeof(unsigned int), &triangulatedIndices[0], GL_STATIC_DRAW);

	delete[] reindexedIndices;

	isUploaded = true;
}