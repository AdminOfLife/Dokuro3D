#include "Transformer.hpp"
using namespace dkr;

#include "../Exceptions/DokuroException.hpp"
#include "../GraphicsMemory/VertexBuffer.hpp"

Transformer::Transformer() {
	// Set the lighting to null
	lightInfo = NULL;
	renderer = NULL;
	memory = NULL;
}

Transformer::~Transformer() {
}

/* Set the graphics memory */
void Transformer::SetGraphicsMemory(GraphicsMemory* _memory) {
	memory = _memory;
}

/* Set the renderer to use for screen space transformations */
void Transformer::SetRenderer(IRenderTarget* _renderer) {
	renderer = _renderer;
}

/* Transform a buffer */
void Transformer::Transform(int _bufferId) {
	// Grab a reference to the buffer and the temp buffer
	const VertexBuffer& buffer = memory->GetBuffer(_bufferId);
	DrawBuffer& drawBuffer = memory->GetTempBuffer(_bufferId);

	// Clear the draw buffer (the triangles).
	drawBuffer.Clear();
	drawBuffer.GetTriangles().reserve(buffer.GetPositionsIndex().size() / 3); // Reserve this much space since we know that we aren't going to need
																	// any more than this.

	// Transform all the points
	TransformPoints(_bufferId);

	// Get the points in world space so we can use them for lighting calculations
	std::vector<Vector4> worldSpacePositions;
	std::vector<Vector4> worldSpaceVertexNormals;
	std::vector<Vector4> worldSpaceFaceNormals;
	std::vector<Vector4> cameraSpaceFaceNormals;
	std::vector<Vector4> cameraSpaceZ;

	// If smooth shading is enabled then transform the vertex normals
	if (buffer.GetShading() != VertexBuffer::SHADE_FLAT) {
		worldSpaceVertexNormals.resize(buffer.GetNormals().size(), 0);
		for (unsigned int i = 0; i < buffer.GetNormals().size(); ++i) {
			worldSpaceVertexNormals[i] = transform.Peek() * buffer.GetNormals()[i];
			worldSpaceVertexNormals[i].Normalize();
		}
	}

	// Get the positions in world space too.
	worldSpacePositions.resize(buffer.GetPositions().size(), 0);
	for (unsigned int i = 0; i < buffer.GetPositions().size(); ++i) {
		worldSpacePositions[i] = transform.Peek() * buffer.GetPositions()[i];
	}

	// If textures are enabled then calculate the camera space z coordinates.
	cameraSpaceZ.resize(buffer.GetPositions().size(), 0);
	for (unsigned int i = 0; i < buffer.GetPositions().size(); ++i) {
		cameraSpaceZ[i] = camera.Peek() * worldSpacePositions[i];
	}

	// Face normals in world space are needed for flat shading.
	if (buffer.GetShading() == VertexBuffer::SHADE_FLAT) {
		worldSpaceFaceNormals.resize(buffer.GetFaceNormals().size(), 0);
		for (unsigned int i = 0; i < buffer.GetFaceNormals().size(); ++i) {
			worldSpaceFaceNormals[i] = transform.Peek() * buffer.GetFaceNormals()[i];
			worldSpaceFaceNormals[i].Normalize();
		}
	}

	// Get the face normals in camera space so we can do back face culling.
	Matrix4 cullCheckMatrix = camera.Peek() * transform.Peek();
	if (buffer.GetFaceCulling() != VertexBuffer::CULL_NONE) {
		cameraSpaceFaceNormals.resize(buffer.GetFaceNormals().size(), 0);
		for (unsigned int i = 0; i < buffer.GetFaceNormals().size(); ++i) {
			cameraSpaceFaceNormals[i] = cullCheckMatrix * buffer.GetFaceNormals()[i];
		}
	}
	for (unsigned int i = 0, j = 0; i < buffer.GetPositionsIndex().size(); i+=3, j++) {
		// Whether or not to add the face.
		bool addFace = false;

		if (buffer.GetFaceCulling() != VertexBuffer::CULL_NONE) {
			// Transform a point in the triangle
			Vector4 testPoint = cullCheckMatrix * buffer.GetPositions()[buffer.GetPositionsIndex()[i]];

			// Cull depending on the dot product
			if (buffer.GetFaceCulling() == VertexBuffer::CULL_BACK) {
				if (cameraSpaceFaceNormals[j].Dot(testPoint) < 0)
					addFace = true;
			} else {
				if (cameraSpaceFaceNormals[j].Dot(testPoint) > 0)
					addFace = true;
			}
		} else {
			addFace = true;
		}

		// Cull anything that is out of the screen
		int vertTest = 0;
		for (unsigned int k = 0; k < 3; ++k) {
			Vector4 test = drawBuffer.GetPositions()[buffer.GetPositionsIndex()[i + k]];
			if (test.x < -1.0f || test.x > 1.0f ||
				test.y < -1.0f || test.y > 1.0f ||
				test.z < 0.0f || test.z > 1.0f) {
				++vertTest;
			}
		}
		if (vertTest >= 3)
			addFace = false;

		// Add the face and create a triangle if we need to.
		if (addFace) {
			// The triangle we are adding.
			DrawBuffer::Triangle triangle;
			triangle.vertexIndices[0] = buffer.GetPositionsIndex()[i];
			triangle.vertexIndices[1] = buffer.GetPositionsIndex()[i+1];
			triangle.vertexIndices[2] = buffer.GetPositionsIndex()[i+2];
			triangle.vertices[0] = buffer.GetPositions()[triangle.vertexIndices[0]];
			triangle.vertices[1] = buffer.GetPositions()[triangle.vertexIndices[1]];
			triangle.vertices[2] = buffer.GetPositions()[triangle.vertexIndices[2]];
			triangle.vertexTextureCoord[0] = buffer.GetTextureCoords()[buffer.GetTextureCoordsIndex()[i]];
			triangle.vertexTextureCoord[1] = buffer.GetTextureCoords()[buffer.GetTextureCoordsIndex()[i+1]];
			triangle.vertexTextureCoord[2] = buffer.GetTextureCoords()[buffer.GetTextureCoordsIndex()[i+2]];
			triangle.texture = buffer.GetTexture();
			triangle.originalZ[0] = -cameraSpaceZ[triangle.vertexIndices[0]].z;
			triangle.originalZ[1] = -cameraSpaceZ[triangle.vertexIndices[1]].z;
			triangle.originalZ[2] = -cameraSpaceZ[triangle.vertexIndices[2]].z;

			// Information for lighting (phong)
			if (buffer.GetShading() == VertexBuffer::SHADE_PHONG) {
				for (int i = 0; i < 3; ++i) {
					triangle.cameraSpace[i] = cameraSpaceZ[triangle.vertexIndices[i]];
				}
				for (int i = 0; i < 3; ++i) {
					triangle.worldSpace[i] = worldSpacePositions[triangle.vertexIndices[i]];
				}
				for (int i = 0; i < 3; ++i) {
					triangle.vertexNormals[i] = worldSpaceVertexNormals[buffer.GetNormalsIndex()[triangle.vertexIndices[i]]];
				}
				// The lighting coefficients.
				triangle.diffuse = buffer.GetDiffuse();
				triangle.specular = buffer.GetSpecular();
				triangle.ambient = buffer.GetAmbient();

				// The lighting information
				triangle.info = lightInfo;

			} else if (buffer.GetShading() == VertexBuffer::SHADE_FLAT) {
				triangle.faceNormal = worldSpaceFaceNormals[j];
			}

			// If we are shading.
			Vector4 ambientColor;
			if (buffer.GetShading() == VertexBuffer::SHADE_FLAT) {
				// Ambient Light
				ambientColor = lightInfo->GetAmbient().GetIntensity() * buffer.GetAmbient();
				Vector4 color = ambientColor;

				// Directional Light
				for (unsigned int k = 0; k < lightInfo->GetDirectionalLights().size(); ++k) {
					color += lightInfo->GetDirectionalLightOnPoint(cameraSpaceZ[triangle.vertexIndices[0]], triangle.faceNormal, k, buffer.GetDiffuse(), buffer.GetSpecular());
				}	

				// Point lights
				for (unsigned int k = 0; k < lightInfo->GetPointLights().size(); ++k) {
					color += lightInfo->GetPointLightOnPoint(cameraSpaceZ[triangle.vertexIndices[0]], worldSpacePositions[triangle.vertexIndices[0]], triangle.faceNormal, k, buffer.GetDiffuse(), buffer.GetSpecular());
				}

				// Clamp the color
				color.Clamp();

				// Push the color onto the buffer
				triangle.faceColor = color;
			}
			else if (buffer.GetShading() == VertexBuffer::SHADE_SMOOTH) {
				// Do lighting for each vertex.
				
				// Ambient lighting
				Vector4 ambient = lightInfo->GetAmbient().GetIntensity() * buffer.GetAmbient();
				for (unsigned int k = 0; k < 3; ++k) {
					triangle.vertexColors[k] = ambient;
				}

				// Directional lighting
				for (unsigned int l = 0; l < lightInfo->GetDirectionalLights().size(); ++l) {
					for (int k = 0; k < 3; ++k) {
						triangle.vertexColors[k] += lightInfo->GetDirectionalLightOnPoint(cameraSpaceZ[triangle.vertexIndices[k]], worldSpaceVertexNormals[buffer.GetNormalsIndex()[triangle.vertexIndices[k]]], l, buffer.GetDiffuse(), buffer.GetSpecular());
					}
				}

				// Point lighting
				for (unsigned int l = 0; l < lightInfo->GetPointLights().size(); ++l) {
					for (int k = 0; k < 3; ++k) {
						triangle.vertexColors[k] += lightInfo->GetPointLightOnPoint(cameraSpaceZ[triangle.vertexIndices[k]], worldSpacePositions[triangle.vertexIndices[k]], worldSpaceVertexNormals[buffer.GetNormalsIndex()[triangle.vertexIndices[k]]], l, buffer.GetDiffuse(), buffer.GetSpecular());
					}
				}

				// Spot lighting
				for (unsigned int l = 0; l < lightInfo->GetSpotLights().size(); ++l) {
					for (int k = 0; k < 3; ++k) {
						triangle.vertexColors[k] +=  lightInfo->GetSpotLightOnPoint(cameraSpaceZ[triangle.vertexIndices[k]], worldSpacePositions[triangle.vertexIndices[k]], worldSpaceVertexNormals[buffer.GetNormalsIndex()[triangle.vertexIndices[k]]], l, buffer.GetDiffuse(), buffer.GetSpecular());
					}
				}

				// Clamp the colors
				for (unsigned int k = 0; k < 3; ++k) {
					triangle.vertexColors[k].Clamp();
				}
			}

			// Add the face to the drawbuffer
			drawBuffer.GetTriangles().push_back(triangle);
		}
	}

	// Transform the points into screen space
	for (unsigned int i = 0; i < buffer.GetPositions().size(); ++i) {
		drawBuffer.GetPositions()[i].x = ((drawBuffer.GetPositions()[i].x + 1) * renderer->GetWidth()) / 2;
		drawBuffer.GetPositions()[i].y = ((-drawBuffer.GetPositions()[i].y + 1) * renderer->GetHeight()) / 2;
	}
}

void Transformer::TransformPoints(int _bufferId) {
	// Get the buffers
	const VertexBuffer& buffer = memory->GetBuffer(_bufferId);
	DrawBuffer& drawBuffer = memory->GetTempBuffer(_bufferId);

	// A full transform matrix.
	Matrix4 matrix = projection.Peek() * camera.Peek() * transform.Peek();

	// Tranform all the points
	for (unsigned int i = 0; i < buffer.GetPositions().size(); ++i) {
		drawBuffer.GetPositions()[i] = Vector4();
		drawBuffer.GetPositions()[i] = matrix * buffer.GetPositions()[i];
		drawBuffer.GetPositions()[i] /= drawBuffer.GetPositions()[i].w;
	}
}

/* Get a matrix stack from the map */
MatrixStack& Transformer::GetTransform() {
	return transform;
}

MatrixStack& Transformer::GetCamera() {
	return camera;
}

MatrixStack& Transformer::GetProjection() {
	return projection;
}

MatrixStack& Transformer::GetTexture() {
	return texture;
}

void Transformer::SetLightInfo(LightInfo* _info) {
	lightInfo = _info;
}
LightInfo* Transformer::GetLightInfo() {
	return lightInfo;
}