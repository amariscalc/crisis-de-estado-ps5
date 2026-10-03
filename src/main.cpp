// Crisis de Estado v0.2.0 - native political arcade satire.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "demo_renderer.hpp"
#include "game.hpp"
#include "art.hpp"
#include "font_metrics.hpp"
#include <cstddef>
extern "C"
{
#include "pad_abi.h"
}
static_assert(sizeof(PS5_PadData) == 120, "Public pad ABI mismatch");
using ps5::demo::Canvas;
using ps5::demo::Color;
namespace
{
crisis::Game game;
crisis::Art leadersArt, congressArt, fontArt;
bool loaded = false, artReady = false;
int handle = -1, retry = 0;
uint32_t old = 0;
#ifdef CRISIS_HOST_PREVIEW
crisis::Input hostInput;
#endif
Color rgb(unsigned c)
{
    return static_cast<Color>(c);
}
constexpr unsigned ink = 0xff1d1717, panelColor = 0xff302823, cream = 0xffe3f0ff;
void box(Canvas &c, int x, int y, int w, int h, unsigned color)
{
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (w > 0 && h > 0)
        c.rectangle(unsigned(x), unsigned(y), unsigned(w), unsigned(h), rgb(color));
}
void panel(Canvas &c, int x, int y, int w, int h, unsigned fill = panelColor)
{
    box(c, x + 6, y + 8, w, h, ink);
    box(c, x, y, w, h, ink);
    box(c, x + 5, y + 5, w - 10, h - 10, fill);
}
void text(Canvas &c, int x, int y, const char *s, int scale = 3, unsigned color = cream)
{
    if (!fontArt.pixels)
    {
        if (x >= 0 && y >= 0)
            c.text(unsigned(x), unsigned(y), s, unsigned(scale), rgb(color));
        return;
    }
    int size = 7 * scale, cell = 64 * size / 48;
    for (; *s; ++s)
    {
        unsigned ch = static_cast<unsigned char>(*s);
        if (ch < 32 || ch > 127)
            ch = 32;
        unsigned index = ch - 32;
        c.blit(fontArt.pixels.get(), 1024, 384, (index % 16) * 64, (index / 16) * 64, 64, 64, x, y,
               cell, cell, false, 0, color);
        x += (crisis::fontAdvance[index] * size + 24) / 48;
    }
}
void num(Canvas &c, int x, int y, int value, int scale = 4, unsigned color = cream)
{
    char b[16]{};
    int n = 0;
    value = std::max(0, value);
    do
    {
        b[n++] = char('0' + value % 10);
        value /= 10;
    } while (value && n < 15);
    for (int i = 0; i < n / 2; ++i)
        std::swap(b[i], b[n - 1 - i]);
    text(c, x, y, b, scale, color);
}
void bar(Canvas &c, int x, int y, int w, int h, int amount, unsigned col)
{
    box(c, x, y, w, h, ink);
    box(c, x + 4, y + 4, (w - 8) * std::clamp(amount, 0, 100) / 100, h - 8, col);
    box(c, x + 6, y + 5, (w - 12) * std::clamp(amount, 0, 100) / 100, 3, 0xffe6eeee);
}
void ellipse(Canvas &c, int x, int y, int rx, int ry, unsigned col)
{
    for (int row = -ry; row <= ry; ++row)
    {
        int half = int(rx * std::sqrt(std::max(0.f, 1.f - float(row * row) / float(ry * ry))));
        box(c, x - half, y + row, half * 2, 1, col);
    }
}
void ring(Canvas &c, int x, int y, int rx, int ry, unsigned col, int t = 6)
{
    for (int i = 0; i < 72; ++i)
    {
        float a = float(i) * 6.2831853f / 72;
        int px = x + int(rx * std::cos(a)), py = y + int(ry * std::sin(a));
        if (px >= 0 && py >= 0)
            c.circle(unsigned(px), unsigned(py), unsigned(t), rgb(col));
    }
}
void can(Canvas &c, int x, int y, crisis::Drink kind, int scale = 1)
{
    bool monster = kind == crisis::Drink::Monster;
    panel(c, x - 13 * scale, y - 44 * scale, 26 * scale, 44 * scale,
          monster ? 0xff1d1b17 : 0xffe4d9c9);
    box(c, x - 11 * scale, y - 42 * scale, 22 * scale, 5 * scale, 0xffc4c8cf);
    if (monster)
    {
        for (int i = -1; i <= 1; ++i)
        {
            c.line(x + i * 6 * scale, y - 32 * scale, x + i * 6 * scale - 3 * scale, y - 14 * scale,
                   2 * scale, rgb(0xff46e366));
            c.line(x + i * 6 * scale - 3 * scale, y - 14 * scale, x + i * 6 * scale - 2 * scale,
                   y - 8 * scale, 2 * scale, rgb(0xff46e366));
        }
    }
    else
    {
        box(c, x, y - 35 * scale, 10 * scale, 27 * scale, 0xffac5926);
        c.circle(x - 4 * scale, y - 19 * scale, 5 * scale, rgb(0xff3839e7));
        c.circle(x + 4 * scale, y - 19 * scale, 5 * scale, rgb(0xff3839e7));
    }
}
void button(Canvas &c, int x, int y, char kind, unsigned color)
{
    c.circle(x, y, 16, rgb(ink));
    c.circle(x, y, 12, rgb(color));
    if (kind == 'X')
    {
        c.line(x - 6, y - 6, x + 6, y + 6, 3, rgb(ink));
        c.line(x - 6, y + 6, x + 6, y - 6, 3, rgb(ink));
    }
    else if (kind == 'S')
    {
        box(c, x - 6, y - 6, 13, 13, ink);
        box(c, x - 3, y - 3, 7, 7, color);
    }
    else if (kind == 'T')
    {
        c.triangle(x, y - 8, 9, 17, rgb(ink));
    }
    else
    {
        c.circle(x, y, 8, rgb(ink));
        c.circle(x, y, 5, rgb(color));
    }
}
void background(Canvas &c)
{
    if (congressArt.pixels)
        c.blit(congressArt.pixels.get(), congressArt.width, congressArt.height, 0, 0,
               congressArt.width, congressArt.height, 0, 0, 1920, 1080);
    else
    {
        c.clear(rgb(0xffdab591));
        box(c, 0, 630, 1920, 450, 0xff9bbbdf);
    }
}
void sprite(Canvas &c, const crisis::Body &b, int height, bool hero = false, bool portrait = false)
{
    int lift = hero && game.jump ? int(130 * std::sin(game.jump * 3.14159265 / 26)) : 0;
    int x = int(b.x), y = int(b.y) - lift;
    if (!portrait)
    {
        ellipse(c, x, int(b.y) + 3, height / 5, 12, 0xff867258);
        if (b.monster || b.redbull)
        {
            unsigned color =
                b.monster && b.redbull ? 0xffe988ce : (b.monster ? 0xff46e366 : 0xffefa648);
            ring(c, x, int(b.y), height / 3, 22, color, 3);
            for (int i = 0; i < 4; ++i)
            {
                float a = game.tick * .12f + i * 1.5708f;
                int px = x + int(std::cos(a) * height / 3),
                    py = y - height / 2 + int(std::sin(a) * 80);
                if (px >= 0 && py >= 0)
                    c.circle(px, py, 4, rgb(color));
            }
        }
        if (b.windup)
            ring(c, int(b.targetX), int(b.targetY), b.role == crisis::Role::Boss ? 200 : 110, 42,
                 0xff5453f4, 4);
    }
    int bob = portrait ? 0 : int(std::sin(game.tick * .2f) * 3);
    int stretch = hero && game.dashTime ? 18 : 0;
    int tilt = hero && game.hit && !game.specialTime ? game.player.face * 20 : 0;
    int w = height + stretch;
    int sx = (b.leader % 3) * 512, sy = (b.leader / 3) * 512;
    if (leadersArt.pixels)
        c.blit(leadersArt.pixels.get(), leadersArt.width, leadersArt.height, sx, sy, 512, 512,
               x - w / 2 + tilt, y - height + bob, w, height, b.face < 0, b.flash % 4 > 1 ? 1 : 0);
    else
    {
        box(c, x - 30, y - 125, 60, 110, 0xff3e2920);
        c.circle(x, y - 160, 42, rgb(0xff86b8e6));
    }
    if (!portrait && b.drinkTime)
    {
        can(c, x + b.face * 28, y - height * 2 / 3, b.pending, 1);
        text(c, x - 70, y - height - 40,
             b.pending == crisis::Drink::Monster ? "MONSTER" : "RED BULL", 2,
             b.pending == crisis::Drink::Monster ? 0xff46e366 : 0xffefa648);
        bar(c, x - 50, y - height - 15, 100, 9, (24 - b.drinkTime) * 100 / 24, 0xffecdf91);
    }
    if (hero && game.blocked)
        ring(c, x + game.player.face * 35, y - height / 2, 75, 110, 0xffefbf79, 5);
    if (hero && game.shield)
        ring(c, x, y - height / 2, height / 2, height / 2, 0xffefcb83, 3);
    if (!hero && !portrait)
        bar(c, x - 45, y - height - 12, 90, 10, std::max(0, b.hp) * 100 / b.maxhp,
            b.role == crisis::Role::Boss ? 0xff3d4ef2 : 0xff5487f6);
}
crisis::Input poll()
{
#ifdef CRISIS_HOST_PREVIEW
    return hostInput;
#else
    crisis::Input in;
    if (handle < 0 && retry++ % 60 == 0)
    {
        sceUserServiceInitialize(nullptr);
        scePadInit();
        int ids[4] = {-1, -1, -1, -1};
        if (sceUserServiceGetLoginUserIdList(ids) >= 0)
            for (int id : ids)
                if (id >= 0)
                {
                    handle = scePadOpen(id, 0, 0, nullptr);
                    if (handle >= 0)
                        break;
                }
        if (handle >= 0)
            scePadSetVibrationMode(handle, 1);
    }
    PS5_PadData p{};
    if (handle < 0 || scePadReadState(handle, &p) < 0 || !p.connected)
    {
        in.connected = false;
        old = 0;
        if (handle >= 0)
        {
            scePadClose(handle);
            handle = -1;
        }
        return in;
    }
    uint32_t edge = p.buttons & ~old;
    old = p.buttons;
    float x = (int(p.leftStick.x) - 128) / 127.f, y = (int(p.leftStick.y) - 128) / 127.f;
    in.x = std::abs(x) > .2f ? x : 0;
    in.y = std::abs(y) > .2f ? y : 0;
    if (p.buttons & PS5_PAD_BUTTON_LEFT)
        in.x = -1;
    if (p.buttons & PS5_PAD_BUTTON_RIGHT)
        in.x = 1;
    if (p.buttons & PS5_PAD_BUTTON_UP)
        in.y = -1;
    if (p.buttons & PS5_PAD_BUTTON_DOWN)
        in.y = 1;
    in.choose = (edge & PS5_PAD_BUTTON_RIGHT ? 1 : 0) - (edge & PS5_PAD_BUTTON_LEFT ? 1 : 0);
    in.attack = p.buttons & PS5_PAD_BUTTON_SQUARE;
    in.jump = edge & PS5_PAD_BUTTON_CROSS;
    in.special = edge & PS5_PAD_BUTTON_TRIANGLE;
    in.block = p.buttons & PS5_PAD_BUTTON_CIRCLE;
    in.pause = edge & PS5_PAD_BUTTON_OPTIONS;
    in.confirm = edge & PS5_PAD_BUTTON_CROSS;
    in.back = edge & PS5_PAD_BUTTON_CIRCLE;
    in.dash = edge & PS5_PAD_BUTTON_R2;
    in.monster = edge & PS5_PAD_BUTTON_L1;
    in.redbull = edge & PS5_PAD_BUTTON_R1;
    return in;
#endif
}
void feedback(bool connected)
{
#ifndef CRISIS_HOST_PREVIEW
    if (handle < 0)
        return;
    PS5_PadVibration vibe{static_cast<uint8_t>(connected && !game.paused && game.rumble ? 110 : 0),
                          static_cast<uint8_t>(connected && !game.paused && game.rumble ? 65 : 0)};
    scePadSetVibration(handle, &vibe);
    PS5_PadColor color{80, 130, 235, 0};
    if (game.player.monster)
        color = {70, 225, 95, 0};
    if (game.player.redbull)
        color = {35, 130, 255, 0};
    if (game.player.monster && game.player.redbull)
        color = {200, 80, 255, 0};
    scePadSetLightBar(handle, &color);
#else
    (void)connected;
#endif
}
void loadArt()
{
    if (loaded)
        return;
    loaded = true;
#ifdef CRISIS_HOST_PREVIEW
    bool a = leadersArt.load("assets/leaders.cdea"), b = congressArt.load("assets/congress.cdea");
    fontArt.load("assets/font.cdea");
#else
    bool a = leadersArt.load("/app0/assets/leaders.cdea"),
         b = congressArt.load("/app0/assets/congress.cdea");
    fontArt.load("/app0/assets/font.cdea");
#endif
    artReady = a && b;
}
void menu(Canvas &c)
{
    c.shade(140);
    text(c, 85, 55, "CRISIS DE ESTADO", 10, 0xff62d9ff);
    text(c, 90, 145, "CUMBRE TOTAL", 5);
    text(c, 1350, 65, "VERSION 0 2", 3);
    text(c, 90, 205,
         game.scene == crisis::Scene::Victory  ? "HAS SALVADO LA CUMBRE"
         : game.scene == crisis::Scene::Defeat ? "CRISIS DE GOBIERNO"
                                               : "ELIGE TU LIDER",
         4, 0xff62d9ff);
    if (game.scene != crisis::Scene::Menu)
    {
        text(c, 1250, 205, "PUNTOS", 3);
        num(c, 1430, 205, game.score, 4);
    }
    for (int i = 0; i < 6; ++i)
    {
        int x = 85 + (i % 3) * 590, y = 280 + (i / 3) * 285;
        bool on = i == game.selected;
        panel(c, x, y, 555, 255, on ? 0xff685d39 : panelColor);
        if (on)
            box(c, x + 5, y + 5, 545, 5, crisis::leaders[i].accent);
        crisis::Body b{};
        b.x = float(x + 115);
        b.y = float(y + 248);
        b.leader = i;
        sprite(c, b, 235, false, true);
        text(c, x + 242, y + 35, crisis::leaders[i].name, 3);
        text(c, x + 242, y + 80, crisis::leaders[i].country, 2, 0xffefbf79);
        text(c, x + 242, y + 130, "GOLPE", 2);
        num(c, x + 380, y + 130, crisis::leaders[i].damage, 2);
        text(c, x + 242, y + 165, on ? "LISTO PARA LA CUMBRE" : "SELECCIONABLE", 2,
             on ? 0xff62d9ff : 0xff93867a);
    }
    panel(c, 85, 885, 1745, 140);
    button(c, 125, 925, 'X', 0xffefa248);
    text(c, 155, 913, "EMPEZAR", 4);
    text(c, 550, 913, "IZQUIERDA DERECHA ELEGIR", 3);
    can(c, 1400, 943, crisis::Drink::Monster, 1);
    text(c, 1425, 918, "MONSTER ENERGY", 2, 0xff46e366);
    can(c, 1660, 943, crisis::Drink::RedBull, 1);
    text(c, 1685, 918, "RED BULL", 2, 0xffefbf79);
    text(c, 120, 980, crisis::leaders[game.selected].special, 3, 0xff62d9ff);
}
void effects(Canvas &c)
{
    if (game.specialTime)
    {
        int x = int(game.player.x), y = int(game.player.y) - 120;
        unsigned col = crisis::leaders[game.selected].accent;
        if (game.selected == 3)
        {
            for (int i = 0; i < 8; ++i)
                c.line(x, y - 30 + i * 8, x + game.player.face * 660, y - 90 + i * 22, 5, rgb(col));
        }
        else if (game.selected == 5)
        {
            panel(c, x - 285, y + 80, 570, 80, 0xff5893ba);
            box(c, x - 250, y + 160, 35, 85, ink);
            box(c, x + 215, y + 160, 35, 85, ink);
        }
        else
        {
            int r = (36 - game.specialTime) * 16;
            ring(c, x, y, r, std::max(10, r / 3), col, 7);
        }
        panel(c, 430, 350, 1060, 80);
        text(c, 460, 374, crisis::leaders[game.selected].special, 4, col);
    }
    for (const auto &p : game.particles)
        if (p.life)
        {
            box(c, int(p.x), int(p.y), p.size, p.size, ink);
            box(c, int(p.x) + 1, int(p.y) + 1, p.size - 2, p.size - 2, p.color);
        }
    if (game.hit && !game.specialTime)
    {
        int x = int(game.player.x) + game.player.face * 110, y = int(game.player.y) - 140;
        for (int i = 0; i < 5; ++i)
            c.line(x, y, x + game.player.face * (45 + i * 16), y - 70 + i * 33, 5, rgb(0xff64e4ff));
        text(c, x - 20, y - 50, game.combo % 3 == 2 ? "POW" : "BAM", 4, 0xff56d9ff);
    }
    for (const auto &s : game.shots)
        if (s.life)
        {
            panel(c, int(s.x) - 20, int(s.y) - 105, 40, 28, 0xffe4eff9);
            box(c, int(s.x) - 12, int(s.y) - 99, 20, 2, ink);
            box(c, int(s.x) - 12, int(s.y) - 93, 14, 2, ink);
        }
}
void hud(Canvas &c)
{
    panel(c, 28, 25, 465, 200);
    text(c, 53, 43, crisis::leaders[game.selected].name, 3);
    text(c, 55, 83, "VIDA", 2);
    bar(c, 165, 77, 300, 22, game.player.hp, 0xff62da65);
    text(c, 55, 119, "ESPECIAL", 2);
    bar(c, 165, 113, 300, 20, game.energy, 0xffefa648);
    text(c, 55, 155, "POPULARIDAD", 2);
    bar(c, 225, 151, 240, 16, game.popularity, 0xff62d9ff);
    text(c, 55, 187, "ESTABILIDAD", 2);
    bar(c, 225, 181, 240, 16, game.stability, 0xffeeac76);
    panel(c, 540, 25, 845, 120);
    text(c, 572, 48, "CRISIS DE ESTADO", 7, 0xff62d9ff);
    text(c, 580, 113, "CUMBRE TOTAL", 2);
    panel(c, 1440, 25, 450, 170);
    text(c, 1460, 49, "CAPITULO", 3);
    num(c, 1730, 49, game.chapter + 1, 4, 0xff62d9ff);
    text(c, 1460, 97, game.wave == 3 ? "JEFE DE CUMBRE" : "OLEADA", 3);
    num(c, 1780, 97, game.wave, 3);
    text(c, 1460, 143, "PUNTOS", 2);
    num(c, 1610, 139, game.score, 3);
    int bi = game.bossIndex();
    if (bi >= 0)
    {
        const auto &b = game.enemies[bi];
        panel(c, 540, 157, 845, 73);
        text(c, 560, 170, crisis::leaders[b.leader].name, 2, 0xff7892ff);
        bar(c, 560, 198, 805, 16, b.hp * 100 / b.maxhp, 0xff5150f4);
    }
    if (game.combo > 1)
    {
        panel(c, 1480, 230, 340, 95);
        text(c, 1500, 250, "COMBO", 4, 0xff62d9ff);
        num(c, 1690, 250, game.combo, 6, 0xff62d9ff);
    }
    if (game.player.monster || game.player.redbull)
    {
        panel(c, 32, 245, 460, 100);
        if (game.player.monster)
        {
            text(c, 50, 258, "MONSTER FUERZA", 2, 0xff46e366);
            bar(c, 55, 280, 420, 11, game.player.monster * 100 / 240, 0xff46e366);
        }
        if (game.player.redbull)
        {
            text(c, 50, 300, "RED BULL VELOCIDAD", 2, 0xffefa648);
            bar(c, 55, 322, 420, 11, game.player.redbull * 100 / 180, 0xffefa648);
        }
    }
    panel(c, 25, 980, 1870, 78);
    button(c, 65, 1016, 'S', 0xffdc71eb);
    text(c, 95, 1007, "GOLPE", 3);
    button(c, 260, 1016, 'X', 0xffefa648);
    text(c, 290, 1007, "SALTO", 3);
    button(c, 455, 1016, 'T', 0xff72e88f);
    text(c, 485, 1007, "ESPECIAL", 3);
    button(c, 720, 1016, 'O', 0xff6465fb);
    text(c, 750, 1007, "BLOQUEO", 3);
    text(c, 1005, 1007, "R2 DASH", 3);
    can(c, 1320, 1035, crisis::Drink::Monster);
    text(c, 1345, 1003, "L1", 3, 0xff46e366);
    num(c, 1420, 1003, game.monsterCans, 3);
    can(c, 1530, 1035, crisis::Drink::RedBull);
    text(c, 1555, 1003, "R1", 3, 0xffefa648);
    num(c, 1630, 1003, game.redbullCans, 3);
    text(c, 1705, 1009, "OPTIONS", 2);
}
void arena(Canvas &c)
{
    for (const auto &p : game.pickups)
        if (p.active)
        {
            int x = int(p.x), y = int(p.y) + int(std::sin(game.tick * .14f + p.x) * 5);
            ellipse(c, x, int(p.y) + 2, 25, 7, 0xff8c7d68);
            if (p.type == crisis::Drink::FirstAid)
            {
                panel(c, x - 18, y - 38, 36, 36, 0xffe4e8ef);
                box(c, x - 4, y - 30, 8, 21, 0xff363bfb);
                box(c, x - 10, y - 24, 20, 8, 0xff363bfb);
            }
            else
                can(c, x, y, p.type, 1);
        }
    std::array<int, 11> order{};
    for (int i = 0; i < 11; ++i)
        order[i] = i;
    std::sort(order.begin(), order.end(),
              [](int a, int b)
              {
                  return (a == 10 ? game.player.y : game.enemies[a].y) <
                         (b == 10 ? game.player.y : game.enemies[b].y);
              });
    for (int i : order)
    {
        if (i == 10)
            sprite(c, game.player, 295, true);
        else if (game.enemies[i].hp > 0)
            sprite(c, game.enemies[i], game.enemies[i].role == crisis::Role::Boss ? 365 : 265);
    }
    effects(c);
    hud(c);
    if (game.banner > 0 || game.transition > 0)
    {
        panel(c, 510, 400, 900, 125);
        text(c, 545, 425,
             game.transition ? "OLEADA SUPERADA" : crisis::chapters[game.chapter].title, 4,
             0xff62d9ff);
        text(c, 545, 480, crisis::chapters[game.chapter].subtitle, 3);
    }
    if (game.paused)
    {
        c.shade(150);
        panel(c, 540, 370, 840, 300);
        text(c, 740, 425, "PAUSA", 9, 0xff62d9ff);
        text(c, 600, 550, "OPTIONS CONTINUAR", 4);
        text(c, 600, 615, "CIRCULO MENU", 4);
    }
}
void draw(Canvas &c) noexcept
{
    loadArt();
    crisis::Input in = poll();
    game.step(in);
    feedback(in.connected);
    background(c);
    if (game.scene == crisis::Scene::Arena)
        arena(c);
    else
        menu(c);
    if (!artReady)
    {
        panel(c, 420, 840, 1080, 70);
        text(c, 450, 865, "NO SE HAN CARGADO LOS GRAFICOS", 3, 0xff6161ff);
    }
    if (!in.connected)
    {
        c.shade(140);
        panel(c, 420, 420, 1080, 180);
        text(c, 495, 480, "CONECTA EL DUALSENSE", 5, 0xff62d9ff);
    }
}
} // namespace
#ifndef CRISIS_HOST_PREVIEW
int main()
{
    ps5::demo::run(draw, "Crisis de Estado v0.2 - Cumbre Total");
}
#endif
