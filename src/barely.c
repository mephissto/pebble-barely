#include <pebble.h>

enum {
	KEY_INVERTED  = 0,
	KEY_BLUETOOTH = 1,
	KEY_VIBE      = 2,
  KEY_THEME     = 3
};

#define PERSIST_INVERTED  1
#define PERSIST_BLUETOOTH 2
#define PERSIST_VIBE      2
#define PERSIST_THEME     3

static Window *window;
static Layer *canvas;

static const uint32_t const segmentsFull[] = { 100 };
static const uint32_t const segmentsQuarterPast[] = { 100, 100, 100 };
static const uint32_t const segmentsHalf[] = { 100, 100, 100, 100, 100 };
static const uint32_t const segmentsQuarterTo[] = { 100, 100, 100, 100, 100, 100, 100 };

// Theme IDs
enum theme_id {
  THEME_BLUE   = 0,
  THEME_GREEN  = 1,
  THEME_RED    = 2,
  THEME_PURPLE = 3
};

static VibePattern patternFull = {
	.durations = segmentsFull,
	.num_segments = 1,
};
static VibePattern patternQuarterPast = {
	.durations = segmentsQuarterPast,
	.num_segments = 3,
};
static VibePattern patternHalf = {
	.durations = segmentsHalf,
	.num_segments = 5,
};
static VibePattern patternQuarterTo = {
	.durations = segmentsQuarterTo,
	.num_segments = 7,
};

static Layer *layers[4];
int digits[4];

// Configuration
bool invertInterface = false;
bool bluetoothIndicator = false;
bool vibeTime = false;
static uint8_t themeSetting;

bool bluetoothConnection = true;

static unsigned short get_display_hour(unsigned short hour) {
	if (clock_is_24h_style()) {
		return hour;
	}
	unsigned short display_hour = hour % 12;
	return display_hour ? display_hour : 12;
}

void drawHorizontalLine(GContext* ctx, GPoint start, GPoint goal) {
	graphics_fill_rect(ctx, GRect(start.x, start.y - 1, goal.x - start.x, 3), 0, GCornerNone);
}

void drawVerticalLine(GContext* ctx, GPoint start, GPoint goal) {
	graphics_fill_rect(ctx, GRect(start.x - 1, start.y - 1, 3, goal.y - start.y + 3), 0, GCornerNone);
}

void renderNumber(int number, GContext* ctx) {
	if (number == 1) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(22,26));
		drawVerticalLine(ctx, GPoint(22,26), GPoint(22,55));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(22,55));
		drawVerticalLine(ctx, GPoint(47,0), GPoint(47,55));
		drawHorizontalLine(ctx, GPoint(47,55), GPoint(71,55));
	} else if (number == 2) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(71,55));
	} else if (number == 3) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else if (number == 4) {
		drawVerticalLine(ctx, GPoint(35,0), GPoint(35,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(35,55));
		drawVerticalLine(ctx, GPoint(35,55), GPoint(35,83));
	} else if (number == 5) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(71,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else if (number == 6) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(71,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(47,55));
	} else if (number == 7) {
		drawHorizontalLine(ctx, GPoint(0,26), GPoint(35,26));
		drawVerticalLine(ctx, GPoint(35,26), GPoint(35,83));
	} else if (number == 8) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(22,55), GPoint(47,55));
	} else if (number == 9) {
		drawHorizontalLine(ctx, GPoint(22,26), GPoint(47,26));
		drawHorizontalLine(ctx, GPoint(0,55), GPoint(47,55));
	} else {
		drawVerticalLine(ctx, GPoint(35,26), GPoint(35,55));
	}
}

void canvas_update_callback(Layer *layer, GContext* ctx) {
	if (invertInterface) {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRichBrilliantLavender);
    }
#else
    graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	}
  else {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  graphics_context_set_fill_color(ctx, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorPurple);
    }
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  }

	if (bluetoothConnection) {
		graphics_fill_rect(ctx, GRect(71,0,3,168), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0,83,144,3), 0, GCornerNone);
	} else {
		graphics_fill_rect(ctx, GRect(71,0,1,168), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(73,0,1,168), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0,83,144,1), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0,85,144,1), 0, GCornerNone);
	}
}

void handle_bluetooth_con(bool connected) {
	bluetoothConnection = connected;
	layer_mark_dirty(canvas);
}

void topLeft_update_callback(Layer *layer, GContext* ctx) {
	if (invertInterface) {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRichBrilliantLavender);
    }
