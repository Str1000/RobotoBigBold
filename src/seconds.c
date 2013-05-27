#include "commons.h"
#include "seconds.h"
	
Layer seconds_bar;

#define SECONDS_PADDING 5
#define SECONDS_SPACE 3
#define SECONDS_BOX 12

unsigned short Second_boxes[11] = {0};

void count_seconds(unsigned short seconds_passed) {
	if (seconds_passed%5 != 0) {
		return;
	}
	
	unsigned short actual_block = seconds_passed/5;
	if (actual_block == 0) {
		int i = 0;
		for(i=0; i<11; i++) {
			Second_boxes[i] = 0;
		}
	}
	else {
		Second_boxes[actual_block-1] = 1;
	}
	layer_mark_dirty(&seconds_bar);
}

void display_seconds(struct Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	int i = 0;
	for(i=0;i<11;i++) {
		unsigned short dimension = SECONDS_BOX*Second_boxes[i]; //if the box should be shown
		graphics_fill_rect(ctx, GRect(SECONDS_PADDING, SECONDS_PADDING+ i*(SECONDS_SPACE+SECONDS_BOX), dimension, dimension), 0, GCornerNone);
	}
}
	
void seconds_init() {
	layer_init(&seconds_bar, GRect(2*WIDTH_IMAGE, 0, 
								   144 - 2*WIDTH_IMAGE, 168));
	layer_set_update_proc(&seconds_bar, &display_seconds);
	layer_add_child(window_get_root_layer(&window), &seconds_bar);
}

void seconds_first_display(unsigned short seconds_passed) {
	unsigned short i;
	for(i=0; i<seconds_passed/5; i++) {
		Second_boxes[i] = 1;
	}
	layer_mark_dirty(&seconds_bar);
}