//this app uses basic logic from "BigTime" watchface made by Pebble Technology to display big numbers
//#include "pebble_fonts.h"
#include "commons.h"
#include "seconds.h"

#include "resource_ids.auto.h"

#define MY_UUID { 0xE2, 0x09, 0x22, 0xBE, 0x25, 0xBB, 0x47, 0xDA, 0x88, 0xA6, 0x33, 0x7C, 0x4D, 0x70, 0x64, 0x74 }
PBL_APP_INFO(MY_UUID, "Roboto Big Bold", "Tom Svoboda", 2, 0, RESOURCE_ID_IMAGE_MENU_ICON, APP_INFO_WATCH_FACE);




//
// There's only enough memory to load about 6 of 10 required images
// so we have to swap them in & out...
//
// We have one "slot" per digit location on screen.
//
// Because layers can only have one parent we load a digit for each
// slot--even if the digit image is already in another slot.
//
// Slot on-screen layout:
//     0 1
//     2 3
//
	
#define TOTAL_IMAGE_SLOTS 4

#define NUMBER_OF_IMAGES 20

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_BOLD_0, RESOURCE_ID_IMAGE_BOLD_1, RESOURCE_ID_IMAGE_BOLD_2,
  RESOURCE_ID_IMAGE_BOLD_3, RESOURCE_ID_IMAGE_BOLD_4, RESOURCE_ID_IMAGE_BOLD_5,
  RESOURCE_ID_IMAGE_BOLD_6, RESOURCE_ID_IMAGE_BOLD_7, RESOURCE_ID_IMAGE_BOLD_8,
  RESOURCE_ID_IMAGE_BOLD_9
};

BmpContainer image_containers[TOTAL_IMAGE_SLOTS];

#define EMPTY_SLOT -1

// The state is either "empty" or the digit of the image currently in
// the slot--which was going to be used to assist with de-duplication
// but we're not doing that due to the one parent-per-layer
// restriction mentioned above.
int image_slot_state[TOTAL_IMAGE_SLOTS] = {EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT, EMPTY_SLOT};


void load_digit_image_into_slot(int slot_number, int digit_value) {
  /*

     Loads the digit image from the application's resources and
     displays it on-screen in the correct location.

     Each slot is a quarter of the screen.

   */

  // TODO: Signal these error(s)?

  if ((slot_number < 0) || (slot_number >= TOTAL_IMAGE_SLOTS)) {
    return;
  }

  if ((digit_value < 0) || (digit_value > 10)) {
    return;
  }

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    return;
  }

  image_slot_state[slot_number] = digit_value;
  bmp_init_container(IMAGE_RESOURCE_IDS[digit_value], &image_containers[slot_number]);
  image_containers[slot_number].layer.layer.frame.origin.x = (slot_number % 2) * WIDTH_IMAGE;
  image_containers[slot_number].layer.layer.frame.origin.y = (slot_number / 2) * HEIGHT_IMAGE;
  layer_add_child(&window.layer, &image_containers[slot_number].layer.layer);

}


void unload_digit_image_from_slot(int slot_number) {
  /*

     Removes the digit from the display and unloads the image resource
     to free up RAM.

     Can handle being called on an already empty slot.

   */

  if (image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(&image_containers[slot_number].layer.layer);
    bmp_deinit_container(&image_containers[slot_number]);
    image_slot_state[slot_number] = EMPTY_SLOT;
  }

}

void check_and_change(unsigned short digit, unsigned short slot_number, bool hide) {
	if (hide) {
		unload_digit_image_from_slot(slot_number);
		return;
	}
	else if (image_slot_state[slot_number] != digit) {
		unload_digit_image_from_slot(slot_number);
		load_digit_image_into_slot(slot_number, digit);
		return;
	}
	else {
		return;
	}
}
	
void display_value(unsigned short value, unsigned short row_number, bool show_first_leading_zero) {
	/*

     Displays a numeric value between 0 and 99 on screen.

     Rows are ordered on screen as:

       Row 0
       Row 1

     Includes optional blanking of first leading zero,
     i.e. displays ' 0' rather than '00'.
	 
	*/
	value = value % 100; // Maximum of two digits per row.
	
	// Column order is: | Column 0 | Column 1 |
	// (We process the columns in reverse order because that makes
	// extracting the digits from the value easier.)
	for (int column_number = 1; column_number >= 0; column_number--) {
		int slot_number = (row_number * 2) + column_number;
		if ((value == 0) && (column_number == 0) && !show_first_leading_zero) { //if number shouldn't be present on the screen
			check_and_change(0, slot_number, true);
		}
		else { 
			check_and_change(value % 10, slot_number, false);
		}
		value = value / 10;
	}
}


unsigned short get_display_hour(unsigned short hour) {

  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;

}


void display_time(PblTm *tick_time) {
  display_value(get_display_hour(tick_time->tm_hour), 0, false);
  display_value(tick_time->tm_min, 1, true);
  count_seconds(tick_time->tm_sec);
}


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)t;
  (void)ctx;

  display_time(t->tick_time);
}


void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "RobotoBig watch");
  window_stack_push(&window, true);
  window_set_background_color(&window, BACK_COLOR);
	
  seconds_init();
	
  resource_init_current_app(&APP_RESOURCES);

  // Avoids a blank screen on watch start.
  PblTm tick_time;

  get_time(&tick_time);
  display_time(&tick_time);
  seconds_first_display(tick_time.tm_sec);
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  for (int i = 0; i < TOTAL_IMAGE_SLOTS; i++) {
    unload_digit_image_from_slot(i);
  }

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,

    .tick_info = {
      .tick_handler = &handle_second_tick,
      .tick_units = SECOND_UNIT
    }

  };
  app_event_loop(params, &handlers);
}
