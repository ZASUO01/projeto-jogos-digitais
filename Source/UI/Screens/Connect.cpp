//
// Created by Lucas N. Ferreira on 06/11/25.
//

#include "Connect.h"
#include "../../Game.h"
#include "../../Math.h"
#include <SDL.h>
#include <sstream>
#include <algorithm>
#include <iostream>

Connect::Connect(class Game* game, const std::string& fontName)
        :UIScreen(game, fontName), mConnecting(false)
{
        mInputField = AddInput(Vector2(0.0f, -80.0f));
        mInputField->SetIsVisible(true);
        mInputField->SetFocused(true);
        mInputField->SetCursorColor(Vector3(1.0f, 1.0f, 1.0f));

        mConnectionText = AddText("Connecting...",
               Vector2(0.0f, -80.0f));
        mConnectionText->SetIsVisible(false);
}

void Connect::HandleKeyPress(const int key)
{
        if ((key ==  SDLK_RETURN || key == SDLK_RETURN2) && !mConnecting) {
                const std::string ip = mInputField->GetTextValue();
                if (!IsValidIPv4(ip)) {
                        return;
                }

                if (!mGame->GetClient()->AddServerAddr(ip.c_str())) {
                        return;
                }

                mConnecting = true;
                mInputField->SetFocused(false);
                mInputField->SetIsVisible(false);
                mConnectionText->SetIsVisible(true);
                mGame->GetClient()->StartConnection();
        }else {
                mInputField->HandleKey(key);
        }
}

void Connect::Update(float deltaTime) {
        if (mConnecting) {
                if (const ConnectionStatus status = mGame->GetClient()->CheckConnection(); status != ConnectionStatus::IN_PROGRESS) {
                        if (status == ConnectionStatus::SUCCESS) {
                                Close();
                                mGame->SetScene(GameScene::Multiplayer);
                        }else {
                                Close();
                                mGame->SetScene(GameScene::MainMenu);
                        }
                }
        }
}

bool Connect::IsValidIPv4(const std::string &ipString)  {
        if (ipString.empty()) {
                return false;
        }

        std::stringstream ss(ipString);
        std::string segment;
        int octetCount = 0;

        while (std::getline(ss, segment, '.')) {

                octetCount++;
                if (octetCount > 4) {
                        return false;
                }

                if (segment.empty()) {
                        return false;
                }


                if (!std::all_of(segment.begin(), segment.end(), ::isdigit)) {
                        return false;
                }
                if (segment.length() > 1 && segment.front() == '0') {
                        return false;
                }

                try {
                        int octet = std::stoi(segment);
                        if (octet < 0 || octet > 255) {
                                return false;
                        }
                } catch (const std::exception& e) {
                        return false;
                }
        }

        if (octetCount != 4) {
                return false;
        }

        return true;
}


