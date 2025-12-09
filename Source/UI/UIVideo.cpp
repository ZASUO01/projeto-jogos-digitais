#include "UIVideo.h"
#include "../Renderer/VideoPlayer.h"
#include "../Renderer/Shader.h"
#include "../Game.h"
#include "../Math.h"
#include <GL/glew.h>

UIVideo::UIVideo(class Game* game, class VideoPlayer* videoPlayer, const Vector2& offset, float scale, float angle, int drawOrder)
    : UIImage(game, offset, scale, angle, drawOrder)
    , mVideoPlayer(videoPlayer)
    , mVideoTextureID(0)
{
    if (mVideoPlayer)
    {
        mVideoTextureID = mVideoPlayer->GetTextureID();
    }
}

void UIVideo::Draw(class Shader* shader)
{
    if (!mVideoPlayer || !GetIsVisible())
        return;
    
    // Usar a textura do vídeo ao invés da textura normal
    unsigned int textureID = mVideoPlayer->GetTextureID();
    if (textureID == 0)
        return;
    
    // Scale the quad by the width/height of video
    int videoWidth = mVideoPlayer->GetWidth();
    int videoHeight = mVideoPlayer->GetHeight();
    
    Matrix4 scaleMat = Matrix4::CreateScale(static_cast<float>(videoWidth) * GetScale(),
                                            static_cast<float>(videoHeight) * GetScale(), 1.0f);
    
    Matrix4 rotMat = Matrix4::CreateRotationZ(GetAngle());
    
    // Translate to position on screen
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(GetOffset().x, GetOffset().y, 0.0f));
    
    // Set world transform
    Matrix4 world = scaleMat * rotMat * transMat;
    shader->SetMatrixUniform("uWorldTransform", world);
    
    // Set uTextureFactor
    shader->SetFloatUniform("uTextureFactor", 1.0f);
    
    // Set color (sempre branco para vídeo)
    shader->SetVectorUniform("uBaseColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    
    // Set current texture and texture uniform (usando a textura do vídeo)
    shader->SetTextureUniform("uTexture", textureID, 0);
    
    // Draw quad (usar os mesmos vertices do sprite que já estão ativos)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

