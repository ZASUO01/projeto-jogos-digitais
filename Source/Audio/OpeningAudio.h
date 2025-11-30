#pragma once
#include <SDL_mixer.h>
#include <string>

class OpeningAudio
{
public:
    OpeningAudio();
    ~OpeningAudio();

    bool Initialize();
    void Shutdown();

    // Tocar áudios correspondentes aos vídeos
    bool PlayBegin(bool loop = true);      // begin.wav com begin.mp4
    bool PlayAbertura(bool loop = false);   // abertura.wav com abertura.mp4
    bool PlayLoop(bool loop = true);        // loop.wav com entrance_loop.mp4

    void Stop();
    void StopBegin();
    void StopAbertura();
    void StopLoop();

    bool IsPlaying() const;
    bool IsBeginPlaying() const;
    bool IsAberturaPlaying() const;
    bool IsLoopPlaying() const;

    void Update(); // Verifica se precisa fazer loop

private:
    Mix_Chunk* mBeginChunk;
    Mix_Chunk* mAberturaChunk;
    Mix_Chunk* mLoopChunk;
    
    int mBeginChannel;
    int mAberturaChannel;
    int mLoopChannel;

    bool mBeginLoop;
    bool mAberturaLoop;
    bool mLoopLoop;

    bool mBeginPlaying;
    bool mAberturaPlaying;
    bool mLoopPlaying;

    bool mInitialized;
};

