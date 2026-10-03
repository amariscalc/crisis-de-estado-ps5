// Crisis de Estado v0.2.0. SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
namespace crisis
{
constexpr int Hz = 30;
constexpr int ChapterCount = 5;
enum class Scene
{
    Menu,
    Arena,
    Victory,
    Defeat
};
enum class Drink
{
    None,
    Monster,
    RedBull,
    FirstAid
};
enum class Role
{
    Delegate,
    Sprinter,
    Heavy,
    Boss
};
struct Leader
{
    const char *name, *country, *special;
    int damage;
    unsigned accent;
};
inline constexpr std::array<Leader, 6> leaders{
    {{"PEDRO SANCHEZ", "ESPANA", "MANUAL DE RESISTENCIA", 18, 0xff4646ef},
     {"DONALD TRUMP", "ESTADOS UNIDOS", "TORMENTA ARANCELARIA", 22, 0xff36b2ff},
     {"EMMANUEL MACRON", "FRANCIA", "CUMBRE DIPLOMATICA", 17, 0xffefa94b},
     {"JAVIER MILEI", "ARGENTINA", "MOTOSIERRA FISCAL", 24, 0xffebce72},
     {"XI JINPING", "CHINA", "PLAN QUINQUENAL", 20, 0xff3a41f2},
     {"VLADIMIR PUTIN", "RUSIA", "MESA INTERMINABLE", 21, 0xffe776bd}}};
struct Chapter
{
    const char *title, *subtitle;
    int boss;
    unsigned accent;
};
inline constexpr std::array<Chapter, ChapterCount> chapters{
    {{"LA INVESTIDURA", "MADRID ESPANA", 1, 0xff4edaff},
     {"GUERRA DE ARANCELES", "CUMBRE ESTADOS UNIDOS", 3, 0xff5c72fa},
     {"EL PACTO IMPOSIBLE", "CUMBRE FRANCIA", 4, 0xffe6b250},
     {"ESTADO DE EMERGENCIA", "CUMBRE ARGENTINA", 5, 0xff92d471},
     {"LA ULTIMA CUMBRE", "MADRID CONSEJO MUNDIAL", 0, 0xffeda6dc}}};
struct Input
{
    float x = 0, y = 0;
    bool attack = false, jump = false, special = false, block = false;
    bool pause = false, confirm = false, back = false, dash = false;
    bool monster = false, redbull = false, connected = true;
    int choose = 0;
};
struct Body
{
    float x = 0, y = 0;
    int hp = 0, maxhp = 100, cool = 0, flash = 0, stun = 0;
    int leader = 0, face = 1, monster = 0, redbull = 0, drinkTime = 0;
    int windup = 0, animation = 0;
    float targetX = 0, targetY = 0;
    Drink pending = Drink::None;
    Role role = Role::Delegate;
};
struct Pickup
{
    float x = 0, y = 0;
    Drink type = Drink::None;
    bool active = false;
};
struct Particle
{
    float x = 0, y = 0, vx = 0, vy = 0;
    int life = 0, size = 0;
    unsigned color = 0;
};
struct Projectile
{
    float x = 0, y = 0, vx = 0, vy = 0;
    int life = 0, damage = 0;
    bool friendly = false;
};
inline float damageScale(const Body &b)
{
    return b.monster ? 1.65f : 1.f;
}
inline float speedScale(const Body &b)
{
    return b.redbull ? 1.55f : 1.f;
}
inline void beginDrink(Body &b, Drink type)
{
    b.pending = type;
    b.drinkTime = 24;
    b.windup = 0;
}
inline void timers(Body &b)
{
    for (int *v : {&b.cool, &b.flash, &b.stun, &b.monster, &b.redbull, &b.animation})
        if (*v > 0)
            --*v;
    if (b.drinkTime > 0 && --b.drinkTime == 0)
    {
        if (b.pending == Drink::Monster)
            b.monster = 8 * Hz;
        if (b.pending == Drink::RedBull)
            b.redbull = 6 * Hz;
        b.pending = Drink::None;
    }
}
struct Game
{
    Scene scene = Scene::Menu;
    int selected = 0, chapter = 0, wave = 1, tick = 0, score = 0, energy = 100;
    int monsterCans = 2, redbullCans = 2, jump = 0, hit = 0, invul = 0;
    int dashTime = 0, dashCool = 0, combo = 0, comboTime = 0, bestCombo = 0;
    int shield = 0, specialTime = 0, freeze = 0, impact = 0, banner = 0, transition = 0;
    int rumble = 0, bossPulse = 0, popularity = 0, stability = 100, frame = 0;
    bool paused = false, walking = false, blocked = false;
    std::uint32_t rng = 0xCE0200u;
    Body player{460, 880, 100};
    std::array<Body, 10> enemies{};
    std::array<Pickup, 16> pickups{};
    std::array<Particle, 160> particles{};
    std::array<Projectile, 32> shots{};
    std::uint32_t random()
    {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return rng;
    }
    void burst(float x, float y, unsigned color, int n = 14)
    {
        for (auto &p : particles)
            if (!p.life && n-- > 0)
            {
                p = {x,
                     y,
                     float(int(random() % 100) - 50) * .16f,
                     -float(20 + random() % 60) * .15f,
                     18 + int(random() % 14),
                     3 + int(random() % 7),
                     color};
            }
    }
    void drop(float x, float y, Drink type)
    {
        for (auto &p : pickups)
            if (!p.active)
            {
                p = {x, y, type, true};
                return;
            }
    }
    void shoot(float x, float y, float vx, float vy, bool friendly, int damage)
    {
        for (auto &s : shots)
            if (!s.life)
            {
                s = {x, y, vx, vy, 90, damage, friendly};
                return;
            }
    }
    int bossIndex() const
    {
        for (int i = 0; i < int(enemies.size()); ++i)
            if (enemies[i].hp > 0 && enemies[i].role == Role::Boss)
                return i;
        return -1;
    }
    int alive() const
    {
        int n = 0;
        for (const auto &e : enemies)
            n += e.hp > 0;
        return n;
    }
    void spawn()
    {
        enemies = {};
        pickups = {};
        shots = {};
        const int count = wave == 3 ? 3 : std::min(8, 3 + chapter + wave);
        for (int i = 0; i < count; ++i)
        {
            auto &e = enemies[i];
            e.x = float(1000 + (i % 4) * 210);
            e.y = float(810 + (i % 3) * 45);
            e.leader = (selected + 1 + i + chapter) % 6;
            e.face = -1;
            e.role = (i % 3 == 1) ? Role::Sprinter : (i % 3 == 2 ? Role::Heavy : Role::Delegate);
            e.hp = e.maxhp = 40 + chapter * 12 + (e.role == Role::Heavy ? 25 : 0);
            e.cool = 24 + i * 7;
            if (wave == 3 && i == 0)
            {
                e.role = Role::Boss;
                e.leader = chapters[chapter].boss;
                if (e.leader == selected)
                    e.leader = (e.leader + 1) % 6;
                e.hp = e.maxhp = 260 + chapter * 70;
                e.x = 1480;
                e.y = 870;
            }
        }
        drop(720, 870, Drink::Monster);
        drop(1450, 920, Drink::RedBull);
        drop(1020, 815, Drink::Monster);
        drop(1650, 800, Drink::RedBull);
        banner = 90;
        bossPulse = wave == 3 ? 90 : 0;
    }
    void start()
    {
        const int pick = selected;
        *this = Game{};
        selected = pick;
        scene = Scene::Arena;
        player.leader = selected;
        spawn();
    }
    void damageEnemy(Body &e, int damage, int push)
    {
        if (e.hp <= 0)
            return;
        e.hp -= damage;
        e.flash = 8;
        e.stun = 10;
        e.x = std::clamp(e.x + float(push), 100.f, 1820.f);
        burst(e.x, e.y - 130, 0xff4ee5ff);
        ++combo;
        comboTime = 75;
        bestCombo = std::max(bestCombo, combo);
        impact = 7;
        rumble = 5;
        energy = std::min(100, energy + 4);
        if (e.hp <= 0)
        {
            score += (e.role == Role::Boss ? 1800 : 100) * (1 + combo / 8);
            popularity = std::min(100, popularity + 2);
            burst(e.x, e.y - 100, chapters[chapter].accent, 26);
            drop(e.x, e.y,
                 (random() % 5 == 0) ? Drink::FirstAid
                                     : (random() % 2 ? Drink::Monster : Drink::RedBull));
        }
    }
    void damagePlayer(int damage)
    {
        if (invul || dashTime || jump > 5)
            return;
        damage = shield ? std::max(1, damage / 4) : damage;
        if (blocked)
            damage = std::max(1, damage / 5);
        if (player.monster)
            damage = std::max(1, damage * 3 / 4);
        player.hp -= damage;
        player.flash = 8;
        invul = 16;
        impact = 5;
        rumble = 4;
        stability = std::max(0, stability - 1);
        combo = 0;
        comboTime = 0;
        burst(player.x, player.y - 130, 0xff6d63ff, 10);
    }
    void special()
    {
        if (energy < 40 || hit || player.drinkTime)
            return;
        energy -= 40;
        hit = 20;
        specialTime = 36;
        impact = 10;
        rumble = 10;
        burst(player.x, player.y - 160, leaders[selected].accent, 32);
        if (selected == 0)
            shield = 5 * Hz;
        if (selected == 5)
            shield = 3 * Hz;
        const int ranges[6] = {440, 660, 500, 700, 520, 620};
        const int base[6] = {42, 30, 30, 62, 50, 48};
        for (auto &e : enemies)
            if (e.hp > 0 && std::abs(e.x - player.x) < ranges[selected] &&
                std::abs(e.y - player.y) < 170 &&
                (selected != 3 || (e.x - player.x) * player.face > -80))
                damageEnemy(e, int(base[selected] * damageScale(player)), player.face * 70);
        if (selected == 2)
            for (auto &e : enemies)
                if (e.hp > 0)
                    e.stun = 3 * Hz;
        if (selected == 1)
            for (int i = 0; i < 7; ++i)
                shoot(player.x + player.face * 100, player.y + float((i - 3) * 40),
                      player.face * 18.f, float(i - 3) * .3f, true, int(16 * damageScale(player)));
    }
    void drink(const Input &in)
    {
        if (player.drinkTime || hit || dashTime || jump)
            return;
        if (in.monster && monsterCans > 0)
        {
            --monsterCans;
            beginDrink(player, Drink::Monster);
            energy = std::min(100, energy + 35);
        }
        else if (in.redbull && redbullCans > 0)
        {
            --redbullCans;
            beginDrink(player, Drink::RedBull);
            energy = std::min(100, energy + 20);
        }
    }
    void enemyStep(Body &e, int index)
    {
        if (e.hp <= 0)
            return;
        timers(e);
        if (e.drinkTime || e.stun)
            return;
        float dx = player.x - e.x, dy = player.y - e.y;
        e.face = dx >= 0 ? 1 : -1;
        if (e.windup > 0)
        {
            if (--e.windup == 0)
            {
                if (e.role == Role::Boss)
                {
                    if (std::abs(player.x - e.targetX) < 205 &&
                        std::abs(player.y - e.targetY) < 110)
                        damagePlayer(int(20 * damageScale(e)));
                    burst(e.targetX, e.targetY - 60, 0xff5151ff, 30);
                    impact = 9;
                    if (e.hp < e.maxhp / 2)
                        for (int k = -1; k <= 1; ++k)
                            shoot(e.x, e.y, e.face * 12.f, float(k) * 3.f, false,
                                  int(12 * damageScale(e)));
                }
                else if (std::abs(dx) < 150 && std::abs(dy) < 65)
                    damagePlayer(int((e.role == Role::Heavy ? 13 : 9) * damageScale(e)));
                e.cool = e.redbull ? 23 : 42;
                e.animation = 10;
            }
            return;
        }
        // Delegates and bosses compete with the player for the same physical cans.
        if (std::abs(dx) > 220 || e.role == Role::Boss)
        {
            Pickup *nearest = nullptr;
            float best = 600.f;
            for (auto &p : pickups)
                if (p.active && p.type != Drink::FirstAid &&
                    ((p.type == Drink::Monster && e.monster < 30) ||
                     (p.type == Drink::RedBull && e.redbull < 30)))
                {
                    float dist = std::abs(p.x - e.x) + std::abs(p.y - e.y) * 2;
                    if (dist < best)
                    {
                        best = dist;
                        nearest = &p;
                    }
                }
            if (nearest)
            {
                float bx = nearest->x - e.x, by = nearest->y - e.y;
                if (std::abs(bx) < 45 && std::abs(by) < 35)
                {
                    nearest->active = false;
                    beginDrink(e, nearest->type);
                    return;
                }
                e.x += std::clamp(bx, -4.f, 4.f) * speedScale(e);
                e.y += std::clamp(by, -2.f, 2.f) * speedScale(e);
                return;
            }
        }
        float speed = (e.role == Role::Sprinter ? 5.f : 3.f) * speedScale(e);
        const float desired = e.role == Role::Boss ? 175.f : 98.f;
        if (std::abs(dx) > desired)
            e.x += e.face * speed;
        if (std::abs(dy) > 12)
            e.y += dy > 0 ? 2.2f : -2.2f;
        e.x = std::clamp(e.x, 100.f, 1820.f);
        e.y = std::clamp(e.y, 790.f, 950.f);
        if (!e.cool && std::abs(dx) < (e.role == Role::Boss ? 370 : 145) && std::abs(dy) < 70)
        {
            e.windup = e.role == Role::Boss ? 32 : 14;
            e.targetX = player.x;
            e.targetY = player.y;
        }
        if (e.monster && (tick + index * 3) % 7 == 0)
            burst(e.x, e.y - 100, 0xff56e06e, 1);
    }
    void step(const Input &in)
    {
        ++frame;
        rumble = 0;
        if (!in.connected)
            return;
        if (scene != Scene::Arena)
        {
            if (in.choose)
                selected = (selected + in.choose + 6) % 6;
            if (in.confirm)
                start();
            return;
        }
        if (in.pause)
            paused = !paused;
        if (in.back && paused)
        {
            scene = Scene::Menu;
            paused = false;
            return;
        }
        if (paused)
            return;
        ++tick;
        for (int *v : {&jump, &hit, &invul, &dashTime, &dashCool, &comboTime, &shield, &specialTime,
                       &impact, &banner, &bossPulse})
            if (*v > 0)
                --*v;
        if (!comboTime)
            combo = 0;
        for (auto &p : particles)
            if (p.life)
            {
                --p.life;
                p.x += p.vx;
                p.y += p.vy;
                p.vy += .25f;
            }
        if (freeze)
        {
            --freeze;
            return;
        }
        if (transition)
        {
            if (--transition == 0)
            {
                if (wave < 3)
                    ++wave;
                else if (chapter < ChapterCount - 1)
                {
                    ++chapter;
                    wave = 1;
                }
                else
                {
                    scene = Scene::Victory;
                    return;
                }
                player.hp = std::min(100, player.hp + 25);
                energy = std::min(100, energy + 25);
                spawn();
            }
            return;
        }
        timers(player);
        blocked = in.block && !player.drinkTime;
        walking = (std::abs(in.x) + std::abs(in.y)) > .1f;
        float speed =
            9.f * speedScale(player) * (blocked ? .45f : 1.f) * (player.drinkTime ? .3f : 1.f);
        float length = std::sqrt(in.x * in.x + in.y * in.y);
        float ix = length > 1 ? in.x / length : in.x, iy = length > 1 ? in.y / length : in.y;
        if (ix)
            player.face = ix > 0 ? 1 : -1;
        if (in.dash && !dashCool && !player.drinkTime)
        {
            dashTime = 8;
            dashCool = 26;
        }
        if (dashTime)
        {
            player.x += player.face * 25.f;
            burst(player.x - player.face * 45, player.y - 45, 0xffe9ba78, 2);
        }
        else
        {
            player.x += ix * speed;
            player.y += iy * speed * .6f;
        }
        player.x = std::clamp(player.x, 100.f, 1820.f);
        player.y = std::clamp(player.y, 790.f, 950.f);
        if (in.jump && !jump && !player.drinkTime)
            jump = 26;
        drink(in);
        for (auto &p : pickups)
            if (p.active && std::abs(player.x - p.x) < 48 && std::abs(player.y - p.y) < 38)
            {
                if (p.type == Drink::Monster && monsterCans >= 5)
                    continue;
                if (p.type == Drink::RedBull && redbullCans >= 5)
                    continue;
                if (p.type == Drink::FirstAid)
                    player.hp = std::min(100, player.hp + 25);
                else if (p.type == Drink::Monster)
                    ++monsterCans;
                else
                    ++redbullCans;
                p.active = false;
                burst(p.x, p.y - 30, 0xffd7ffff, 8);
            }
        if (tick % Hz == 0)
            energy = std::min(100, energy + 2);
        if (in.special)
            special();
        if (in.attack && !hit && !player.drinkTime && !blocked)
        {
            hit = player.redbull ? 8 : 13;
            int damage =
                int(leaders[selected].damage * damageScale(player) * (combo % 3 == 2 ? 1.3f : 1.f));
            bool success = false;
            for (auto &e : enemies)
                if (e.hp > 0 && std::abs(player.y - e.y) < 75 && std::abs(player.x - e.x) < 190 &&
                    (e.x - player.x) * player.face >= -25)
                {
                    damageEnemy(e, damage, player.face * (combo % 3 == 2 ? 55 : 28));
                    success = true;
                }
            if (success)
                freeze = 2;
        }
        for (int i = 0; i < int(enemies.size()); ++i)
            enemyStep(enemies[i], i);
        for (auto &s : shots)
            if (s.life)
            {
                --s.life;
                s.x += s.vx;
                s.y += s.vy;
                if (s.x < 0 || s.x > 1920 || s.y < 620 || s.y > 980)
                {
                    s.life = 0;
                    continue;
                }
                if (s.friendly)
                {
                    for (auto &e : enemies)
                        if (e.hp > 0 && std::abs(e.x - s.x) < 65 && std::abs(e.y - s.y) < 45)
                        {
                            damageEnemy(e, s.damage, s.vx > 0 ? 25 : -25);
                            s.life = 0;
                            break;
                        }
                }
                else if (std::abs(player.x - s.x) < 50 && std::abs(player.y - s.y) < 40)
                {
                    damagePlayer(s.damage);
                    s.life = 0;
                }
            }
        if (player.hp <= 0)
        {
            scene = Scene::Defeat;
            return;
        }
        if (!alive())
        {
            transition = 70;
            score += 500;
        }
    }
};
} // namespace crisis
