#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <lvgl.h>

#include "secrets.h"

LilyGo_Class amoled;

static const char* FW_VERSION = "v0.5.0";

static lv_obj_t* tileview;
static lv_obj_t* t1;              // Tile #1: start screen (FR-01)
static lv_obj_t* t2;              // Tile #2: settings screen (FR-04)
static lv_obj_t* t3;              // Tile #3: departure screen (FR-02)
static lv_obj_t* t1_label;
static lv_obj_t* t2_label;
static lv_obj_t* t3_label;

// Settings (FR-05, FR-06)
static const char* STOPS[] = {"Campus Grasvik", "Karlskrona C", "Karlskrona centrum", "Handelshamnen", "Lyckeby centrum"};
static const int STOP_COUNT = 5;
// Trafiklab stop IDs, same order as STOPS (FR-02, FR-05)
static const char* STOP_IDS[] = {"740032188", "740000230", "740023372", "740020699", "740000663"};
static const char* TYPE_OPTIONS = "All\nBus\nTrain\nFerry";
// The same types as Trafiklab names them in the live data, in dropdown order (FR-06)
static const char* TYPE_API[] = {"", "BUS", "TRAIN", "BOAT"};

static int current_stop = 0;
static int current_type = 0;      // 0 = All
static lv_obj_t* stop_dropdown;
static lv_obj_t* type_dropdown;
static lv_obj_t* departure_list;

// Live departure data from Trafiklab (FR-02)
static JsonDocument live_data;
static bool live_ok = false;      // true when the last download worked
static lv_obj_t* data_label;      // shows the Trafiklab attribution (NF-03)

// Function: Sets a tile to dark or light colours
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

// Function: Replaces å, ä, and ö, since the screen fonts do not have them
static String to_ascii(String s)
{
  s.replace("å", "a"); s.replace("ä", "a"); s.replace("ö", "o");
  s.replace("Å", "A"); s.replace("Ä", "A"); s.replace("Ö", "O");
  return s;
}

// Function: Downloads live departures for the chosen stop from Trafiklab (FR-02, FR-05)
static void load_live_departures()
{
  live_ok = false;
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String("https://realtime-api.trafiklab.se/v1/departures/") + STOP_IDS[current_stop] + "?key=" + TRAFIKLAB_API_KEY;

  WiFiClientSecure client;
  client.setInsecure();           // skip certificate check for now
  HTTPClient http;
  http.begin(client, url);
  int code = http.GET();
  Serial.printf("Live data for %s: HTTP %d\n", STOPS[current_stop], code);

  if (code == 200) {
    DeserializationError error = deserializeJson(live_data, http.getString());
    live_ok = !error;
  }
  http.end();
}

