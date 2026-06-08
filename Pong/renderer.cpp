#include "utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global float render_scale = 0.01f;

struct Bitmap
{
    int width, height;
    u32* pixels;
};
internal void clear_screen(u32 color)
{
    u32* pixel = (u32*)render_state.memory;
    for (int y = 0; y < render_state.height; y++)
    {
        for (int x = 0; x < render_state.width; x++)
        {
            *pixel = color;
            pixel++;
        }
    }
}
internal void draw_rect_in_pixels(int x0, int y0, int x1, int y1, u32 color)
{
    x0 = clampInt(0, x0, render_state.width);
    x1 = clampInt(0, x1, render_state.width);
    y0 = clampInt(0, y0, render_state.height);
    y1 = clampInt(0, y1, render_state.height);
    for (int y = y0; y < y1; y++)
    {
        u32* pixel = (u32*)render_state.memory + x0 + y * render_state.width;
        for (int x = x0; x < x1; x++)
        {
            *pixel = color;
            pixel++;
        }
    }
}
internal void
draw_rect(float x, float y, float half_size_x, float half_size_y, u32 color)
{
    x *= render_state.height * render_scale;
    y *= render_state.height * render_scale;
    half_size_x *= render_state.height * render_scale;
    half_size_y *= render_state.height * render_scale;

    x += render_state.width / 2.0f;
    y += render_state.height / 2.0f;

    int x0 = x - half_size_x;
    int x1 = x + half_size_x;
    int y0 = y - half_size_y;
    int y1 = y + half_size_y;

    draw_rect_in_pixels(x0, y0, x1, y1, color);
}
internal Bitmap load_bitmap(const char* path)
{
    Bitmap result = {};
    int channels;
    result.pixels = (u32*)stbi_load(path, &result.width, &result.height, &channels, 4);
    return result;
}
internal void draw_char(Bitmap bitmap, char c, float x, float y, int scale = 4)
{
    x *= render_state.height * render_scale;
    y *= render_state.height * render_scale;

    x += render_state.width / 2.0f;
    y += render_state.height / 2.0f;

    int screen_x = (int)x;
    int screen_y = (int)y;

    const int glyphs_per_row = 13;
    const int glyph_width = 8;
    const int glyph_height = 8;
   

    int index = 0;

    if (c >= 'A' && c <= 'Z')
    {
        index = c - 'A';
    }
    if (c >= 'a' && c <= 'z')
    {
        index = 26 + (c - 'a');
    }
    if (c >= '0' && c <= '9')
    {
        index = 52 + (c - '0');
    }
    int glyph_x = index % glyphs_per_row;
    int glyph_y = index / glyphs_per_row;

    int src_x = glyph_x * glyph_width;
    int src_y = glyph_y * glyph_height;

    u32 background = bitmap.pixels[0];

    for (int y = 0; y < glyph_height; y++)
    {
        for (int x = 0; x < glyph_width; x++)
        {
            u32 color = bitmap.pixels[
                (src_y + (glyph_height - 1 - y)) * bitmap.width +
                    (src_x + x)
            ];

            if (color == background)
                continue;

            for (int py = 0; py < scale; py++)
            {
                for (int px = 0; px < scale; px++)
                {
                    u32* pixel =
                        (u32*)render_state.memory +
                        (screen_y + y * scale + py) * render_state.width +
                        (screen_x + x * scale + px);

                    *pixel = color;
                }
            }
        }
    }
}
internal void draw_text(Bitmap bitmap, const char* text, float x, float y, int scale = 4)
{
    while (*text)
    {
        draw_char(bitmap, *text, x, y, scale);
       
        x += 5*(scale/4); // character spacing
        text++;
    }
}