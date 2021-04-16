#include "Scene_024_TestingBubble.h"
#include "../engine/Timer.h"
#include "../engine/MacroUtils.h"
#include "../engine/Log.h"

#include <cstdlib>
#include <ctime>
#include <GL/glew.h>


Scene_024_TestingBubble::Scene_024_TestingBubble() : totalTime(0), frameIndex(0) {

}

Scene_024_TestingBubble::~Scene_024_TestingBubble() {
    clean();
}

void Scene_024_TestingBubble::setGame(Game *_game) {
    game = _game;
}

void Scene_024_TestingBubble::clean() {

}

void Scene_024_TestingBubble::pause() {
}

void Scene_024_TestingBubble::resume() {
}

void Scene_024_TestingBubble::handleEvent(const InputState &inputState) {

}

void Scene_024_TestingBubble::load() {
    Assets::loadComputeShader(SHADER_COMP(SHADER_NAME), SHADER_ID(SHADER_NAME));
    Assets::loadShader(SHADER_VERT(SHADER_NAME), SHADER_FRAG(SHADER_NAME), "", "", "", SHADER_ID(SHADER_NAME));


    // This is position and normal data for a paper airplane
    static const Vector3 geometry[] =
    {
        // Positions
   // Positions
        Vector3(1.0f,  1.0f,  1.0f),
        Vector3(-1.0f,  1.0f,  1.0f),
        Vector3(1.0f, -1.0f,  1.0f),
        Vector3(-1.0f, -1.0f,  1.0f),
        Vector3(1.0f,  1.0f, -1.0f),
        Vector3(-1.0f,  1.0f, -1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, -1.0f),

        // Normals
        Vector3(-2.0f, 0.0f,  0.0f),
        Vector3(2.0f, 2.0f,  0.0f),
        Vector3(-2.0f, 0.0f,  0.0f),
        Vector3(2.0f, 2.0f, -2.0f),
        Vector3(-2.0f, 0.0f, -2.0f),
        Vector3(2.0f, 2.0f, -2.0f),
        Vector3(-2.0f, 0.0f, -2.0f),
        Vector3(2.0f, 2.0f,  0.0f),
    };

    computeShader = Assets::getComputeShader(SHADER_ID(SHADER_NAME));
    Log t();
    renderShader = Assets::getShader(SHADER_ID(SHADER_NAME));

    glGenBuffers(2, flockBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, flockBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);

    int i;

    glGenBuffers(1, &geometryBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, geometryBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

    glGenVertexArrays(2, flockRenderVao);

    for (i = 0; i < 2; i++)
    {
        glBindVertexArray(flockRenderVao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, geometryBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(8 * sizeof(Vector3)));

        glBindBuffer(GL_ARRAY_BUFFER, flockBuffer[i]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), NULL);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), (void *)sizeof(Vector4));
        glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, flockBuffer[0]);
    flock_member * ptr = reinterpret_cast<flock_member *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, FLOCK_SIZE * sizeof(flock_member), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
    const float PI = 3.14159265;
    radius = 300;
    for (i = 0; i < FLOCK_SIZE; i++)
    {
        float theta = 2 * PI * randomFloat();
        float phi = acos(1 - 2 * randomFloat());

        ptr[i].position = Vector3(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)) * radius;
        ptr[i].velocity = Vector3(0.0f,0.0f,0.0f);
    }
    
    glUnmapBuffer(GL_ARRAY_BUFFER);
    

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}
void Scene_024_TestingBubble::update(float dt) {
    totalTime += dt;

    computeShader.use();

    Vector3 center = Vector3(0.0f, 0.0f, 0.0f);

    computeShader.setVector3f("center", center);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flockBuffer[frameIndex]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flockBuffer[frameIndex ^ 1]);

    glDispatchCompute(NUM_WORKGROUPS, 1, 1);
}

void Scene_024_TestingBubble::draw()
{
    renderShader.use();

    Matrix4 mv_matrix = Matrix4::createLookAt(Vector3(0.0f, 0.0f, -600.0f),
                                            Vector3(0.0f, 0.0f, 0.0f),
                                            Vector3(0.0f, 1.0f, 0.0f));
    Matrix4 proj_matrix = Matrix4::createPerspectiveFOV(60.0f, game->windowWidth, game->windowHeight, 0.1f, 3000.0f);
    Matrix4 mvp = proj_matrix * mv_matrix;

    renderShader.setMatrix4("mvp", mvp);

    glBindVertexArray(flockRenderVao[frameIndex]);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

    frameIndex ^= 1;
}