#else
    graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	}
  else {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  graphics_context_set_fill_color(ctx, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorPurple);
    }
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  }
	renderNumber(digits[0], ctx);
}

void topRight_update_callback(Layer *layer, GContext* ctx) {
	if (invertInterface) {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRichBrilliantLavender);
    }
#else
    graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	}
  else {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  graphics_context_set_fill_color(ctx, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorPurple);
    }
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  }
	renderNumber(digits[1], ctx);
}

void bottomLeft_update_callback(Layer *layer, GContext* ctx) {
		if (invertInterface) {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRichBrilliantLavender);
    }
#else
    graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	}
  else {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  graphics_context_set_fill_color(ctx, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorPurple);
    }
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  }
	renderNumber(digits[2], ctx);
}

void bottomRight_update_callback(Layer *layer, GContext* ctx) {
	if (invertInterface) {
#ifdef PBL_COLOR
    if (THEME_BLUE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRichBrilliantLavender);
    }
#else
    graphics_context_set_fill_color(ctx, GColorWhite);
#endif
	}
  else {
#ifdef PBL_COLOR
    if (THEME_BLUE == themeSetting) {
  	  graphics_context_set_fill_color(ctx, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      graphics_context_set_fill_color(ctx, GColorPurple);
    }
#else
    graphics_context_set_fill_color(ctx, GColorBlack);
#endif
  }
	renderNumber(digits[3], ctx);
}

