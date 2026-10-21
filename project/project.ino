#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <lvgl.h>

#include "secrets.h"

LilyGo_Class amoled;

static const char* FW_VERSION = "v0.4.0";

static lv_obj_t* tileview;
static lv_obj_t* t1;              // Tile #1: start screen (US1.1)
static lv_obj_t* t2;              // Tile #2: settings screen (US3.1)
static lv_obj_t* t3;              // Tile #3: departure screen (US1.2A)
static lv_obj_t* t1_label;
static lv_obj_t* t2_label;
static lv_obj_t* t3_label;

// Settings (US3.2A, US3.3)
static const char* STOPS[] = {"Campus Grasvik", "Karlskrona C", "Karlskrona centrum", "Handelshamnen", "Lyckeby centrum"};
static const int STOP_COUNT = 5;
static const char* TYPE_OPTIONS = "All\nBus\nTrain\nFerry";
static const char* TYPE_NAMES[] = {"", "Bus", "Train", "Ferry"};

static int current_stop = 0;
static int current_type = 0;      // 0 = All
static lv_obj_t* stop_dropdown;
static lv_obj_t* type_dropdown;
static lv_obj_t* departure_list;

// Demo departure data (US1.2A). Replaced by live Trafiklab data later.
struct Departure {
  int stop;                       // index in STOPS
  const char* time;
  const char* line;
  const char* destination;
  const char* type;               // "Bus", "Train" or "Ferry"
  int delay_min;                  // 0 = on time, -1 = cancelled
};

static Departure DEMO[] = {
  {0, "08:05", "1",    "Karlskrona C", "Bus",    0},
  {0, "08:15", "6",    "Lyckeby",      "Bus",    3},
  {0, "08:20", "1",    "Karlskrona C", "Bus",    0},
  {0, "08:50", "1",    "Karlskrona C", "Bus",   -1},
  {1, "08:02", "Kust", "Kalmar C",     "Train",  0},
  {1, "08:10", "1",    "Grasvik",      "Bus",    0},
  {1, "08:18", "Ore",  "Malmo C",      "Train",  7},
  {1, "08:40", "500",  "Ronneby",      "Bus",    0},
  {2, "08:04", "1",    "Grasvik",      "Bus",    0},
  {2, "08:09", "4",    "Hasslo",       "Bus",    0},
  {2, "08:14", "6",    "Lyckeby",      "Bus",    2},
  {3, "08:30", "Boat", "Aspo",         "Ferry",  0},
  {3, "09:15", "Boat", "Tjurko",       "Ferry",  0},
  {3, "10:00", "Boat", "Sturko",       "Ferry", -1},
  {4, "08:12", "6",    "Karlskrona C", "Bus",    0},
  {4, "08:27", "6",    "Karlskrona C", "Bus",    4},
};
static const int DEMO_COUNT = 16;

// Function: Tile #2 Color change
static void apply_tile_colors(lv_obj_t* tile, lv_obj_t* label, bool dark)
{
  // Background
  lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(tile, dark ? lv_color_black() : lv_color_white(), 0);

  // Text
  lv_obj_set_style_text_color(label, dark ? lv_color_white() : lv_color_black(), 0);
}

// Function: Creates a white label in the given font
static lv_obj_t* add_label(lv_obj_t* parent, const char* text, const lv_font_t* font)
{
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  return label;
}

// Function: Creates a button with a text on it
static lv_obj_t* add_button(lv_obj_t* parent, const char* text, lv_event_cb_t on_click)
{
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_t* label = add_label(btn, text, &lv_font_montserrat_20);
  lv_obj_center(label);
  lv_obj_add_event_cb(btn, on_click, LV_EVENT_CLICKED, NULL);
  return btn;
}

// Function: Places the children of a tile top to bottom
static void make_column(lv_obj_t* tile)
{
  lv_obj_set_style_pad_all(tile, 12, 0);
  lv_obj_set_style_pad_row(tile, 8, 0);
  lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
}

// Function: Shows the demo departures for the chosen stop and transport type (US1.2A)
static void show_departures()
{
  lv_label_set_text(t3_label, STOPS[current_stop]);
  lv_obj_clean(departure_list);

  int shown = 0;
  for (int i = 0; i < DEMO_COUNT; i++) {
    Departure& dep = DEMO[i];
    if (dep.stop != current_stop) continue;
    if (current_type != 0 && strcmp(dep.type, TYPE_NAMES[current_type]) != 0) continue;

    String status = "On time";
    if (dep.delay_min == -1)    status = "Cancelled";
    else if (dep.delay_min > 0) status = "+" + String(dep.delay_min) + " min";

    String row = String(dep.time) + "   " + dep.line + "   " + dep.destination + "   " + status;
    lv_obj_t* label = add_label(departure_list, row.c_str(), &lv_font_montserrat_20);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    if (dep.delay_min == -1)    lv_obj_set_style_text_color(label, lv_color_hex(0xFF5252), 0);  // red
    else if (dep.delay_min > 0) lv_obj_set_style_text_color(label, lv_color_hex(0xFFD740), 0);  // yellow
    shown++;
  }

  if (shown == 0) add_label(departure_list, "No departures of this type.", &lv_font_montserrat_20);
}

