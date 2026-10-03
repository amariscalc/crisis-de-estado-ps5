// Desktop visual QA using the same game and scene code. GPL-3.0-or-later.
#define main ps5_entry
#include "../src/main.cpp"
#undef main
#include <fstream>
#include <vector>
#include <cstdlib>
namespace
{
uint32_t simulatedButtons = 0;
struct Glyph
{
    char character;
    std::array<std::uint8_t, 7> rows;
};

constexpr std::array<Glyph, 37> glyphs{{
    {' ', {0, 0, 0, 0, 0, 0, 0}},        {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},      {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},     {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},   {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},       {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},   {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}}, {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}}, {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}}, {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}}, {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {7, 2, 2, 2, 18, 18, 12}},     {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}}, {'M', {17, 27, 21, 21, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}}, {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}}, {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}}, {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},       {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},  {'W', {17, 17, 17, 21, 21, 21, 10}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},  {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}},
}};

} // namespace
extern "C"
{
    int scePadInit()
    {
        return 0;
    }
    int scePadOpen(int, int, int, void *)
    {
        return 1;
    }
    int scePadReadState(int, PS5_PadData *p)
    {
        *p = {};
        p->connected = 1;
        p->leftStick.x = p->leftStick.y = 128;
        p->buttons = simulatedButtons;
        return 0;
    }
    int scePadClose(int)
    {
        return 0;
    }
    int sceUserServiceInitialize(void *)
    {
        return 0;
    }
    int sceUserServiceGetLoginUserIdList(int *ids)
    {
        ids[0] = 1;
        return 0;
    }
}
namespace ps5::demo
{
void Canvas::clear(Color color) noexcept
{
    rectangle(0, 0, 1920, 1080, color);
}
void Canvas::rectangle(unsigned x, unsigned y, unsigned w, unsigned h, Color color) noexcept
{
    if (x >= 1920 || y >= 1080)
        return;
    unsigned right = x + std::min(w, 1920 - x), bottom = y + std::min(h, 1080 - y);
    for (unsigned r = y; r < bottom; r++)
        for (unsigned col = x; col < right; col++)
            pixels_[r * 1920 + col] = unsigned(color);
}
void Canvas::circle(unsigned cx, unsigned cy, unsigned radius, Color color) noexcept
{
    for (int y = -int(radius); y <= int(radius); y++)
        for (int x = -int(radius); x <= int(radius); x++)
            if (x * x + y * y <= int(radius * radius))
            {
                int px = int(cx) + x, py = int(cy) + y;
                if (px >= 0 && py >= 0 && px < 1920 && py < 1080)
                    pixels_[py * 1920 + px] = unsigned(color);
            }
}
void Canvas::triangle(unsigned x, unsigned y, unsigned half, unsigned h, Color color) noexcept
{
    for (unsigned r = 0; r < h; r++)
    {
        unsigned hw = r * half / h;
        rectangle(x > hw ? x - hw : 0, y + r, hw * 2 + 1, 1, color);
    }
}
void Canvas::text(unsigned x, unsigned y, std::string_view text, unsigned scale,
                  Color color) noexcept
{
    for (char ch : text)
    {
        const Glyph *g = &glyphs.front();
        for (const auto &candidate : glyphs)
            if (candidate.character == ch)
                g = &candidate;
        for (unsigned r = 0; r < 7; r++)
            for (unsigned col = 0; col < 5; col++)
                if (g->rows[r] & (1 << (4 - col)))
                    rectangle(x + col * scale, y + r * scale, scale, scale, color);
        x += 6 * scale;
    }
}
[[noreturn]] void run(DrawScene, std::string_view) noexcept
{
    std::abort();
}
} // namespace ps5::demo
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    std::vector<uint32_t> pixels(1920 * 1080);
    auto c = Canvas::preview(pixels.data());
    // Exercise the actual PS5 input mapping with an emulated PadData record.
    auto input = poll();
    if (!input.connected)
        return 3;
    simulatedButtons = PS5_PAD_BUTTON_RIGHT;
    input = poll();
    if (input.choose != 1 || input.x != 1)
        return 4;
    input = poll();
    if (input.choose != 0)
        return 5; // rising-edge menu navigation
    simulatedButtons = PS5_PAD_BUTTON_SQUARE | PS5_PAD_BUTTON_CIRCLE;
    input = poll();
    if (!input.attack || !input.block)
        return 6;
    simulatedButtons = PS5_PAD_BUTTON_CROSS | PS5_PAD_BUTTON_TRIANGLE | PS5_PAD_BUTTON_OPTIONS;
    input = poll();
    if (!input.jump || !input.special || !input.pause || !input.confirm)
        return 7;
    simulatedButtons = 0;
    poll();
    for (int mode = 0; mode < 2; mode++)
    {
        if (mode)
        {
            simulatedButtons = PS5_PAD_BUTTON_CROSS;
        }
        draw(c);
        std::ofstream out(std::string(argv[1]) + (mode ? "-arena.ppm" : "-menu.ppm"),
                          std::ios::binary);
        out << "P6\n1920 1080\n255\n";
        for (auto px : pixels)
        {
            char rgb[3] = {char(px & 255), char((px >> 8) & 255), char((px >> 16) & 255)};
            out.write(rgb, 3);
        }
    }
}