// Refresh watchface after settings have changed
void refresh_watchface() {
  // Inverted & color settings
  if (invertInterface) {
#ifdef PBL_COLOR
    if (THEME_BLUE == themeSetting) {
  	  window_set_background_color(window, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      window_set_background_color(window, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      window_set_background_color(window, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      window_set_background_color(window, GColorPurple);
    }
#else
    window_set_background_color(window, GColorBlack);
#endif
  } 
  else {
#ifdef PBL_COLOR
    if (THEME_BLUE == themeSetting) {
  	  window_set_background_color(window, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      window_set_background_color(window, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      window_set_background_color(window, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      window_set_background_color(window, GColorRichBrilliantLavender);
    }
#else
    window_set_background_color(window, GColorWhite);
#endif
  }
  layer_mark_dirty(layers[0]);
  layer_mark_dirty(layers[1]);
  layer_mark_dirty(layers[2]);
  layer_mark_dirty(layers[3]);
  
  // Bluetooth
  if (bluetoothIndicator) {
    bluetooth_connection_service_subscribe(handle_bluetooth_con);
    bluetoothConnection = bluetooth_connection_service_peek();
  }
  else {
    bluetooth_connection_service_unsubscribe();
    bluetoothConnection = true;
  }
  
  // Mark the layer dirty
  layer_mark_dirty(canvas);
}

void process_tuple(Tuple *t) {
	int key = t->key;
	char string_value[32];
	strcpy(string_value, t->value->cstring);
  
  // KEY INVERTED
	if (key == KEY_INVERTED) {
		if (strcmp(string_value, "on") == 0) {
			invertInterface = true;
		} 
    else {
			invertInterface = false;
		}
	}
  
  // KEY THEME
  else if (key == KEY_THEME) {
    if (strcmp(string_value, "blue") == 0) {
      themeSetting = THEME_BLUE;
    }
    else if (strcmp(string_value, "green") == 0) {
      themeSetting = THEME_GREEN;
    }
    else if (strcmp(string_value, "red") == 0) {
      themeSetting = THEME_RED;
    }
    else if (strcmp(string_value, "purple") == 0) {
      themeSetting = THEME_PURPLE;
    }
  }
  
  // KEY BLUETOOTH
  else if (key == KEY_BLUETOOTH) {
		if (strcmp(string_value, "on") == 0) {
			bluetoothIndicator = true;
		} else {
			bluetoothIndicator = false;
		}
	}
  
  // KEY VIBE
  else if (key == KEY_VIBE) {
		if (strcmp(string_value, "on") == 0) {
			vibeTime = true;
		} else {
			vibeTime = false;
		}
	}
  
  // Refresh the watchface to take new setting(s) into account
  refresh_watchface();
}

void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *t = dict_read_first(iter);
	if(t) {
		process_tuple(t);
	}
  	while(t != NULL) {
		t = dict_read_next(iter);
		if(t) {
      process_tuple(t);
		}
	}
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (digits[1] != hour % 10) {
		digits[1] = hour % 10;
		layer_mark_dirty(layers[1]);
	}
	if (digits[0] != hour / 10 % 10) {
		digits[0] = hour / 10 % 10;
		layer_mark_dirty(layers[0]);
	}
	if (digits[3] != minute % 10) {
		digits[3] = minute % 10;
		layer_mark_dirty(layers[3]);
	}
	if (digits[2] != minute / 10 % 10) {
		digits[2] = minute / 10 % 10;
		layer_mark_dirty(layers[2]);
	}

	if (vibeTime && minute % 15 == 0) {
		int phase = minute / 15;
		if (phase == 0) {
			vibes_enqueue_custom_pattern(patternFull);
		} else if (phase == 1) {
			vibes_enqueue_custom_pattern(patternQuarterPast);
		} else if (phase == 2) {
			vibes_enqueue_custom_pattern(patternHalf);
		} else if (phase == 4) {
			vibes_enqueue_custom_pattern(patternQuarterTo);
		}
	}
}

void handle_init(void) {
	window = window_create();
	window_stack_push(window, true);

	app_message_register_inbox_received(in_received_handler);
	app_message_open(512, 512);

	bluetoothIndicator = persist_exists(PERSIST_BLUETOOTH) ? persist_read_bool(PERSIST_BLUETOOTH) : false;
	invertInterface = persist_exists(PERSIST_INVERTED) ? persist_read_bool(PERSIST_INVERTED) : false;
	vibeTime = persist_exists(PERSIST_VIBE) ? persist_read_bool(PERSIST_VIBE) : false;
  themeSetting = persist_exists(PERSIST_THEME) ? persist_read_int(PERSIST_THEME) : THEME_BLUE;
  

	if (invertInterface) {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  window_set_background_color(window, GColorBlue);
    }
    if (THEME_GREEN == themeSetting) {
      window_set_background_color(window, GColorGreen);
    }
    if (THEME_RED == themeSetting) {
      window_set_background_color(window, GColorRed);
    }
    if (THEME_PURPLE == themeSetting) {
      window_set_background_color(window, GColorPurple);
    }
#else
    window_set_background_color(window, GColorBlack);
#endif
	} else {
#ifdef PBL_COLOR
		if (THEME_BLUE == themeSetting) {
  	  window_set_background_color(window, GColorCeleste);
    }
    if (THEME_GREEN == themeSetting) {
      window_set_background_color(window, GColorMintGreen);
    }
    if (THEME_RED == themeSetting) {
      window_set_background_color(window, GColorMelon);
    }
    if (THEME_PURPLE == themeSetting) {
      window_set_background_color(window, GColorRichBrilliantLavender);
    }
#else
    window_set_background_color(window, GColorWhite);
#endif
	}

	if (bluetoothIndicator) {
		bluetooth_connection_service_subscribe(handle_bluetooth_con);
		bluetoothConnection = bluetooth_connection_service_peek();
	} else {
		bluetoothConnection = true;
	}

	struct tm *tick_time;
	time_t temp = time(NULL);
	tick_time = localtime(&temp);
	int hour = get_display_hour(tick_time->tm_hour);
	int minute = tick_time->tm_min;
	if (digits[1] != hour % 10) {
		digits[1] = hour % 10;
	}
	if (digits[0] != hour / 10 % 10) {
		digits[0] = hour / 10 % 10;
	}
	if (digits[3] != minute % 10) {
		digits[3] = minute % 10;
	}
	if (digits[2] != minute / 10 % 10) {
		digits[2] = minute / 10 % 10;
	}

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	canvas = layer_create(bounds);
	layer_set_update_proc(canvas, canvas_update_callback);
	layer_add_child(window_layer, canvas);

	layers[0] = layer_create(GRect(0,0,71,83));
	layer_set_update_proc(layers[0], topLeft_update_callback);
	layer_add_child(window_layer, layers[0]);

	layers[1] = layer_create(GRect(73,0,71,83));
	layer_set_update_proc(layers[1], topRight_update_callback);
	layer_add_child(window_layer, layers[1]);

	layers[2] = layer_create(GRect(0,85,71,83));
	layer_set_update_proc(layers[2], bottomLeft_update_callback);
	layer_add_child(window_layer, layers[2]);

	layers[3] = layer_create(GRect(73,85,71,83));
	layer_set_update_proc(layers[3], bottomRight_update_callback);
	layer_add_child(window_layer, layers[3]);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
	if (bluetoothIndicator) {
		bluetooth_connection_service_unsubscribe();
	}
	window_destroy(window);
	persist_write_bool(PERSIST_INVERTED, invertInterface);
	persist_write_bool(PERSIST_BLUETOOTH, bluetoothIndicator);
	persist_write_bool(PERSIST_VIBE, vibeTime);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
