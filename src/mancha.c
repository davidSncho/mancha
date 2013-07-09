#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MILL_LENGTH 18
#define MILL_CENTER_X 46
#define MILL_CENTER_Y 115


#define SUN_OFFSET_X -10
#define SUN_OFFSET_Y -10

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


int sun_x[] = {72, 108, 131, 131, 131,  108,  72,  36,   13,  13,  13, 36};
int sun_y[] = {13,  13,  42,  84, 126,  155, 155, 155,  126,  84,  42, 13};


void mill_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;
  get_time(&t);
  int32_t second_angle = t.tm_sec * (0xffff/60);
  int32_t alpha_arm= 0x00600 ;
 // int32_t second_angle = t.tm_min * (0xffff/60 ) ;


  graphics_context_set_stroke_color(ctx, GColorBlack);
  //main arm
  //TODO : close arm contour
  for (int i = -alpha_arm   ; i<= alpha_arm  ; i+=0x000ff )
  {
    //close main arm contour
    graphics_draw_pixel(ctx,
                       GPoint(MILL_CENTER_X -  MILL_LENGTH * sin_lookup(second_angle + i )/0xffff,
                              MILL_CENTER_Y +  MILL_LENGTH * cos_lookup(second_angle + i )/0xffff )
                        );
    //draw orthogonal arms
    graphics_draw_line(ctx,
                 GPoint(MILL_CENTER_X -  MILL_LENGTH * cos_lookup(second_angle + i )/0xffff,
                        MILL_CENTER_Y -  MILL_LENGTH * sin_lookup(second_angle + i  )/0xffff ),
                 GPoint(MILL_CENTER_X +  MILL_LENGTH * cos_lookup(second_angle + i )/0xffff,
                        MILL_CENTER_Y +  MILL_LENGTH * sin_lookup(second_angle + i  )/0xffff )
                  );
    //draw opposite of main arm
    graphics_draw_line(ctx,
                 GPoint(MILL_CENTER_X,
                        MILL_CENTER_Y ),
                 GPoint(MILL_CENTER_X +  MILL_LENGTH * sin_lookup(second_angle + i )/0xffff,
                        MILL_CENTER_Y -  MILL_LENGTH * cos_lookup(second_angle + i )/0xffff )
                  );
    //erase main arm inside
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx,
         GPoint(MILL_CENTER_X,
                MILL_CENTER_Y ),
         GPoint(MILL_CENTER_X -  (MILL_LENGTH-3) * sin_lookup(second_angle + i   )/0xffff,
                MILL_CENTER_Y +  (MILL_LENGTH-3) * cos_lookup(second_angle + i   )/0xffff )
          );
    graphics_context_set_stroke_color(ctx, GColorBlack);
  }

   graphics_draw_line(ctx,
             GPoint(MILL_CENTER_X,
                    MILL_CENTER_Y ),
             GPoint(MILL_CENTER_X -  MILL_LENGTH * sin_lookup(second_angle + alpha_arm )/0xffff,
                    MILL_CENTER_Y +  MILL_LENGTH * cos_lookup(second_angle + alpha_arm )/0xffff )
              );
   graphics_draw_line(ctx,
             GPoint(MILL_CENTER_X,
                    MILL_CENTER_Y ),
             GPoint(MILL_CENTER_X -  MILL_LENGTH * sin_lookup(second_angle - alpha_arm )/0xffff,
                    MILL_CENTER_Y +  MILL_LENGTH * cos_lookup(second_angle - alpha_arm )/0xffff )
              );




}


//void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
void handle_seconde_tick(AppContextRef ctx, PebbleTickEvent *t) {
  PblTm current_time;
  get_time(&current_time);

  // position of the sun is computed according to hour
//  int h= current_time.tm_hour % 12 ;
   int h= current_time.tm_min % 12 ;
  //float m_float_norm= (current_time.tm_min)/(float) 60        ;
  float m_float_norm= (current_time.tm_sec)/(float) 60        ;

  int x_sun=  (int) (m_float_norm*sun_x[(h+1)%12] + (1-m_float_norm)*sun_x[h]) + SUN_OFFSET_X;
  int y_sun=  (int) (m_float_norm*sun_y[(h+1)%12] + (1-m_float_norm)*sun_y[h]) + SUN_OFFSET_Y;

  layer_set_frame  ( &sun_container.layer.layer,
                    GRect(x_sun, y_sun, 21, 21 )
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


//  handle_minute_tick(ctx, NULL);
    handle_seconde_tick(ctx, NULL);
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
//      .tick_handler= &handle_minute_tick,
      .tick_handler= &handle_seconde_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
