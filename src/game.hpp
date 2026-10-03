// Crisis de Estado gameplay. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace crisis
{
struct Leader
{
    const char *name;
    const char *country;
    const char *special;
    unsigned hair, tie;
};
inline constexpr std::array<Leader, 6> leaders{
    {{"PEDRO SANCHEZ", "ESPANA", "MANUAL DE RESISTENCIA", 0xff252528, 0xff3020cc},
     {"DONALD TRUMP", "ESTADOS UNIDOS", "TORMENTA ARANCELARIA", 0xff55bbdd, 0xff3020dd},
     {"EMMANUEL MACRON", "FRANCIA", "CUMBRE DIPLOMATICA", 0xff303045, 0xffbb5020},
     {"JAVIER MILEI", "ARGENTINA", "MOTOSIERRA FISCAL", 0xff202028, 0xffaa4420},
     {"XI JINPING", "CHINA", "PLAN QUINQUENAL", 0xff161616, 0xff3020cc},
     {"VLADIMIR PUTIN", "RUSIA", "MESA INTERMINABLE", 0xff80a0b0, 0xff3020aa}}};
struct Body
{
    float x = 0, y = 0;
    int hp = 0, cool = 0, flash = 0;
};
struct Input
{
    float x = 0, y = 0;
    bool attack = false, jump = false, special = false, block = false, pause = false,
         confirm = false;
    int choose = 0;
    bool connected = true;
};
struct Pickup
{
    float x, y;
    bool funds, active;
};
struct Game
{
    int scene = 0, selected = 0, wave = 1, score = 0, energy = 100, tick = 0, hit = 0, jump = 0,
        invul = 0;
    bool paused = false;
    int face = 1;
    int corruption = 0;
    std::array<Pickup, 3> pickups{};
    Body player{450, 760, 100};
    std::array<Body, 8> enemies{};
    void spawn()
    {
        pickups = {{{750, 830, false, true}, {1500, 820, true, true}, {1200, 620, false, true}}};
        for (int i = 0; i < 8; i++)
            enemies[i] = {float(1100 + (i % 3) * 170), float(660 + (i % 3) * 90),
                          i < wave + 2 ? 35 + wave * 5 : 0, 30 + i * 9, 0};
    }
    void start()
    {
        scene = 1;
        wave = 1;
        score = 0;
        corruption = 0;
        face = 1;
        energy = 100;
        tick = 0;
        player = {450, 760, 100};
        paused = false;
        jump = hit = invul = 0;
        spawn();
    }
    void step(const Input &in)
    {
        if (scene != 1)
        {
            if (in.choose)
                selected = (selected + in.choose + 6) % 6;
            if (in.confirm)
            {
                start();
            }
            return;
        }
        if (in.pause)
            paused = !paused;
        if (paused || !in.connected)
            return;
        ++tick;
        if (invul > 0)
            --invul;
        if (hit > 0)
            --hit;
        if (jump > 0)
            --jump;
        player.x = std::clamp(player.x + in.x * 7.f, 100.f, 1800.f);
        player.y = std::clamp(player.y + in.y * 4.f, 600.f, 890.f);
        if (in.x)
            face = in.x > 0 ? 1 : -1;
        if (in.jump && !jump)
            jump = 42;
        if (tick % 60 == 0)
            energy = std::min(100, energy + 3);
        for (auto &item : pickups)
        {
            if (item.active && std::abs(player.x - item.x) < 45 && std::abs(player.y - item.y) < 40)
            {
                item.active = false;
                energy = std::min(100, energy + (item.funds ? 40 : 20));
                if (item.funds)
                    corruption = std::min(100, corruption + 25);
                else
                    player.hp = std::min(100, player.hp + 15);
            }
        }
        if (corruption == 100 && tick % 120 == 0)
            player.hp -= 2;
        bool strike = in.attack && hit == 0;
        bool super = in.special && energy >= 40 && hit == 0;
        if (strike || super)
        {
            hit = 18;
            if (super)
                energy -= 40;
        }
        int alive = 0;
        for (auto &e : enemies)
        {
            if (e.hp <= 0)
                continue;
            ++alive;
            if (e.cool > 0)
                --e.cool;
            if (e.flash > 0)
                --e.flash;
            float dx = player.x - e.x, dy = player.y - e.y;
            if ((strike || super) && std::abs(dx) < (super ? 430 : 170) &&
                std::abs(dy) < (super ? 210 : 80) && (super || dx * face <= 0))
            {
                e.hp -= super ? (corruption >= 50 ? 55 : 40) : 18;
                e.flash = 12;
                e.cool = 30;
                e.x = std::clamp(e.x + face * 35.f, 100.f, 1800.f);
                if (e.hp <= 0)
                {
                    score += 100;
                    energy = std::min(100, energy + 8);
                }
            }
            if (e.hp <= 0)
                continue;
            if (std::abs(dx) > 90)
                e.x += dx > 0 ? 2.6f : -2.6f;
            if (std::abs(dy) > 15)
                e.y += dy > 0 ? 1.5f : -1.5f;
            if (std::abs(dx) < 110 && std::abs(dy) < 65 && e.cool == 0)
            {
                e.cool = 50;
                if (!invul && jump < 12)
                {
                    player.hp -= in.block ? 1 : 8;
                    invul = 24;
                }
            }
        }
        if (player.hp <= 0)
        {
            scene = 3;
            return;
        }
        if (!alive)
        {
            if (wave == 3)
            {
                scene = 2;
                return;
            }
            ++wave;
            player.hp = std::min(100, player.hp + 20);
            energy = std::min(100, energy + 25);
            spawn();
        }
    }
};
} // namespace crisis
