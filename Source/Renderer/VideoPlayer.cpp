#include "VideoPlayer.h"
#include "Renderer.h"
#include <SDL.h>
#include <iostream>

// FFmpeg 4.0+ não precisa mais de av_register_all(), mas vamos garantir compatibilidade

VideoPlayer::VideoPlayer()
    : mFormatContext(nullptr)
    , mCodecContext(nullptr)
    , mFrame(nullptr)
    , mFrameRGB(nullptr)
    , mPacket(nullptr)
    , mSwsContext(nullptr)
    , mVideoStreamIndex(-1)
    , mTextureID(0)
    , mWidth(0)
    , mHeight(0)
    , mCurrentTime(0.0)
    , mDuration(0.0)
    , mFrameTime(0.0)
    , mAccumulatedTime(0.0)
    , mFinished(false)
    , mLoaded(false)
    , mFrameBuffer(nullptr)
{
}

VideoPlayer::~VideoPlayer()
{
    Unload();
}

bool VideoPlayer::Load(const std::string& fileName)
{
    Unload();
    
    // Abrir arquivo de vídeo
    if (avformat_open_input(&mFormatContext, fileName.c_str(), nullptr, nullptr) != 0)
    {
        SDL_Log("Erro ao abrir arquivo de vídeo: %s", fileName.c_str());
        return false;
    }
    
    // Encontrar informações do stream
    if (avformat_find_stream_info(mFormatContext, nullptr) < 0)
    {
        SDL_Log("Erro ao encontrar informações do stream");
        return false;
    }
    
    // Encontrar stream de vídeo
    mVideoStreamIndex = -1;
    for (unsigned int i = 0; i < mFormatContext->nb_streams; i++)
    {
        if (mFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            mVideoStreamIndex = i;
            break;
        }
    }
    
    if (mVideoStreamIndex == -1)
    {
        SDL_Log("Nenhum stream de vídeo encontrado");
        return false;
    }
    
    // Obter codec
    AVCodecParameters* codecpar = mFormatContext->streams[mVideoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec)
    {
        SDL_Log("Codec não encontrado");
        return false;
    }
    
    // Criar context do codec
    mCodecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(mCodecContext, codecpar) < 0)
    {
        SDL_Log("Erro ao copiar parâmetros do codec");
        return false;
    }
    
    // Abrir codec
    if (avcodec_open2(mCodecContext, codec, nullptr) < 0)
    {
        SDL_Log("Erro ao abrir codec");
        return false;
    }
    
    mWidth = mCodecContext->width;
    mHeight = mCodecContext->height;
    
    // Calcular duração
    AVRational timeBase = mFormatContext->streams[mVideoStreamIndex]->time_base;
    int64_t duration = mFormatContext->streams[mVideoStreamIndex]->duration;
    mDuration = av_q2d(timeBase) * duration;
    
    // Calcular frame time
    AVRational frameRate = mFormatContext->streams[mVideoStreamIndex]->avg_frame_rate;
    mFrameTime = av_q2d(av_inv_q(frameRate));
    
    // Alocar frames
    mFrame = av_frame_alloc();
    mFrameRGB = av_frame_alloc();
    if (!mFrame || !mFrameRGB)
    {
        SDL_Log("Erro ao alocar frames");
        return false;
    }
    
    // Alocar buffer para frame RGBA (mais compatível com OpenGL)
    // Usar AV_PIX_FMT_RGBA que é nativo do OpenGL
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, mWidth, mHeight, 1);
    mFrameBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    if (!mFrameBuffer)
    {
        SDL_Log("Erro ao alocar buffer de frame");
        return false;
    }
    av_image_fill_arrays(mFrameRGB->data, mFrameRGB->linesize, mFrameBuffer, AV_PIX_FMT_RGBA, mWidth, mHeight, 1);
    
    // Criar contexto de conversão para RGBA
    // Usar flags para melhor qualidade e garantir conversão correta
    mSwsContext = sws_getContext(
        mWidth, mHeight, mCodecContext->pix_fmt,
        mWidth, mHeight, AV_PIX_FMT_RGBA,
        SWS_BILINEAR | SWS_ACCURATE_RND, nullptr, nullptr, nullptr
    );
    
    if (!mSwsContext)
    {
        SDL_Log("Erro ao criar contexto de conversão de vídeo");
        return false;
    }
    
    // Criar textura OpenGL com formato RGBA
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    mPacket = av_packet_alloc();
    mCurrentTime = 0.0;
    mAccumulatedTime = 0.0;
    mFinished = false;
    mLoaded = true;
    
    // Decodificar primeiro frame
    DecodeFrame();
    
    return true;
}

