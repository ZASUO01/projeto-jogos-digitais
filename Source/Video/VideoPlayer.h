//
// VideoPlayer simplificado - independente do jogo
//

#pragma once
#include <SDL.h>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    // Carrega e reproduz um vídeo de forma autônoma
    // Retorna true se o vídeo foi carregado com sucesso
    bool PlayVideo(const std::string& filePath, SDL_Window* window, bool loop = false);

    // Executa o loop do vídeo até que termine ou seja interrompido
    // Retorna true se o vídeo terminou normalmente, false se foi interrompido
    bool Run();

    // Atualiza o vídeo (para uso em loop externo)
    void Update();

    // Renderiza o frame atual (para uso em loop externo)
    void Render();

    // Verifica se o vídeo está reproduzindo
    bool IsPlaying() const { return mIsPlaying; }

    // Para a reprodução
    void Stop();

    // Limpa recursos
    void Shutdown();

private:
    bool LoadVideo(const std::string& filePath);
    bool DecodeFrame();
    void Cleanup();

    SDL_Window* mWindow;
    SDL_Renderer* mSDLRenderer;
    SDL_Texture* mVideoTexture;
    
    // FFmpeg structures
    AVFormatContext* mFormatContext;
    AVCodecContext* mCodecContext;
    AVFrame* mFrame;
    AVFrame* mFrameRGB;
    AVPacket* mPacket;
    struct SwsContext* mSwsContext;
    uint8_t* mBuffer;
    
    int mVideoStreamIndex;
    bool mIsPlaying;
    bool mLoop;
    bool mVideoLoaded;
    
    // Timing
    double mFrameTime;
    double mLastFrameTime;
    
    // Dimensões do vídeo
    int mVideoWidth;
    int mVideoHeight;
};

