// Interactive developer build using the same logic and renderer as PS5.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/main.cpp"
#include <SDL2/SDL.h>
#include <vector>
int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
        return 1;
    auto *win = SDL_CreateWindow("Crisis de Estado - Cumbre Total", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
    auto *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    auto *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                      1920, 1080);
    if (!win || !renderer || !texture)
        return 2;
    std::vector<uint32_t> pixels(1920 * 1080);
    auto c = Canvas::preview(pixels.data());
    bool running = true;
    SDL_GameController *pad = nullptr;
    uint32_t previous = 0;
    while (running)
    {
        auto started = SDL_GetTicks();
        crisis::Input input;
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN && !e.key.repeat)
            {
                auto k = e.key.keysym.sym;
                input.confirm = k == SDLK_RETURN;
                input.jump = k == SDLK_k;
                input.special = k == SDLK_l;
                input.pause = k == SDLK_ESCAPE;
                input.back = k == SDLK_BACKSPACE;
                input.monster = k == SDLK_q;
                input.redbull = k == SDLK_e;
                input.dash = k == SDLK_SPACE;
                input.choose = (k == SDLK_RIGHT) - (k == SDLK_LEFT);
            }
            if (e.type == SDL_CONTROLLERDEVICEADDED && !pad)
                pad = SDL_GameControllerOpen(e.cdevice.which);
            if (e.type == SDL_CONTROLLERDEVICEREMOVED && pad &&
                SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad)) == e.cdevice.which)
            {
                SDL_GameControllerClose(pad);
                pad = nullptr;
                previous = 0;
            }
        }
        const auto *k = SDL_GetKeyboardState(nullptr);
        input.x = float(k[SDL_SCANCODE_D] || k[SDL_SCANCODE_RIGHT]) -
                  float(k[SDL_SCANCODE_A] || k[SDL_SCANCODE_LEFT]);
        input.y = float(k[SDL_SCANCODE_S] || k[SDL_SCANCODE_DOWN]) -
                  float(k[SDL_SCANCODE_W] || k[SDL_SCANCODE_UP]);
        input.attack = k[SDL_SCANCODE_J];
        input.block = k[SDL_SCANCODE_I];
        if (!pad)
            for (int i = 0; i < SDL_NumJoysticks(); ++i)
                if (SDL_IsGameController(i))
                {
                    pad = SDL_GameControllerOpen(i);
                    break;
                }
        if (pad)
        {
            uint32_t buttons = 0;
            for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)
                if (SDL_GameControllerGetButton(pad, SDL_GameControllerButton(i)))
                    buttons |= 1u << i;
            if (SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000)
                buttons |= 1u << 31;
            uint32_t edge = buttons & ~previous;
            previous = buttons;
            auto press = [edge](int b) { return bool(edge & (1u << b)); };
            auto held = [buttons](int b) { return bool(buttons & (1u << b)); };
            float x = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.f;
            float y = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY) / 32767.f;
            if (std::abs(x) > .2f)
                input.x = x;
            if (std::abs(y) > .2f)
                input.y = y;
            if (held(SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                input.x = -1;
            if (held(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                input.x = 1;
            if (held(SDL_CONTROLLER_BUTTON_DPAD_UP))
                input.y = -1;
            if (held(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                input.y = 1;
            input.choose += int(press(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) -
                            int(press(SDL_CONTROLLER_BUTTON_DPAD_LEFT));
            input.attack |= held(SDL_CONTROLLER_BUTTON_X);
            input.block |= held(SDL_CONTROLLER_BUTTON_B);
            input.jump |= press(SDL_CONTROLLER_BUTTON_A);
            input.confirm |= press(SDL_CONTROLLER_BUTTON_A);
            input.special |= press(SDL_CONTROLLER_BUTTON_Y);
            input.pause |= press(SDL_CONTROLLER_BUTTON_START);
            input.back |= press(SDL_CONTROLLER_BUTTON_B);
            input.monster |= press(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            input.redbull |= press(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            input.dash |= press(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
            input.dash |= bool(edge & (1u << 31));
        }
        hostInput = input;
        draw(c);
        SDL_UpdateTexture(texture, nullptr, pixels.data(), 1920 * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        auto elapsed = SDL_GetTicks() - started;
        if (elapsed < 33)
            SDL_Delay(33 - elapsed);
    }
    if (pad)
        SDL_GameControllerClose(pad);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
