//
// Created by pedro-souza on 23/11/2025.
//

#include "VertexArray.h"
#include <GL/glew.h>

VertexArray::VertexArray(const float* verts, unsigned int numVerts, const unsigned int* indices,
                         unsigned int numIndices)
: mNumVerts(numVerts)
, mNumIndices(numIndices)
, mVertexBuffer(0)
, mIndexBuffer(0)
, mVertexArray(0)
{
    // Create vertex array
    glGenVertexArrays(1, &mVertexArray);
    glBindVertexArray(mVertexArray);

    // Create vertex buffer
    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    
    // Determine format based on numVerts and numIndices
    // Sprite format: 4 vertices (numVerts=4) with 6 indices, and 8 floats per vertex = 32 floats total
    // 2D line format: numVerts is actually number of floats (vertices * 2), with 2 floats per vertex
    
    // Check if this is sprite format (4 vertices, 6 indices = quad)
    if (numVerts == 4 && numIndices == 6) {
        // Sprite format: 8 floats per vertex (3 pos + 3 normal + 2 texCoord)
        unsigned int numFloats = numVerts * 8; // 4 vertices * 8 floats = 32 floats
        unsigned int stride = 8 * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), verts, GL_STATIC_DRAW);
        
        // Position is 3 floats
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
        // Normal is 3 floats
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3));
        // Texture coordinates is 2 floats
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 6));
    } else {
        // 2D line format: 2 floats per vertex (x, y)
        // numVerts is actually the number of floats (not vertices), so numVerts / 2 = actual vertex count
        unsigned int numFloats = numVerts;
        unsigned int stride = 2 * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), verts, GL_STATIC_DRAW);
        
        // Position is 2 floats (x, y) - only use attribute 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, 0);
        // Disable other attributes for line format
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    // Create index buffer
    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

VertexArray::~VertexArray()
{
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mIndexBuffer);
    glDeleteVertexArrays(1, &mVertexArray);
}

void VertexArray::SetActive() const
{
    glBindVertexArray(mVertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
}
