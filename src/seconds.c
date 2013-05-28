#include "commons.h"
#include "seconds.h"
	
Layer seconds_bar;

#define NUMBER_OF_BOXES 11
#define SECONDS_PADDING 5
#define SECONDS_SPACE 3
#define SECONDS_BOX 12
//layers representing 5 second boxes
Layer box_layer[NUMBER_OF_BOXES];
bool show_box[NUMBER_OF_BOXES] = {false};
	
void display_box(struct Layer *l, GContext *ctx) {
	uint8_t i = 0;
	while(&box_layer[i] != l) { //to find out which box we are updating
		i++;
		if(i == NUMBER_OF_BOXES) {
        graphics_context_set_fill_color(ctx, FRONT_COLOR);
        graphics_fill_rect(ctx, GRect(0,0, SECONDS_BOX/2, SECONDS_BOX/2), 0, GCornerNone); //ERROR! shown by half square
		return;
		}
	}
	if (show_box[i]) {
        graphics_context_set_fill_color(ctx, FRONT_COLOR);
        graphics_fill_rect(ctx, GRect(0,0, SECONDS_BOX, SECONDS_BOX), 0, GCornerNone);
    }
}

void count_seconds(unsigned short seconds_passed) {
	if (seconds_passed%5 != 0) {
		return;
	}
	
	short actual_box = seconds_passed/5 - 1;
	if (actual_box == -1) { //when seconds are in range 0-4 all layers are cleaned up
		for(uint8_t i=0; i<NUMBER_OF_BOXES; i++) {
			show_box[i] = false;
			layer_mark_dirty(&box_layer[i]);
		}
	}
	else {
		show_box[actual_box] = true;
		layer_mark_dirty(&box_layer[actual_box]);
	}
}

void display_seconds(struct Layer *l, GContext *ctx) {
}


void seconds_init() {
	layer_init(&seconds_bar, GRect(2*WIDTH_IMAGE, 0, 
								   144 - 2*WIDTH_IMAGE, 168));
	layer_set_update_proc(&seconds_bar, &display_seconds);
	layer_add_child(window_get_root_layer(&window), &seconds_bar);
	layer_mark_dirty(&seconds_bar);
	//initialising layers representing 5 second' box each (11 boxes = layers 0-10)
	for(uint8_t i=0;i<NUMBER_OF_BOXES;i++) {
		layer_init(&box_layer[i], GRect(SECONDS_PADDING, SECONDS_PADDING+ i*(SECONDS_SPACE+SECONDS_BOX), SECONDS_BOX, SECONDS_BOX)); //layer 0 doesn't exist
		layer_set_update_proc(&box_layer[i], &display_box);
	} 
	//finilised on first display with access to seconds var
}

void seconds_first_display(unsigned short seconds_passed) {
	for(uint8_t i=0; i <= seconds_passed/5; i++) {
		show_box[i] = true;	
	}
	for(uint8_t i=0;i<NUMBER_OF_BOXES;i++) { //finilising initialisation and automatic rendering by adding them as childs
		layer_add_child(&seconds_bar, &box_layer[i]);
	}
}