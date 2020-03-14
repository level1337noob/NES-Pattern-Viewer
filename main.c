#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

u8 palette[0x40][0x4]; // rgba

u32 get_palette(u8 idx) {
	return (palette[idx][0] << 24) | (palette[idx][1] << 16) | (palette[idx][2] << 8) | 255;
}

void g(u8 idx, u8 c, u8 col) { palette[idx][c] = col; }
#define g(idx, r, _g, b)	\
	g(idx, 0x0, r), g(idx, 0x1, _g), g(idx, 0x2, b), g(idx, 0x3, 0xFF);

void set_palette_2c02() {
	g(0x0F, 0, 0, 0)
	g(0x1F, 0, 0, 0)
	g(0x2F, 0, 0, 0)
	g(0x3F, 0, 0, 0)
	g(0x0E, 0, 0, 0)
	g(0x1E, 0, 0, 0)
	g(0x2E, 0, 0, 0)
	g(0x3E, 0, 0, 0)
	g(0x00, 84, 84, 84)
	g(0x01, 0, 30, 116)
	g(0x02, 8, 16, 144)
	g(0x03, 48, 0, 136)
	g(0x04, 68, 0, 100)
	g(0x05, 92, 0, 48)
	g(0x06, 84, 4, 0)
	g(0x07, 60, 24, 0)
	g(0x08, 32, 42, 0)
	g(0x09, 8, 58, 0)
	g(0x0A, 0, 64, 0)
	g(0x0B, 0, 60, 0)
	g(0x0C, 0, 50, 60)
	g(0x0D, 0, 0, 0)
	g(0x10, 152, 150, 152)
	g(0x11, 8, 76, 196)
	g(0x12, 48, 50, 236)
	g(0x13, 92, 30, 228)
	g(0x14, 136, 20, 176)
	g(0x15, 160, 20, 100)
	g(0x16, 152, 34, 32)
	g(0x17, 120, 60, 0)
	g(0x18, 84, 90, 0)
	g(0x19, 40, 114, 0)
	g(0x1A, 8, 124, 0)
	g(0x1B, 0, 118, 40)
	g(0x1C, 0, 102, 120)
	g(0x1D, 0, 0, 0)
	g(0x20, 236, 238, 236)
	g(0x21, 76, 154, 236)
	g(0x22, 120, 124, 236)
	g(0x23, 176, 98, 236)
	g(0x24, 228, 84, 236)
	g(0x25, 236, 88, 180)
	g(0x26, 236, 106, 100)
	g(0x27, 212, 136, 32)
	g(0x28, 160, 170, 0)
	g(0x29, 116, 196, 0)
	g(0x2A, 76, 208, 32)
	g(0x2B, 56, 204, 108)
	g(0x2C, 56, 180, 204)
	g(0x2D, 60, 60, 60)
	g(0x30, 236, 238, 236)
	g(0x31, 168, 204, 236)
	g(0x32, 188, 188, 236)
	g(0x33, 212, 178, 236)
	g(0x34, 236, 174, 236)
	g(0x35, 236, 174, 212)
	g(0x36, 236, 180, 176)
	g(0x37, 228, 196, 144)
	g(0x38, 204, 210, 120)
	g(0x39, 180, 222, 120)
	g(0x3A, 168, 226, 144)
	g(0x3B, 152, 226, 180)
	g(0x3C, 160, 214, 228)
	g(0x3D, 160, 162, 160)
}

unsigned int filesize;
unsigned char *load_rom(const char *file)
{
	FILE *p = fopen(file, "rb");

	if (!p) {
		return NULL;
	}

	fseek(p, 0, SEEK_END);
	filesize = ftell(p);

	rewind(p);
	unsigned char *buffer = (unsigned char *) malloc(filesize);
	fread(buffer, filesize, 1, p);
	fclose(p);
	return buffer;
}

unsigned char *chr_rom;

// Maximum of 127 tiles aka 128 tiles selected
// from one nametable
// This selects based on each bitplane
void select_tile(u8 selected_tile[8][8],
		u8 tile_index, u8 nametable)
{
	u16 size = nametable ? 0x1000 : 0;
	u16 selection;
	if (tile_index > 127) {
		tile_index &= 0x7F;
	}
	
	selection = (0x10 * tile_index) + size;

	u8 vert = 0;
	for (u16 i = selection; i < selection + 8; i++) {
		u8 index = 0;
		for (u8 spr = 0x80; spr > 0; spr >>= 1) {
			u8 lsb = (chr_rom[0+i] & spr) << index;
			u8 msb = (chr_rom[8+i] & spr) << index;
			u8 sum;
			// Yeah this sums up everything to select the appropiate color
			sum = (lsb && msb) ? 3 : (msb) ? 2 : (lsb) ? 1 : 0;
			selected_tile[vert][index] = sum;
			index++;
		}

		vert++;
	}
}

SDL_Window *window;
SDL_Renderer *render;
SDL_Event ev;
SDL_Texture *tex;