void VideoPlayer::Unload()
{
    if (mTextureID != 0)
    {
        glDeleteTextures(1, &mTextureID);
        mTextureID = 0;
    }
    
    if (mFrameBuffer)
    {
        av_free(mFrameBuffer);
        mFrameBuffer = nullptr;
    }
    
    if (mSwsContext)
    {
        sws_freeContext(mSwsContext);
        mSwsContext = nullptr;
    }
    
    if (mFrame)
    {
        av_frame_free(&mFrame);
        mFrame = nullptr;
    }
    
    if (mFrameRGB)
    {
        av_frame_free(&mFrameRGB);
        mFrameRGB = nullptr;
    }
    
    if (mCodecContext)
    {
        avcodec_free_context(&mCodecContext);
        mCodecContext = nullptr;
    }
    
    if (mFormatContext)
    {
        avformat_close_input(&mFormatContext);
        mFormatContext = nullptr;
    }
    
    if (mPacket)
    {
        av_packet_free(&mPacket);
        mPacket = nullptr;
    }
    
    mLoaded = false;
    mFinished = false;
    mWidth = 0;
    mHeight = 0;
}

bool VideoPlayer::Update(float deltaTime)
{
    if (!mLoaded || mFinished)
        return false;
    
    mAccumulatedTime += deltaTime;
    
    // Se passou tempo suficiente para o próximo frame
    if (mAccumulatedTime >= mFrameTime)
    {
        mAccumulatedTime -= mFrameTime;
        mCurrentTime += mFrameTime;
        
        if (mCurrentTime >= mDuration)
        {
            mFinished = true;
            return false;
        }
        
        // Decodificar próximo frame
        if (!DecodeFrame())
        {
            mFinished = true;
            return false;
        }
    }
    
    return true;
}

bool VideoPlayer::DecodeFrame()
{
    while (av_read_frame(mFormatContext, mPacket) >= 0)
    {
        if (mPacket->stream_index == mVideoStreamIndex)
        {
            // Enviar packet para decoder
            if (avcodec_send_packet(mCodecContext, mPacket) < 0)
            {
                av_packet_unref(mPacket);
                continue;
            }
            
            // Receber frame decodificado
            int ret = avcodec_receive_frame(mCodecContext, mFrame);
            if (ret == 0)
            {
                // Converter para RGBA (com flip vertical se necessário)
                // FFmpeg geralmente tem Y=0 no topo, OpenGL tem Y=0 embaixo
                sws_scale(mSwsContext,
                    mFrame->data, mFrame->linesize, 0, mHeight,
                    mFrameRGB->data, mFrameRGB->linesize);
                
                // Atualizar textura
                UpdateTexture();
                
                av_packet_unref(mPacket);
                return true;
            }
            else if (ret == AVERROR(EAGAIN))
            {
                av_packet_unref(mPacket);
                continue;
            }
        }
        
        av_packet_unref(mPacket);
    }
    
    return false;
}

void VideoPlayer::UpdateTexture()
{
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    
    // Configurar alinhamento de pixels correto para RGBA
    // RGBA = 4 bytes por pixel, então o pitch deve ser divisível por 4
    int pitch = mFrameRGB->linesize[0];
    int expectedPitch = mWidth * 4; // 4 bytes por pixel (RGBA)
    
    // Se o pitch for diferente do esperado, precisamos usar GL_UNPACK_ROW_LENGTH
    if (pitch != expectedPitch)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 4);
    }
    else
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0); // Usar width padrão
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // RGBA precisa de alinhamento de 4 bytes
    
    // Atualizar textura com dados RGBA
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mFrameRGB->data[0]);
    
    // Restaurar alinhamento padrão
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void VideoPlayer::SeekToTime(double timeInSeconds)
{
    if (!mLoaded)
        return;
    
    AVRational timeBase = mFormatContext->streams[mVideoStreamIndex]->time_base;
    int64_t timestamp = (int64_t)(timeInSeconds / av_q2d(timeBase));
    
    // Fazer seek no arquivo
    if (av_seek_frame(mFormatContext, mVideoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD) >= 0)
    {
        // Limpar buffers do codec
        avcodec_flush_buffers(mCodecContext);
        
        // Atualizar tempo atual
        mCurrentTime = timeInSeconds;
        mAccumulatedTime = 0.0;
        mFinished = false;
        
        // Decodificar frame na nova posição
        DecodeFrame();
    }
}

void VideoPlayer::Render(class Renderer* renderer)
{
    // Esta função será implementada para renderizar a textura na tela
    // Por enquanto, a textura será renderizada através de uma UIImage
}

