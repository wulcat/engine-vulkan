//
// Created by Akira Mujawar on 22/06/24.
//

#include "opengl_grid_pipeline.hpp"

using physicat::pipeline::OpenGLGridPipeline;

OpenGLGridPipeline::OpenGLGridPipeline(const GLuint &shaderProgramID)
        : ShaderProgramID(shaderProgramID) {
    glGenVertexArrays(1, &VertexArrayID);
}

OpenGLGridPipeline::~OpenGLGridPipeline() {
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(ShaderProgramID);
}

void OpenGLGridPipeline::Render(
        const physicat::OpenGLAssetManager &assetManager,
        const physicat::entity::RenderComponentBase *renderComponent,
        const physicat::entity::Transform3DComponent* transform3DComponent,
        const physicat::PerspectiveCamera* camera) const {

    glUseProgram(ShaderProgramID);

    glUniformMatrix4fv(glGetUniformLocation(ShaderProgramID, "u_view"), 1, GL_FALSE, &camera->GetViewMatrix()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProgramID, "u_projection"), 1, GL_FALSE, &camera->GetProjectionMatrix()[0][0]);

    glBindVertexArray(VertexArrayID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}