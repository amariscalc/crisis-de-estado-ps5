// Crisis de Estado v0.1 - GPL-3.0-or-later. Political arcade satire.
#include "demo_renderer.hpp"
#include "game.hpp"
#include <cstddef>
extern "C"
{
#include "pad_abi.h"
}
static_assert(sizeof(PS5_PadData) == 120, "Public pad ABI size mismatch");
using ps5::demo::Canvas;
using ps5::demo::Color;
namespace
{
crisis::Game game;
int handle = -1, retry = 0;
uint32_t old = 0;
Color rgb(unsigned v)
{
    return static_cast<Color>(v);
}
void rect(Canvas &c, int x, int y, int w, int h, Color color)
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
        c.rectangle(x, y, w, h, color);
}
void number(Canvas &c, int x, int y, int n)
{
    char b[12]{};
    int k = 0;
    do
    {
        b[k++] = char('0' + n % 10);
        n /= 10;
    } while (n && k < 11);
    for (int i = 0; i < k / 2; i++)
    {
        char t = b[i];
        b[i] = b[k - 1 - i];
        b[k - 1 - i] = t;
    }
    c.text(x, y, b, 4, Color::white);
}
void person(Canvas &c, int x, int y, int leader, int frame, bool enemy = false,
            bool portrait = false)
{
    const auto &l = crisis::leaders[leader];
    Color skin = rgb(leader == 1 ? 0xff70a1ea : 0xff91b9e6),
          suit = enemy ? rgb(0xff505065) : rgb(0xff50301c);
    int bob = (frame / 10) % 2 * 3;
    if (!portrait)
    {
        c.circle(x, y + 15, 42, rgb(0xff302020));
        rect(c, x - 22, y - 95, 17, 85, suit);
        rect(c, x + 5, y - 95, 17, 85, suit);
    }
    rect(c, x - 35, y - 180 + bob, 70, 90, suit);
    rect(c, x - 8, y - 177 + bob, 16, 50, Color::white);
    rect(c, x - 4, y - 168 + bob, 8, 44, rgb(l.tie));
    c.circle(x, y - 217 + bob, 38, skin);
    if (leader == 5)
    {
        rect(c, x - 34, y - 240 + bob, 12, 22, rgb(l.hair));
        rect(c, x + 22, y - 240 + bob, 12, 22, rgb(l.hair));
    }
    else
    {
        rect(c, x - 34, y - 250 + bob, 68, 20, rgb(l.hair));
    }
    if (leader == 1)
    {
        c.circle(x - 13, y - 252 + bob, 21, rgb(l.hair));
        rect(c, x + 12, y - 207 + bob, 12, 4, rgb(0xff404080));
    }
    if (leader == 0)
    {
        rect(c, x - 4, y - 220 + bob, 8, 21, rgb(0xff7ca3cc));
    }
    if (leader == 3)
    {
        c.circle(x, y - 255 + bob, 20, rgb(l.hair));
        c.circle(x - 22, y - 248 + bob, 17, rgb(l.hair));
        c.circle(x + 22, y - 246 + bob, 17, rgb(l.hair));
    }
    if (leader == 4)
    {
        rect(c, x - 20, y - 225 + bob, 40, 2, rgb(0xff202020));
    }
    if (leader == 3)
    {
        c.circle(x - 28, y - 225 + bob, 12, rgb(l.hair));
        c.circle(x + 28, y - 225 + bob, 12, rgb(l.hair));
    }
    rect(c, x - 17, y - 222 + bob, 7, 5, rgb(0xff202020));
    rect(c, x + 10, y - 222 + bob, 7, 5, rgb(0xff202020));
    rect(c, x - 9, y - 202 + bob, 18, 3, rgb(0xff404080));
    rect(c, x - 51, y - 172 + bob, 15, 64, suit);
    rect(c, x + 36, y - 172 + bob, 15, 64, suit);
}
crisis::Input poll()
{
    crisis::Input in;
    if (handle < 0 && retry++ % 120 == 0)
    {
        sceUserServiceInitialize(nullptr);
        scePadInit();
        int ids[4] = {-1, -1, -1, -1};
        if (sceUserServiceGetLoginUserIdList(ids) >= 0 && ids[0] >= 0)
            handle = scePadOpen(ids[0], 0, 0, nullptr);
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
    return in;
}
void draw(Canvas &c) noexcept
{
    auto in = poll();
    game.step(in);
    c.clear(rgb(0xff21120d));
    c.text(95, 65, "CRISIS DE ESTADO", 10, Color::white);
    c.text(100, 165, "SATIRA POLITICA ARCADE", 4, Color::cyan);
    if (game.scene != 1)
    {
        c.text(100, 270,
               game.scene == 2   ? "CUMBRE SUPERADA"
               : game.scene == 3 ? "CRISIS DE GOBIERNO"
                                 : "ELIGE TU PERSONAJE",
               6, Color::yellow);
        for (int i = 0; i < 6; i++)
        {
            int x = 100 + (i % 3) * 600, y = 370 + (i / 3) * 260;
            rect(c, x, y, 560, 230, i == game.selected ? rgb(0xff705030) : Color::panel);
            person(c, x + 75, y + 280, i, 0, false, true);
            c.text(x + 145, y + 50, crisis::leaders[i].name, 3, Color::white);
            c.text(x + 145, y + 100, crisis::leaders[i].country, 2, Color::cyan);
        }
        c.text(100, 950, "IZQUIERDA DERECHA ELEGIR   X JUGAR", 4, Color::white);
        c.text(100, 1015, "CARICATURAS Y ACCIONES FICTICIAS", 3, Color::cyan);
    }
    else
    {
        rect(c, 0, 520, 1920, 560, rgb(0xff56504b));
        rect(c, 650, 270, 700, 320, rgb(0xffb0b5c0));
        for (int i = 0; i < 6; i++)
            rect(c, 700 + i * 100, 330, 40, 180, Color::white);
        c.text(740, 285, "MADRID ESPANA", 5, Color::panel);
        rect(c, 100, 220, 450, 25, Color::panel);
        rect(c, 100, 220, std::max(0, game.player.hp) * 4, 25, rgb(0xff60cc50));
        rect(c, 100, 260, game.energy * 4, 18, Color::cyan);
        c.text(100, 305, crisis::leaders[game.selected].name, 4, Color::white);
        c.text(100, 350, crisis::leaders[game.selected].special, 3, Color::yellow);
        c.text(100, 400, "CORRUPCION ARCADE", 2, Color::white);
        rect(c, 100, 425, 300, 12, Color::panel);
        rect(c, 100, 425, game.corruption * 3, 12, Color::magenta);
        c.text(1450, 230, "OLEADA", 4, Color::white);
        number(c, 1660, 230, game.wave);
        number(c, 1450, 280, game.score);
        for (const auto &item : game.pickups)
        {
            if (!item.active)
                continue;
            int x = int(item.x), y = int(item.y);
            if (item.funds)
            {
                rect(c, x - 26, y - 36, 52, 36, rgb(0xff4070a0));
                rect(c, x - 12, y - 46, 24, 8, Color::yellow);
                c.text(x - 7, y - 27, "F", 3, Color::yellow);
            }
            else
            {
                rect(c, x - 10, y - 35, 20, 35, Color::cyan);
                rect(c, x - 10, y - 35, 20, 5, Color::white);
                c.text(x - 5, y - 25, "E", 2, Color::panel);
            }
        }
        // Draw back-to-front by lane so feet establish depth.
        std::array<int, 9> order{0, 1, 2, 3, 4, 5, 6, 7, 8};
        std::sort(order.begin(), order.end(),
                  [](int a, int b)
                  {
                      return (a == 8 ? game.player.y : game.enemies[a].y) <
                             (b == 8 ? game.player.y : game.enemies[b].y);
                  });
        for (int i : order)
        {
            if (i == 8)
            {
                int lift = game.jump ? int(100 * std::sin(game.jump * 3.14159 / 42)) : 0;
                if (game.invul % 4 < 2)
                    person(c, game.player.x, game.player.y - lift, game.selected, game.tick);
                if (game.hit)
                {
                    c.circle(game.player.x + game.face * 100, game.player.y - 150 - lift, 32,
                             Color::yellow);
                }
            }
            else
            {
                auto &e = game.enemies[i];
                if (e.hp > 0)
                {
                    person(c, e.x, e.y, (game.selected + 1 + i) % 6, game.tick, true);
                    rect(c, e.x - 35, e.y - 285, e.hp, 6, e.flash ? Color::white : Color::magenta);
                }
            }
        }
        c.text(95, 965, "CUADRADO GOLPE   X SALTO   TRIANGULO ESPECIAL", 3, Color::white);
        c.text(95, 1005, "CIRCULO BLOQUEO   OPTIONS PAUSA", 3, Color::cyan);
        if (game.paused)
        {
            rect(c, 620, 410, 680, 150, Color::panel);
            c.text(770, 460, "PAUSA", 10, Color::white);
        }
    }
    if (!in.connected)
    {
        rect(c, 420, 860, 1100, 65, Color::panel);
        c.text(460, 880, "CONECTA EL DUALSENSE", 5, Color::yellow);
    }
}
} // namespace
int main()
{
    ps5::demo::run(draw, "Crisis de Estado v0.1 ready");
}