u32 *framebuffer = 0;
int pitch;

void plot_pixel(int x, int y, u32 color)
{
	if (x >= 256 || y >= 128)	return;

	framebuffer[y * 256 + x] = color;
}


int s = 2;
void test(int x) { exit(0); }


void ScreenshotPNG(const char *filename)
{
	unsigned int format = SDL_PIXELFORMAT_ARGB8888;
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	SDL_Renderer *renderer = render;
	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, format);
	SDL_RenderReadPixels(renderer, NULL, format, surface->pixels, surface->pitch);
	IMG_SavePNG(surface, filename);
	SDL_FreeSurface(surface);
}


int main(int argc, char *argv[])
{
	signal(SIGINT, test);
	if (argc < 2) {
		printf("%s: usage nesrom scale paletteconfig\n", argv[0]);
		return 2;
	}

	if (argv[2]) {
		s = strtol(argv[2], 0, 10);
	}

	unsigned char *nes_rom = load_rom(argv[1]);

	unsigned int chr_offset;
	unsigned int chr_size;

	if (!nes_rom) {
		printf("Expected valid file\n");
		return 2;
	}

	chr_offset = 0x4000 * nes_rom[4];

	if (nes_rom[5] == 0) {
		printf("File is using CHR RAM\n");
		return 2;
	}

	u8 bank = 0;
	if (argv[3]) {
		bank = strtol(argv[3], 0, 10);
	}

	u8 pal0, pal1, pal2, pal3;

	pal0 = 0x3F;
	pal1 = 0x00;
	pal2 = 0x10;
	pal3 = 0x2D;


	chr_size = 0x2000 * nes_rom[5] + bank * 0x2000;
	if (0x2000 * nes_rom[5] * bank > filesize) {
		printf("File size exceeded\n");
		return 2;
	}

	chr_rom = nes_rom + chr_offset;
	printf("CHR ROM size 0x%04x\n", chr_size);

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("NES Tile Viewer", SDL_WINDOWPOS_UNDEFINED,
	SDL_WINDOWPOS_UNDEFINED, 256 * s, 128 * s, SDL_WINDOW_SHOWN);
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 256, 128);
	set_palette_2c02();

	SDL_Rect rescale;
	rescale.x = 0;
	rescale.y = 0;
	rescale.w = 256 * s;
	rescale.h = 2 * 128 * s;


	int frames = 0;
	float delta = 0.0f;
	float fps_limit = 60.01;
	while (1) {
		size_t st = SDL_GetPerformanceCounter();
		if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				break;
			if (ev.type == SDL_KEYDOWN) {
				switch (ev.key.keysym.scancode) {
				case SDL_SCANCODE_F2:
					ScreenshotPNG("preview1.png");
					goto err;
				}
			}
		}

		if (SDL_LockTexture(tex, 0, (void **) &framebuffer, &pitch)) {
			break;
		}

		u8 tile[8][8];

		u8 x = 0;
		u8 y = 0;
		int table_nr = 0;
		for (u32 sel = 0; sel <= 256; sel++) {
			select_tile(tile, sel & 0x7F, table_nr);

			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					int dx = table_nr ? 128 + x + j : x + j;
					switch (tile[i][j])
					{
					case 0:
						plot_pixel(dx, y+i, get_palette(pal0 & 0x3F));
						break;
					case 1:
						plot_pixel(dx, y+i, get_palette(pal1 & 0x3F));
						break;
					case 2:
						plot_pixel(dx, y+i, get_palette(pal2 & 0x3F));
						break;
					case 3:
						plot_pixel(dx, y+i, get_palette(pal3 & 0x3F));
						break;
					}
				}
			}

			x += 8;

			if (!table_nr) {
				if (sel >= 128) {
					x = y = 0;
					table_nr = 1;
				}
			}

			if (y >= 128) { x = 0; y = 0; }
			if (x >= 128) { y += 8; x = 0; }
		}


		SDL_UnlockTexture(tex);
		SDL_RenderClear(render);
		SDL_RenderCopy(render, tex, NULL, &rescale);
		SDL_RenderPresent(render);

		size_t freq = SDL_GetPerformanceFrequency();
		size_t et = SDL_GetPerformanceCounter();
		float seconds = (float)((et - st) * 1000) / (float)freq;
		frames++;
		delta += seconds;

		// 1 second elapsed at 60 frames
		if (delta >= fps_limit) {
			static char buffer[1024];
			if (frames >= 60) {
				frames = 60;
				SDL_Delay(1000.0 / delta);
			}

			memset(buffer, 0, 256);
			sprintf(buffer, "NES Tile Viewer FPS:%d @ %.2fms", frames, (1.0 / delta) * 1000.0);
			SDL_SetWindowTitle(window, buffer);
			frames = 0;
			delta = 0.0f;
		}
	}
err:
	SDL_DestroyTexture(tex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(render);
	free(nes_rom);
	return 0;
}
