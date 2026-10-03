// Behaviour tests for the shared PS5/desktop gameplay engine.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../src/game.hpp"
#include <cassert>
#include <iostream>
using namespace crisis;
void clearEnemies(Game &g)
{
    for (auto &e : g.enemies)
        e.hp = 0;
}
void keepWave(Game &g)
{
    clearEnemies(g);
    g.enemies[9] = {1800, 940, 100000};
}
int main()
{
    {
        Game g;
        Input i;
        i.choose = -1;
        g.step(i);
        assert(g.selected == 5);
        i = {};
        i.confirm = true;
        g.step(i);
        assert(g.scene == Scene::Arena && g.player.leader == 5 && g.alive() == 4);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        Input i;
        i.monster = true;
        g.step(i);
        assert(g.monsterCans == 1 && g.player.drinkTime == 24 && !g.player.monster);
        int inventory = g.monsterCans;
        g.step(i);
        assert(g.monsterCans == inventory);
        for (int k = 0; k < 23; ++k)
            g.step({});
        assert(g.player.monster == 240);
        assert(damageScale(g.player) == 1.65f);
        for (int k = 0; k < 240; ++k)
            g.step({});
        assert(!g.player.monster);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        Input i;
        i.redbull = true;
        g.step(i);
        for (int k = 0; k < 24; ++k)
            g.step({});
        assert(g.player.redbull == 180);
        float x = g.player.x;
        i = {};
        i.x = 1;
        g.step(i);
        assert(g.player.x - x > 13.f);
        g.player.monster = 100;
        assert(damageScale(g.player) > 1 && speedScale(g.player) > 1);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.pickups = {};
        g.drop(g.player.x, g.player.y, Drink::Monster);
        g.step({});
        assert(g.monsterCans == 3 && !g.pickups[0].active && !g.player.monster);
    }
    {
        Game g;
        g.start();
        g.monsterCans = 0;
        g.redbullCans = 0;
        Input i;
        i.monster = true;
        i.redbull = true;
        g.step(i);
        assert(!g.player.drinkTime && g.monsterCans == 0 && g.redbullCans == 0);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.pickups = {};
        g.enemies[0] = {1000, 800, 50};
        g.drop(1000, 800, Drink::Monster);
        g.enemyStep(g.enemies[0], 0);
        assert(g.enemies[0].drinkTime == 24 && !g.pickups[0].active);
        for (int k = 0; k < 24; ++k)
            g.enemyStep(g.enemies[0], 0);
        assert(g.enemies[0].monster == 240 && damageScale(g.enemies[0]) > 1);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.pickups = {};
        g.enemies[0] = {1000, 800, 50};
        g.drop(1000, 800, Drink::RedBull);
        g.enemyStep(g.enemies[0], 0);
        for (int k = 0; k < 24; ++k)
            g.enemyStep(g.enemies[0], 0);
        assert(g.enemies[0].redbull == 180 && speedScale(g.enemies[0]) > 1);
    }
    {
        Game g;
        g.start();
        g.player.monster = 240;
        g.player.redbull = 180;
        Input i;
        i.pause = true;
        g.step(i);
        const int t = g.tick;
        g.step({});
        assert(g.tick == t && g.player.monster == 240);
        i = {};
        i.connected = false;
        g.paused = false;
        g.step(i);
        assert(g.tick == t && g.player.redbull == 180);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.enemies[0] = {g.player.x + 100, g.player.y, 100};
        g.enemies[1] = {g.player.x - 100, g.player.y, 100};
        Input i;
        i.attack = true;
        g.step(i);
        assert(g.enemies[0].hp == 82 && g.enemies[1].hp == 100 && g.combo == 1);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.player.monster = 120;
        g.enemies[0] = {g.player.x + 100, g.player.y, 100};
        Input i;
        i.attack = true;
        g.step(i);
        assert(g.enemies[0].hp == 71);
    }
    {
        Game g;
        g.selected = 0;
        g.start();
        g.special();
        assert(g.energy == 60 && g.shield == 150);
    }
    {
        Game g;
        g.selected = 1;
        g.start();
        g.special();
        int n = 0;
        for (auto &s : g.shots)
            n += s.life > 0;
        assert(n == 7);
    }
    {
        Game g;
        g.selected = 2;
        g.start();
        g.special();
        for (auto &e : g.enemies)
            if (e.hp > 0)
                assert(e.stun == 90);
    }
    {
        Game g;
        g.start();
        g.energy = 39;
        g.special();
        assert(g.energy == 39 && !g.specialTime);
    }
    {
        Game g;
        g.start();
        keepWave(g);
        g.pickups = {};
        g.enemies[0] = {600, 790, 400};
        auto &b = g.enemies[0];
        b.role = Role::Boss;
        b.maxhp = 400;
        b.cool = 0;
        g.player.x = 500;
        b.y = g.player.y;
        g.enemyStep(b, 0);
        assert(b.windup == 32);
        int hp = g.player.hp;
        for (int k = 0; k < 31; ++k)
            g.enemyStep(b, 0);
        assert(g.player.hp == hp);
        g.player.x = 1000;
        g.enemyStep(b, 0);
        assert(g.player.hp == hp);
    }
    {
        Game g;
        g.start();
        g.jump = 10;
        g.damagePlayer(20);
        assert(g.player.hp == 100);
        g.jump = 0;
        g.blocked = true;
        g.damagePlayer(20);
        assert(g.player.hp == 96);
    }
    {
        Game g;
        g.start();
        Input i;
        i.dash = true;
        g.step(i);
        assert(g.dashTime == 8 && g.dashCool == 26);
        int hp = g.player.hp;
        g.damagePlayer(50);
        assert(g.player.hp == hp);
    }
    {
        Game g;
        g.start();
        for (int chapter = 0; chapter < 5; ++chapter)
            for (int wave = 1; wave <= 3; ++wave)
            {
                assert(g.chapter == chapter && g.wave == wave);
                clearEnemies(g);
                g.step({});
                assert(g.transition == 70);
                for (int k = 0; k < 70; ++k)
                    g.step({});
            }
        assert(g.scene == Scene::Victory);
    }
    {
        Game g;
        g.start();
        g.player.hp = 0;
        g.step({});
        assert(g.scene == Scene::Defeat);
        Input i;
        i.confirm = true;
        g.step(i);
        assert(g.scene == Scene::Arena && g.player.hp == 100 && g.chapter == 0);
    }
    {
        Game g;
        g.start();
        for (int n = 0; n < 12000 && g.scene == Scene::Arena; ++n)
        {
            Input i;
            i.x = (n % 100 < 50) ? 1 : -1;
            i.y = (n % 200 < 100) ? 1 : -1;
            i.attack = true;
            i.special = n % 60 == 0;
            i.monster = n % 150 == 0;
            i.redbull = n % 180 == 0;
            i.dash = n % 30 == 0;
            g.step(i);
            assert(g.player.x >= 100 && g.player.x <= 1820 && g.player.y >= 790 &&
                   g.player.y <= 950);
            assert(g.monsterCans >= 0 && g.monsterCans <= 5 && g.redbullCans >= 0 &&
                   g.redbullCans <= 5);
            assert(g.energy >= 0 && g.energy <= 100);
        }
    }
    std::cout << "20 gameplay scenarios passed\n";
}
