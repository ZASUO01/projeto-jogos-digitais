#pragma once

#include "UIText.h"
#include <string>
#include <functional>

class UIInputField : public UIText
{
public:
    UIInputField(class Game* game, const Vector2 &offset, class Font* font,
                 float scale = 1.0f, int pointSize = 40, int drawOrder = 100);
    ~UIInputField();

    // Novo método para lidar com a entrada de teclas.
    // É necessário mapear o keycode do seu motor (SDL, GLFW) para um int aqui.
    void HandleKey(int key);
    void SetCursorColor(const Vector3& color) const { if (mCursor) {mCursor->SetTextColor(color);}}

    // Método para desenhar o fundo (se necessário) e o cursor
    void Draw(class Shader* shader) override;

    // Getters e Setters
    const std::string& GetTextValue() const { return mTextValue; }
    void SetFocused(bool focused);
    bool IsFocused() const { return mIsFocused; }

    // Constante para o tamanho máximo (Ex: "255.255.255.255" = 15 caracteres)
    static constexpr size_t MAX_IPV4_LENGTH = 15;

private:
    std::string mTextValue;
    bool mIsFocused;
    float mCursorTimer;

    // Objeto de texto para o cursor ( "|")
    class UIText* mCursor;
};