// Function: Shows the live departures for the chosen stop and transport type (FR-02)
static void show_live_departures()
{
  int shown = 0;
  for (JsonObject dep : live_data["departures"].as<JsonArray>()) {
    String type = dep["route"]["transport_mode"] | "";
    if (current_type != 0 && type != TYPE_API[current_type]) continue;

    String time = dep["scheduled"] | "";              // e.g. "2026-11-17T12:05:00"
    String line = dep["route"]["designation"] | "";
    String dest = to_ascii(dep["route"]["direction"] | "");
    int delay_min = (dep["delay"] | 0) / 60;          // delay is given in seconds
    bool cancelled = dep["canceled"] | false;

    String status = "On time";
    if (cancelled)          status = "Cancelled";
    else if (delay_min > 0) status = "+" + String(delay_min) + " min";

    String row = time.substring(11, 16) + "   " + line + "   " + dest + "   " + status;
    lv_obj_t* label = add_label(departure_list, row.c_str(), &lv_font_montserrat_20);
    lv_obj_set_width(label, lv_pct(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    if (cancelled)          lv_obj_set_style_text_color(label, lv_color_hex(0xFF5252), 0);  // red
    else if (delay_min > 0) lv_obj_set_style_text_color(label, lv_color_hex(0xFFD740), 0);  // yellow

    if (++shown >= 15) break;     // keep the list short
  }

  if (shown == 0) add_label(departure_list, "No departures of this type.", &lv_font_montserrat_20);
  lv_label_set_text(data_label, "Data: Trafiklab.se (CC BY 4.0)");
}

// Function: Shows the departures for the chosen stop and transport type (FR-02)
static void show_departures()
{
  lv_label_set_text(t3_label, STOPS[current_stop]);
  lv_obj_clean(departure_list);

  // Live data from Trafiklab (FR-02)
  if (live_ok) {
    show_live_departures();
    return;
  }
  // No live data: tell the user instead of showing old or fake departures
  add_label(departure_list, "No live data. Check the Wi-Fi connection.", &lv_font_montserrat_20);
  lv_label_set_text(data_label, "");
}

// Function: Loads and shows departures for the stop chosen in the dropdown (FR-05)
static void on_stop_changed(lv_event_t* e)
{
  LV_UNUSED(e);
  current_stop = lv_dropdown_get_selected(stop_dropdown);
  load_live_departures();         // get live data for the new stop (FR-05)
  show_departures();
}

// Function: Shows departures for the transport type chosen in the dropdown (FR-06)
static void on_type_changed(lv_event_t* e)
{
  LV_UNUSED(e);
  current_type = lv_dropdown_get_selected(type_dropdown);
  show_departures();
}

// Function: Goes to the departure screen
static void on_show_clicked(lv_event_t* e)
{
  LV_UNUSED(e);
  lv_obj_set_tile(tileview, t3, LV_ANIM_ON);
}

// Function: Goes back to the settings screen
static void on_change_clicked(lv_event_t* e)
{
  LV_UNUSED(e);
  lv_obj_set_tile(tileview, t2, LV_ANIM_ON);
}

// Function: Creates the three screens
static void create_ui()
{
  // Fullscreen Tileview
  tileview = lv_tileview_create(lv_scr_act());
  lv_obj_set_size(tileview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

  // Add three horizontal tiles (swipe between them, FR-03)
  t1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
  t2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);
  t3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR);

  // Tile #1: start screen (FR-01)
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

  // Tile #2: settings screen (FR-04, FR-05, FR-06)
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

  // Tile #3: departure screen (FR-02)
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

    data_label = add_label(t3, "", &lv_font_montserrat_20);
  }

  show_departures();
}

// Function: Connects to Wi-Fi
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
    Serial.println("WiFi connected.");
  } else {
    Serial.println("WiFi could not connect (timeout).");
  }
}

// Function: Asks Trafiklab for departures from one stop and prints them, as a start-up test (FR-02)
static void test_trafiklab(int stop)
{
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String("https://realtime-api.trafiklab.se/v1/departures/") + STOP_IDS[stop] + "?key=" + TRAFIKLAB_API_KEY;

  WiFiClientSecure client;
  client.setInsecure();           // skip certificate check for now
  HTTPClient http;
  http.begin(client, url);
  int code = http.GET();
  Serial.printf("Trafiklab %s: HTTP %d\n", STOPS[stop], code);   // 200 = OK, 400 = bad stop ID, 401 = bad key

  if (code == 200) {
    JsonDocument data;
    deserializeJson(data, http.getString());
    JsonArray deps = data["departures"].as<JsonArray>();
    Serial.printf("%d departures found\n", deps.size());

    // Print the first 5 departures
    int printed = 0;
    for (JsonObject dep : deps) {
      String time = dep["scheduled"] | "";
      const char* line = dep["route"]["designation"] | "";
      const char* dest = dep["route"]["direction"] | "";
      Serial.printf("  %s  %s  %s\n", time.substring(11, 16).c_str(), line, dest);
      if (++printed >= 5) break;
    }
  }
  http.end();
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

  // Test every stop ID once (results in the serial monitor)
  for (int i = 0; i < STOP_COUNT; i++) test_trafiklab(i);

  // Show live departures for the first stop (FR-02)
  load_live_departures();
  show_departures();
}

// Must have function: Loop runs continuously on device after setup
void loop()
{
  lv_timer_handler();
  delay(5);
}