static void on_stop_changed(lv_event_t* e)
{
  LV_UNUSED(e);
  current_stop = lv_dropdown_get_selected(stop_dropdown);
  show_departures();
}

static void on_type_changed(lv_event_t* e)
{
  LV_UNUSED(e);
  current_type = lv_dropdown_get_selected(type_dropdown);
  show_departures();
}

static void on_show_clicked(lv_event_t* e)
{
  LV_UNUSED(e);
  lv_obj_set_tile(tileview, t3, LV_ANIM_ON);
}

static void on_change_clicked(lv_event_t* e)
{
  LV_UNUSED(e);
  lv_obj_set_tile(tileview, t2, LV_ANIM_ON);
}

// Function: Creates UI
static void create_ui()
{
  // Fullscreen Tileview
  tileview = lv_tileview_create(lv_scr_act());
  lv_obj_set_size(tileview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

  // Add three horizontal tiles (swipe between them, US2.1)
  t1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
  t2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
  t3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);

  // Tile #1: start screen (US1.1)
  {
    make_column(t1);
    lv_obj_set_flex_align(t1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    t1_label = lv_label_create(t1);
    lv_label_set_text(t1_label, "Public Transport Info on ESP32");
    lv_obj_set_style_text_font(t1_label, &lv_font_montserrat_28, 0);
    apply_tile_colors(t1, t1_label, /*dark=*/true);

    lv_obj_t* info = add_label(t1, "", &lv_font_montserrat_20);
    lv_label_set_text_fmt(info, "%s  |  Group 12", FW_VERSION);

    add_label(t1, "Albert Rogo",        &lv_font_montserrat_20);
    add_label(t1, "Anton Ohlander",     &lv_font_montserrat_20);
    add_label(t1, "Felix Forsman",      &lv_font_montserrat_20);
    add_label(t1, "Jeremy Andersson",   &lv_font_montserrat_20);
    add_label(t1, "Paulius Galiauskas", &lv_font_montserrat_20);
  }

  // Tile #2: settings screen (US3.1, US3.2A, US3.3)
  {
    make_column(t2);

    t2_label = lv_label_create(t2);
    lv_label_set_text(t2_label, "Settings");
    lv_obj_set_style_text_font(t2_label, &lv_font_montserrat_28, 0);
    apply_tile_colors(t2, t2_label, /*dark=*/true);

    String stop_options = "";
    for (int i = 0; i < STOP_COUNT; i++) {
      if (i > 0) stop_options += "\n";
      stop_options += STOPS[i];
    }
    stop_dropdown = lv_dropdown_create(t2);
    lv_dropdown_set_options(stop_dropdown, stop_options.c_str());
    lv_obj_set_width(stop_dropdown, lv_pct(100));
    lv_obj_add_event_cb(stop_dropdown, on_stop_changed, LV_EVENT_VALUE_CHANGED, NULL);

    type_dropdown = lv_dropdown_create(t2);
    lv_dropdown_set_options(type_dropdown, TYPE_OPTIONS);
    lv_obj_set_width(type_dropdown, lv_pct(100));
    lv_obj_add_event_cb(type_dropdown, on_type_changed, LV_EVENT_VALUE_CHANGED, NULL);

    add_button(t2, "Show departures " LV_SYMBOL_RIGHT, on_show_clicked);
  }

  // Tile #3: departure screen (US1.2A)
  {
    make_column(t3);

    t3_label = lv_label_create(t3);
    lv_obj_set_style_text_font(t3_label, &lv_font_montserrat_28, 0);
    apply_tile_colors(t3, t3_label, /*dark=*/true);

    add_button(t3, "Change stop", on_change_clicked);

    // Scrollable list of departures
    departure_list = lv_obj_create(t3);
    lv_obj_set_width(departure_list, lv_pct(100));
    lv_obj_set_flex_grow(departure_list, 1);
    lv_obj_set_flex_flow(departure_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(departure_list, lv_color_black(), 0);
    lv_obj_set_style_border_width(departure_list, 0, 0);

    add_label(t3, "Demo data", &lv_font_montserrat_20);
  }

  show_departures();
}

// Function: Connects to WIFI
static void connect_wifi()
{
  Serial.printf("Connecting to WiFi SSID: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000) {
    delay(250);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected.");
  } else {
    Serial.println("WiFi could not connect (timeout).");
  }
}

// Must have function: Setup is run once on startup
void setup()
{
  Serial.begin(115200);
  delay(200);

  if (!amoled.begin()) {
    Serial.println("Failed to init LilyGO AMOLED.");
    while (true) delay(1000);
  }

  beginLvglHelper(amoled);   // init LVGL for this board

  create_ui();
  lv_timer_handler();        // draw the start screen before waiting for WiFi
  connect_wifi();
}

// Must have function: Loop runs continously on device after setup
void loop()
{
  lv_timer_handler();
  delay(5);
}
