// Desktop visual QA: exact shared scene and texture renderer. GPL-3.0-or-later.
#include "../src/main.cpp"
#include <fstream>
#include <vector>
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    std::vector<uint32_t> pixels(1920 * 1080);
    auto c = Canvas::preview(pixels.data());
    for (int mode = 0; mode < 3; ++mode)
    {
        if (mode == 1)
        {
            game.start();
            game.banner = 0;
            game.player.x = 530;
            game.player.y = 855;
            game.player.monster = 200;
            game.player.redbull = 130;
            game.combo = 12;
            game.comboTime = 60;
            game.energy = 83;
            game.popularity = 35;
            game.score = 18450;
            game.enemies[0].monster = 190;
            game.enemies[0].x = 970;
            game.enemies[0].y = 865;
            game.enemies[1].redbull = 150;
            game.enemies[1].x = 1280;
            game.enemies[1].y = 900;
            game.enemies[2].x = 1600;
            game.enemies[2].y = 820;
        }
        if (mode == 2)
        {
            game.chapter = 4;
            game.wave = 3;
            game.spawn();
            game.banner = 0;
            game.enemies[0].windup = 24;
            game.enemies[0].targetX = game.player.x;
            game.enemies[0].targetY = game.player.y;
            game.enemies[0].monster = 220;
            game.selected = 3;
            game.player.leader = 3;
            game.specialTime = 25;
        }
        draw(c);
        if (!artReady)
            return 3;
        const char *names[] = {"-menu.ppm", "-arena.ppm", "-boss.ppm"};
        std::ofstream out(std::string(argv[1]) + names[mode], std::ios::binary);
        out << "P6\n1920 1080\n255\n";
        for (auto px : pixels)
        {
            char b[3] = {char(px & 255), char((px >> 8) & 255), char((px >> 16) & 255)};
            out.write(b, 3);
        }
        if (!out)
            return 4;
    }
}
