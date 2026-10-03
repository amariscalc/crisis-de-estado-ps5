// Gameplay regression tests. SPDX-License-Identifier: GPL-3.0-or-later
#include "../../src/game.hpp"
#include <cassert>
#include <iostream>
using namespace crisis;
static void isolate(Game &g)
{
    for (auto &e : g.enemies)
        e.hp = 0;
    g.enemies[0] = {1700, 890, 1000, 999, 0};
    for (auto &p : g.pickups)
        p.active = false;
}
int main()
{
    Game g;
    Input in;
    in.choose = -1;
    g.step(in);
    assert(g.selected == 5);
    in.choose = 1;
    g.step(in);
    assert(g.selected == 0);
    in = {};
    in.confirm = true;
    g.step(in);
    assert(g.scene == 1 && g.player.hp == 100 && g.wave == 1);
    in = {};
    in.pause = true;
    g.step(in);
    assert(g.paused);
    int tick = g.tick;
    float x = g.player.x;
    in = {};
    in.x = 1;
    g.step(in);
    assert(g.tick == tick && g.player.x == x);
    in.pause = true;
    g.step(in);
    assert(!g.paused && g.tick == tick + 1);
    in = {};
    in.connected = false;
    x = g.player.x;
    tick = g.tick;
    g.step(in);
    assert(g.tick == tick && g.player.x == x);
    // Attacks affect the front, not a target behind the player.
    g.start();
    isolate(g);
    g.enemies[0] = {550, 760, 100, 100, 0};
    g.enemies[1] = {350, 760, 100, 100, 0};
    in = {};
    in.attack = true;
    g.step(in);
    assert(g.enemies[0].hp == 82 && g.enemies[1].hp == 100);
    g.step(in);
    assert(g.enemies[0].hp == 82); // cooldown prevents per-frame hits
    // Specials spend energy and affect both directions.
    g.start();
    isolate(g);
    g.enemies[0] = {550, 760, 100, 100, 0};
    g.enemies[1] = {350, 760, 100, 100, 0};
    in = {};
    in.special = true;
    g.step(in);
    assert(g.energy == 60 && g.enemies[0].hp == 60 && g.enemies[1].hp == 60);
    g.start();
    isolate(g);
    g.energy = 39;
    g.enemies[0] = {550, 760, 100, 100, 0};
    g.step(in);
    assert(g.energy == 39 && g.enemies[0].hp == 100);
    g.start();
    isolate(g);
    g.corruption = 50;
    g.enemies[0] = {550, 760, 100, 100, 0};
    g.step(in);
    assert(g.enemies[0].hp == 45);
    // Defenses, jump immunity and the post-hit grace period.
    g.start();
    isolate(g);
    g.enemies[0] = {500, 760, 100, 0, 0};
    in = {};
    in.block = true;
    g.step(in);
    assert(g.player.hp == 99);
    g.enemies[0].cool = 0;
    g.step(in);
    assert(g.player.hp == 99);
    g.start();
    isolate(g);
    g.enemies[0] = {500, 760, 100, 0, 0};
    in = {};
    in.jump = true;
    g.step(in);
    assert(g.jump == 42 && g.player.hp == 100);
    g.start();
    isolate(g);
    g.enemies[0] = {500, 760, 100, 0, 0};
    g.step({});
    assert(g.player.hp == 92);
    // Pickups are consumed once; state is capped and reset between runs.
    g.start();
    isolate(g);
    g.player.hp = 80;
    g.energy = 20;
    g.pickups[0] = {450, 760, false, true};
    g.step({});
    assert(g.player.hp == 95 && g.energy == 40 && !g.pickups[0].active);
    g.step({});
    assert(g.energy == 40);
    g.pickups[1] = {450, 760, true, true};
    g.step({});
    assert(g.energy == 80 && g.corruption == 25);
    g.corruption = 100;
    g.tick = 119;
    g.step({});
    assert(g.player.hp == 93);
    // Lane and arena limits hold across long movement sequences.
    g.start();
    isolate(g);
    in = {};
    in.x = 1;
    in.y = 1;
    for (int i = 0; i < 1000; i++)
        g.step(in);
    assert(g.player.x == 1800 && g.player.y == 890 && g.energy <= 100);
    in.x = -1;
    in.y = -1;
    for (int i = 0; i < 1000; i++)
        g.step(in);
    assert(g.player.x == 100 && g.player.y == 600);
    // Progression and end screens; explicit restart restores the gameplay state.
    g.start();
    for (auto &e : g.enemies)
        e.hp = 0;
    g.step({});
    assert(g.wave == 2);
    for (auto &e : g.enemies)
        e.hp = 0;
    g.step({});
    assert(g.wave == 3);
    for (auto &e : g.enemies)
        e.hp = 0;
    g.step({});
    assert(g.scene == 2);
    in = {};
    in.confirm = true;
    g.step(in);
    assert(g.scene == 1 && g.wave == 1 && g.corruption == 0 && g.face == 1);
    g.player.hp = 1;
    g.enemies[0] = {500, 760, 100, 0, 0};
    g.step({});
    assert(g.scene == 3);
    g.step(in);
    assert(g.player.hp == 100 && g.score == 0);
    std::cout << "PASS: selection, combat, energy, defenses, pickups, pause, disconnect, bounds "
                 "and progression\n";
}
