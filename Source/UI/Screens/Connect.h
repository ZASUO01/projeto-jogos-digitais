//
// Created by pedro-souza on 09/12/2025.
//
#pragma once

#include "UIScreen.h"

class Connect : public UIScreen
{
public:
    Connect(class Game* game, const std::string& fontName);

    void HandleKeyPress(int key) override;
    void Update(float deltaTime) override;

private:
    UIInputField* mInputField;
    UIText* mConnectionText;
    static bool IsValidIPv4(const std::string& ipString);
    bool mConnecting;
};
