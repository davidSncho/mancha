#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MILL_LENGTH 18
#define MILL_CENTER_X 46
#define MILL_CENTER_Y 115


#define MY_UUID { 0x4B, 0x20, 0x5A, 0xDC, 0x2B, 0xF2, 0x45, 0x3E, 0xA2, 0xB4, 0xE0, 0x81, 0x76, 0x35, 0xB0, 0x0B }
PBL_APP_INFO(MY_UUID,
             "Mancha", "DM_cnsltng",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
BmpContainer bg_container;
BmpContainer  sun_container ;

Layer mill_layer;
GPoint mill_center ;


int sun_x[] = {62, 98, 120, 120, 120,  98,  62,  26,   3,  3,  3, 36};
int sun_y[] = { 3,  3,  32,  74, 116, 145, 145, 145, 116, 74, 32,  3};


void mill_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;
  get_time(&t);
  //int32_t second_angle = t.tm_sec * (0xffff/60);
  int32_t second_angle = t.tm_min * (0xffff/60 ) ;
  graphics_context_set_stroke_color(ctx, GColorBlack);


  for(int i=-MILL_LENGTH; i< MILL_LENGTH-2; i+=3)
  {
    graphics_fill_circle( ctx,
                        GPoint(MILL_CENTER_X +  i * sin_lookup(second_angle)/0xffff,
                               MILL_CENTER_Y -  i * cos_lookup(second_angle)/0xffff ),
                        2
                      );
    graphics_fill_circle( ctx,
                        GPoint(MILL_CENTER_X+ i * cos_lookup(second_angle )/0xffff,
                               MILL_CENTER_Y+ i * sin_lookup(second_angle )/0xffff ),
                        2
                      );
  }

  graphics_fill_circle( ctx,
                        GPoint(MILL_CENTER_X+ MILL_LENGTH * cos_lookup(second_angle )/0xffff,
                               MILL_CENTER_Y+ MILL_LENGTH * sin_lookup(second_angle )/0xffff ),
                        2
                      );

  graphics_draw_circle( ctx,
                        GPoint(MILL_CENTER_X + MILL_LENGTH * sin_lookup(second_angle )/0xffff,
                               MILL_CENTER_Y - MILL_LENGTH * cos_lookup(second_angle )/0xffff ),
                        2
                      );



}


void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  PblTm current_time;
  get_time(&current_time);

  // position of the sun is computed according to hour
  int h= current_time.tm_hour % 12 ;
  //int h= current_time.tm_min % 12 ;
  layer_set_frame  ( &sun_container.layer.layer,
                    GRect(sun_x[h], sun_y[h], 21, 21 )
                    ) ;
  //position of the mill wings is computed according to minutes
  //int m= current_time.tm_min ;
  layer_mark_dirty(&mill_layer) ;

}



void handle_init(AppContextRef ctx) {
  (void)ctx;
  mill_center= GPoint(MILL_CENTER_X, MILL_CENTER_Y);
  window_init(&window, "WF");
  window_stack_push(&window, true /* Animated */);
  resource_init_current_app(&RSRC_V0);
  bmp_init_container(RESOURCE_ID_MAIN_IMAGE, &bg_container);
  bmp_init_container(RESOURCE_ID_SUN_IMAGE, &sun_container);
  layer_add_child(&window.layer, &bg_container.layer.layer);

  layer_add_child(&window.layer, &sun_container.layer.layer);
  layer_insert_above_sibling(&sun_container.layer.layer, &bg_container.layer.layer) ;

  layer_init(&mill_layer, window.layer.frame);
  mill_layer.update_proc = &mill_layer_update_callback;
  layer_add_child(&window.layer, &mill_layer);
  layer_insert_above_sibling(&mill_layer, &bg_container.layer.layer) ;


  handle_minute_tick(ctx, NULL);
//    handle_seconde_tick(ctx, NULL);
}




void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  // Note: Failure to de-init this here will result in instability and
  //       unable to allocate memory errors.
  bmp_deinit_container(&sun_container);
  bmp_deinit_container(&bg_container);

}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info={
      .tick_handler= &handle_minute_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
