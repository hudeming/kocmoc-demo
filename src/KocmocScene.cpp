#include "KocmocScene.hpp"
#include "Kocmoc.hpp"

using namespace kocmoc;

KocmocScene::KocmocScene() : name("Scene")
{
	gizmoShader = new Shader("vertexColor.vert", "vertexColor.frag");
}

KocmocScene::~KocmocScene(void)
{
	for (std::list<PolyMesh* >::iterator it = polyMeshList.begin(); it != polyMeshList.end(); it++)
	{
		delete [] *it;
	}
}

void KocmocScene::addPolyMesh(PolyMesh* mesh)
{
	polyMeshList.push_back(mesh);
}

std::list<PolyMesh*> KocmocScene::getPolyMeshList()
{
	return polyMeshList;
}

void KocmocScene::transferData(Shader * shader)
{
	// create gizmo
	// TODO: make model and load it
	gizmo_vbos = new GLuint[2];
	glGenVertexArrays(1, &gizmo_vao);
	glGenBuffers(2, gizmo_vbos);

	GLfloat vertexPositions[] = {0.0f, 0.0f, 0.0f, 1.0f,
								1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f,
								0.0f, 0.0f, 1.0f, 0.0f};
	GLfloat vertexColors[] = {1.0f, 0.0f, 0.0f,
							1.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f,
							0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 1.0f,
							0.0f, 0.0f, 1.0f};

	glBindVertexArray(gizmo_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, gizmo_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 *sizeof(float), vertexPositions, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_ATTR_INDEX_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VERTEX_ATTR_INDEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, gizmo_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	glVertexAttribPointer(VERTEX_ATTR_INDEX_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VERTEX_ATTR_INDEX_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void KocmocScene::draw(Shader* shader)
{
	for (PolyMeshPointerList::const_iterator ci = polyMeshList.begin(); ci != polyMeshList.end(); ci++)
	{
		(*ci)->draw();
	}

	gizmoShader->bind();
	{
		Camera *camera = Kocmoc::getInstance().getCamera();

		GLint viewMatrix_location = gizmoShader->get_uniform_location("viewMatrix");		GLint projectionMatrix_location = gizmoShader->get_uniform_location("projectionMatrix");		
		glUniformMatrix4fv(viewMatrix_location, 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
		glUniformMatrix4fv(projectionMatrix_location, 1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));		
		glBindVertexArray(gizmo_vao);
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);
	}
	gizmoShader->unbind();
}