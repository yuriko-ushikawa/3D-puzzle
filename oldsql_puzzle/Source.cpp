#define _USE_MATH_DEFINES
#include<fstream>
#include<iostream>
#include<cassert>
#include<vector>
#include<string>
#include<cstdint>
#include <cmath>

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {

	return (a << 24) + (b << 16) + (g << 8) + r;

}

void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
	
	r = (color >> 0) & 255;
	g = (color >> 8) & 255;
	b = (color >> 16) & 255;
	a = (color >> 24) & 255;

}

void drop_image(const std::string filename, const std::vector<uint32_t>& image, const size_t w, const size_t h) {
	assert(image.size() == w * h);
	std::ofstream ofs(filename, std::ios::binary);
	ofs << "P6\n" << w << " " << h << "\n255\n";
	for (size_t i = 0; i < h * w; ++i) {
		uint8_t r, g, b, a;
		unpack_color(image[i], r, g, b, a);
		ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b); // why cast to char?
	}
	ofs.close();
}

void draw_rectangle(std::vector<uint32_t>& img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
	assert(img.size() == img_w * img_h);
	for (size_t i = 0; i < w; i++) {
		for (size_t j = 0; j < h; j++) {
			size_t cx = x + i;
			size_t cy = y + j;
			assert(cx < img_w&& cy < img_h);
			img[cx + cy * img_w] = color;
		}
	}
}



int main() {
	const size_t win_w = 1024; // image width
	const size_t win_h = 512; // image height

	std::vector<uint32_t> framebuffer(win_w * win_h, pack_color(255, 255, 255)); // prepare the image

	const size_t map_w = 16; // map width
	const size_t map_h = 16; // map height
	const char map[] = "0000222222220000"\
					   "1              0"\
					   "1      11111   0"\
					   "1     0        0"\
					   "0     0  1110000"\
					   "0     3        0"\
					   "0   10000      0"\
					   "0   0   11100  0"\
					   "0   0   0      0"\
					   "0   0   1  00000"\
					   "0       1      0"\
					   "2       1      0"\
					   "0       0      0"\
					   "0 0000000      0"\
					   "0              0"\
					   "0002222222200000"; // our game map
	assert(sizeof(map) == map_w * map_h + 1); // +1 for the null terminated string


	float player_x = 3.456; // player x position (in map coordinates [map_w,map_h], not image coordinates)
	float player_y = 2.345; // player y position 
	float player_a = 1.523; // player view direction
	const float fov = M_PI / 3.; // field of view

	const size_t ncolors = 10;
	std::vector<uint32_t> colors(ncolors);
	for (size_t i = 0; i < ncolors; i++) {
		colors[i] = pack_color(rand() % 255, rand() % 255, rand() % 255);
	}

	//for (size_t j = 0; j < win_h; j++) { // fill the screen with color gradients
	//	for (size_t i = 0; i < win_w; i++) {
	//		uint8_t r = 255 * j / float(win_h); // varies between 0 and 255 as j sweeps the vertical
	//		uint8_t g = 255 * i / float(win_w); // varies between 0 and 255 as i sweeps the horizontal
	//		uint8_t b = 0;
	//		framebuffer[i + j * win_w] = pack_color(r, g, b);
	//	}
	//}



	const size_t rect_w = win_w / (map_w*2);
	const size_t rect_h = win_h / map_h;
	for (size_t j = 0; j < map_h; j++) { // draw the map
		for (size_t i = 0; i < map_w; i++) {
			if (map[i + j * map_w] == ' ') continue; // skip empty spaces
			size_t rect_x = i * rect_w;
			size_t rect_y = j * rect_h;
			size_t icolor = map[i + j * map_w] - '0';
			assert(icolor < ncolors);
			draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, colors[icolor]);
		}
	}

	// draw the player on the map
	// draw_rectangle(framebuffer, win_w, win_h, player_x * rect_w, player_y * rect_h, 5, 5, pack_color(255, 255, 255));


	// draw the visibility cone
	for (size_t i = 0; i < win_w / 2; i++) { // draw the visibility cone AND the "3D" view
		float angle = player_a - fov / 2 + fov * i / float(win_w / 2);

		for (float t = 0; t < 20; t += .05) {
			float cx = player_x + t * cos(angle);
			float cy = player_y + t * sin(angle);

			size_t pix_x = cx * rect_w;
			size_t pix_y = cy * rect_h;
			framebuffer[pix_x + pix_y * win_w] = pack_color(160, 160, 160); 

			if (map[int(cx) + int(cy) * map_w] != ' ') { // our ray touches a wall, so draw the vertical column to create an illusion of 3D
				size_t icolor = map[int(cx) + int(cy) * map_w] - '0';
				assert(icolor < ncolors);
				size_t column_height = win_h / (t * cos(angle - player_a)); // fish-eye correction

				draw_rectangle(framebuffer, win_w, win_h, win_w / 2 + i, win_h / 2 - column_height / 2, 1, column_height, colors[icolor]);
				break;
			}
		}

	}
	drop_image("./out.ppm", framebuffer, win_w, win_h);

	return 0;
}
