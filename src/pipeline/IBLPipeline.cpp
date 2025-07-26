#include "pipeline/IBLPipeline.h"
#include "resource/ResourceManager.h"
#include "assets/VerticesData.h"
#include <iostream>
#include <stb_image.h>

namespace pipeline {

IBLPipeline::IBLPipeline(std::shared_ptr<graphics::Shader> pbrShader,
                         std::shared_ptr<graphics::Shader> irradianceShader,
                         std::shared_ptr<graphics::Shader> prefilterShader,
                         std::shared_ptr<graphics::Shader> brdfShader,
                         std::shared_ptr<graphics::Shader> backgroundShader,
                         std::shared_ptr<graphics::Shader> equirectangular2cubemapShader,
                            const std::string& equirectangularMapPath,
                         std::shared_ptr<core::Window> windowPtr)
                        :m_PBRShader(std::move(pbrShader)),
                        m_IrradianceShader(std::move(irradianceShader)),
                        m_PrefilterShader(std::move(prefilterShader)),
                        m_BRDFShader(std::move(brdfShader)),
                        m_BackgroundShader(std::move(backgroundShader)),
                        m_Equirectangular2CubemapShader(std::move(equirectangular2cubemapShader)),
                        m_WindowPtr(std::move(windowPtr)) {
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    m_WindowPtr->GetSize(m_Width, m_Height);

    // setup cube VAO and VBO
    glGenBuffers(1, &m_cubeVBO);
    glGenVertexArrays(1, & m_cubeVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
    
    
    // setup quad VAO and VBO
    glGenBuffers(1, &m_quadVBO);
    glGenVertexArrays(1, &m_quadVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    m_PBRShader->Bind();
    m_PBRShader->SetUniform("albedoMap", 0);
    m_PBRShader->SetUniform("normalMap", 1);
    m_PBRShader->SetUniform("metallicMap", 2);
    m_PBRShader->SetUniform("roughnessMap", 3);
    m_PBRShader->SetUniform("aoMap", 4);
    m_PBRShader->SetUniform("irradianceMap", 5);
    m_PBRShader->SetUniform("prefilterMap", 6);
    m_PBRShader->SetUniform("brdfLUT", 7);
    m_PBRShader->Unbind();

    
    glGenFramebuffers(1, &m_captureFBO);
    glGenRenderbuffers(1, &m_captureRBO);

    // setup environment map                       
    SetupEnvironmentMap(equirectangularMapPath);
                       
    // generate irradiance map
    GenerateIrradianceMap();

    // generate prefilter map
    GeneratePrefilterMap();

    // generate BRDF LUT 
    GenerateBRDFLUT();     
}

void IBLPipeline::Render(const std::shared_ptr<scene::Scene>& scene,
                        const std::shared_ptr<graphics::Camera>& camera) {
    m_WindowPtr->GetSize(m_Width, m_Height);
    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_PBRShader->Bind();
    // set camera uniforms
    m_PBRShader->SetUniform("projection", camera->GetProjectionMatrix());
    m_PBRShader->SetUniform("view", camera->GetViewMatrix());
    m_PBRShader->SetUniform("camPos", camera->GetPosition());

    // set light
    const auto& lights = scene->GetLights();
    m_PBRShader->SetUniform("lightCount", static_cast<int>(lights.size()));
    for (size_t i = 0; i < lights.size(); ++i) {
        const auto& light = lights[i];
        m_PBRShader->SetUniform("lightPositions[" + std::to_string(i) + "]", light->GetPosition());
        m_PBRShader->SetUniform("lightColors[" + std::to_string(i) + "]", light->GetColor());
    }

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, m_brdfLUT);
    // render entities
    for (const auto& entity : scene->GetEntities()) {
        m_PBRShader->SetUniform("model", entity->GetModelMatrix());
        //entity->Draw();
    }
    m_PBRShader->Unbind();

    // render environment map
    m_BackgroundShader->Bind();
    m_BackgroundShader->SetUniform("environmentMap", 0);
    m_BackgroundShader->SetUniform("projection", camera->GetProjectionMatrix());
    m_BackgroundShader->SetUniform("view", camera->GetViewMatrix());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    RenderCube(); // render the cube with the background shader
    m_BackgroundShader->Unbind();
}

void IBLPipeline::GenerateBRDFLUT() {
    glGenTextures(1, &m_brdfLUT);
    glBindTexture(GL_TEXTURE_2D, m_brdfLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // use 24-bit depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brdfLUT, 0);
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_BRDFShader->Bind();
    RenderQuad(); // render the quad with the BRDF shader
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_BRDFShader->Unbind();
}

void IBLPipeline::GeneratePrefilterMap() {
    glGenTextures(1, &m_prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    m_PrefilterShader->Bind();
    m_PrefilterShader->SetUniform("environmentMap", 0);
    m_PrefilterShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    unsigned int maxMipLevels = 5; // 128x128 -> 64x64 -> 32x32 -> 16x16 -> 8x8
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        m_PrefilterShader->SetUniform("roughness", static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1));
        for (unsigned int i = 0; i < 6; ++i) {
            m_PrefilterShader->SetUniform("view", m_captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_PrefilterShader->Unbind();
}


void IBLPipeline::GenerateIrradianceMap(){
    glGenTextures(1, &m_irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32); // use 24-bit depth buffer

    // render to irradiance map
    m_IrradianceShader->Bind();
    m_IrradianceShader->SetUniform("environmentMap", 0);
    m_IrradianceShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glViewport(0, 0, 32, 32);
    for (unsigned int i = 0; i < 6; ++i) {
        m_IrradianceShader->SetUniform("view", m_captureViews[i]); // set view matrix for each face
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube(); // render the cube to each face of the cubemap
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_IrradianceShader->Unbind();
}


void IBLPipeline::SetupEnvironmentMap(const std::string& equirectangularMapPath) {
    
    // load equirectangular map texture
    stbi_set_flip_vertically_on_load(true);
    m_equirectangularMap = core::ResourceManager::LoadTexture(equirectangularMapPath, false, true);
    if (m_equirectangularMap == 0) {
        std::cerr << "Failed to load equirectangular map: " << equirectangularMapPath << std::endl;
        return;
    }


    // create cubemap for environment map for HDR rendering
    glGenTextures(1, &m_envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // use mipmaps for better quality
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear filtering
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // clamp to edge to avoid artifacts
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // create framebuffer and renderbuffer for capturing equirectangular map to cubemap
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // use 24-bit depth buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO); // attach depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup shaders for converting equirectangular map to cubemap
    m_Equirectangular2CubemapShader->Bind();
    m_Equirectangular2CubemapShader->SetUniform("equirectangularMap", 0);
    m_Equirectangular2CubemapShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    //glActiveTexture(GL_TEXTURE0);
    m_equirectangularMap->Bind(0); // bind equirectangular map texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glViewport(0, 0, 512, 512);
    // render to cubemap
    for (unsigned int i = 0; i < 6; ++i) {
        m_Equirectangular2CubemapShader->SetUniform("view", m_captureViews[i]); // set view matrix for each face
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube(); // render the cube to each face of the cubemap
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Equirectangular2CubemapShader->Unbind();

    // generate mipmaps for the cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

IBLPipeline::~IBLPipeline() {
    glDepthFunc(GL_LESS); // reset depth function
    glDeleteTextures(1, &m_envCubemap);
    glDeleteTextures(1, &m_irradianceMap);
    glDeleteTextures(1, &m_prefilterMap);
    glDeleteTextures(1, &m_brdfLUT);
    glDeleteFramebuffers(1, &m_captureFBO);
    glDeleteRenderbuffers(1, &m_captureRBO);
    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_cubeVBO);
}

void IBLPipeline::RenderCube(){
    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void IBLPipeline::RenderQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

} // namespace pipeline