#include "renderPass/IBLPass.h"
#include "resource/ResourceManager.h"
#include "assets/VerticesData.h"
#include "utils/PathResolver.h"
#include "stb_image.h"
#include <iostream>

namespace pipeline {
IBLPass::IBLPass(std::shared_ptr<core::Window> windowPtr,
                 const std::string& equirectangularMapPath)
    : m_WindowPtr(std::move(windowPtr)),
      m_EquirectangularMapPath(equirectangularMapPath) {

    // 启用无缝立方体贴图采样
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    m_IrradianceShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/ibl_irradiance.vert"),
        PathResolver::Resolve("shaders/alpha/ibl_irradiance.frag")
    );
    m_PrefilterShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/ibl_prefilter.vert"),
        PathResolver::Resolve("shaders/alpha/ibl_prefilter.frag")
    );
    m_BRDFShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/ibl_brdf.vert"),
        PathResolver::Resolve("shaders/alpha/ibl_brdf.frag")
    );
    m_Equirectangular2CubemapShader = core::ResourceManager::LoadShader(
        PathResolver::Resolve("shaders/alpha/ibl_equirectangular2Cubemap.vert"),
        PathResolver::Resolve("shaders/alpha/ibl_equirectangular2Cubemap.frag")
    );


    // setup cube VAO and VBO
    glGenBuffers(1, &m_CubeVBO);
    glGenVertexArrays(1, & m_CubeVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_CubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    

    // setup quad VAO and VBO
    glGenBuffers(1, &m_QuadVBO);
    glGenVertexArrays(1, &m_QuadVAO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 创建捕获帧缓冲和渲染缓冲
    glGenFramebuffers(1, &m_CaptureFBO);
    glGenTextures(1, &m_CaptureRBO);

}

void IBLPass::Execute(const std::shared_ptr<scene::Scene>& scene) {
    // 设置OpenGL状态
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    RenderEnvironmentMap();
    RenderIrradianceMap();
    RenderPrefilterMap();
    RenderBRDFLUT();
    stbi_set_flip_vertically_on_load(false);
}


void IBLPass::RenderEnvironmentMap() {
    // load equirectangular map texture
    stbi_set_flip_vertically_on_load(true);
    m_EquirectangularMap = core::ResourceManager::LoadTexture(m_EquirectangularMapPath, false, true);
    if (m_EquirectangularMap == 0) {
        std::cerr << "Failed to load equirectangular map: " << m_EquirectangularMapPath << std::endl;
        return;
    }


    // create cubemap for environment map for HDR rendering
    glGenTextures(1, &m_EnvCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
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
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // use 24-bit depth buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRBO); // attach depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup shaders for converting equirectangular map to cubemap
    m_Equirectangular2CubemapShader->Bind();
    m_Equirectangular2CubemapShader->SetUniform("equirectangularMap", 0);
    m_Equirectangular2CubemapShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    //glActiveTexture(GL_TEXTURE0);
    m_EquirectangularMap->Bind(0); // bind equirectangular map texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glViewport(0, 0, 512, 512);
    // render to cubemap
    for (unsigned int i = 0; i < 6; ++i) {
        m_Equirectangular2CubemapShader->SetUniform("view", m_CaptureViews[i]); // set view matrix for each face
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_EnvCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube(); // render the cube to each face of the cubemap
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_Equirectangular2CubemapShader->Unbind();

    // generate mipmaps for the cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void IBLPass::RenderIrradianceMap() {
    glGenTextures(1, &m_IrradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32); // use 24-bit depth buffer
    m_IrradianceShader->Bind();
    m_IrradianceShader->SetUniform("environmentMap", 0);
    m_IrradianceShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    glViewport(0, 0, 32, 32);
    // render to irradiance map
    for (unsigned int i = 0; i < 6; ++i) {
        m_IrradianceShader->SetUniform("view", m_CaptureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_IrradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderCube();
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_IrradianceShader->Unbind();
}

void IBLPass::RenderPrefilterMap() {
    glGenTextures(1, &m_PrefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    m_PrefilterShader->Bind();
    m_PrefilterShader->SetUniform("environmentMap", 0);
    m_PrefilterShader->SetUniform("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);

    unsigned int maxMipLevels = 5; // 128x128 -> 64x64 -> 32x32 -> 16x16 -> 8x8
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);
        m_PrefilterShader->SetUniform("roughness", static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1));
        for (unsigned int i = 0; i < 6; ++i) {
            m_PrefilterShader->SetUniform("view", m_CaptureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_PrefilterShader->Unbind();
}

void IBLPass::RenderBRDFLUT() {
    glGenTextures(1, &m_BRDFLUT);
    glBindTexture(GL_TEXTURE_2D, m_BRDFLUT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUT, 0);

    m_BRDFShader->Bind();
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_BRDFShader->Unbind();
}

void IBLPass::BindMap() {
    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_ENV_CUBEMAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvCubemap);
    
    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_IRRADIANCE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceMap);

    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_PREFILTER);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);

    glActiveTexture(GL_TEXTURE0 + graphics::TextureSlots::TEX_SLOT_BRDF_LUT);
    glBindTexture(GL_TEXTURE_2D, m_BRDFLUT);
}

void IBLPass::RenderCube() {
    glBindVertexArray(m_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void IBLPass::RenderQuad() {
    glBindVertexArray(m_QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

IBLPass::~IBLPass() {
    glDeleteFramebuffers(1, &m_CaptureFBO);
    glDeleteRenderbuffers(1, &m_CaptureRBO);
    glDeleteVertexArrays(1, &m_CubeVAO);
    glDeleteBuffers(1, &m_CubeVBO);
    glDeleteVertexArrays(1, &m_QuadVAO);
    glDeleteBuffers(1, &m_QuadVBO);
    glDeleteTextures(1, &m_EnvCubemap);
    glDeleteTextures(1, &m_IrradianceMap);
    glDeleteTextures(1, &m_PrefilterMap);
    glDeleteTextures(1, &m_BRDFLUT);
}



}

