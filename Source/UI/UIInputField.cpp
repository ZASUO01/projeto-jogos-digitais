#include "UIInputField.h"
#include "../Game.h"
#include "../Math.h"
#include "../Renderer/Texture.h"
#include <SDL2/SDL.h>


UIInputField::UIInputField(Game* game, const Vector2 &offset, class Font* font,
                           float scale, int pointSize, int drawOrder)
    : UIText(game, "", font, offset, scale, 0.0f, pointSize, 1024, drawOrder)
    , mIsFocused(false)
    , mCursorTimer(0.0f)
{
    // Criar o UIText para o cursor
    mCursor = new UIText(game, "|", font, offset, scale, 0.0f, pointSize, 1024, drawOrder + 1);
    mCursor->SetIsVisible(false); // Inicia invisível

    // Configurações visuais iniciais (Texto inicial vazio, fundo escuro)
    SetTextColor(Vector3(1.0f, 1.0f, 1.0f)); // Cor do texto (Branco)
    mBackgroundColor = Vector4(0.1f, 0.1f, 0.1f, 1.0f); // Cor do fundo (Cinza escuro)
}

UIInputField::~UIInputField()
{
    delete mCursor;
}

void UIInputField::SetFocused(bool focused)
{
    mIsFocused = focused;
    mCursorTimer = 0.0f;
    mCursor->SetIsVisible(focused); // Mostra cursor se focado
}

void UIInputField::HandleKey(int key)
{
    if (!mIsFocused) return;

    // Lógica para Backspace
    if (key == SDLK_BACKSPACE || key == SDLK_DELETE) {
        if (!mTextValue.empty()) {
            mTextValue.pop_back();
        }
    }
    // Lógica para entrada de dígitos (0-9)
    else if (key >= SDLK_0 && key <= SDLK_9) {
        if (mTextValue.length() < MAX_IPV4_LENGTH) {
            char digit = (char)(key - SDLK_0 + '0');
            mTextValue += digit;
        }
    }
    // Lógica para Ponto ('.')
    else if (key == SDLK_PERIOD) {
        // Permitir o ponto, desde que não seja o primeiro caractere e haja espaço
        if (!mTextValue.empty() && mTextValue.length() < MAX_IPV4_LENGTH && mTextValue.back() != '.') {
             mTextValue += '.';
        }
    }

    // Se o texto mudou, atualiza o UIText base
    UIText::SetText(mTextValue);

    // Reseta o timer do cursor para que ele pisque corretamente
    mCursorTimer = 0.0f;
}

void UIInputField::Draw(class Shader* shader)
{
    // 1. Desenha o fundo e o texto (chamando a Draw da classe base)
    UIText::Draw(shader);

    // Se o elemento estiver focado, desenha o cursor piscando
    if (mIsFocused) {
        // Simular um update para o cursor (alternar a visibilidade)
        // Isso normalmente seria feito em um método Update, mas simplificamos aqui.
        float blinkRate = 0.5f; // Pisca a cada 0.5 segundos

        // Simulação do tempo: idealmente você passaria o deltaTime aqui.
        // Já que a UIElement não tem Update, faremos o controle de visibilidade
        // no HandleKey (para resetar) e contamos que a Draw seja frequente.

        // Simulação do posicionamento do cursor:
        // Posição inicial (offset) + Largura do texto atual.
        float textWidth = 0.0f;
        if (mTexture) {
            textWidth = (float)mTexture->GetWidth() * mScale;
        }

        Vector2 cursorOffset = GetOffset();

        cursorOffset.x += textWidth + 5.0f * mScale;

        mCursor->SetOffset(cursorOffset);

        mCursor->Draw(shader);
    }
}