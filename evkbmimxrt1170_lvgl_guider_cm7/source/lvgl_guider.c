/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

//#include "FreeRTOS.h"
//#include "task.h"

#include <core/lv_obj_scroll.h>
#include "fsl_debug_console.h"
#include "lvgl_support.h"
#include "pin_mux.h"
#include "board.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

#include "fsl_soc_src.h"

// dinesh added
#include "fsl_pit.h"
// Dinesh CAN driver
#include "fsl_flexcan.h"
#include "FlexCan_3.h"
//dinesh added
//#include "Dinesh_TFT_PinConfig.h"

// Screen task
#include "Screen_Task.h"

#include "left_place_draw.h"
#include "right_place_draw.h"
#include "arc_line.h"
#include "drawing_speed_RPM.h"

#include "Battery_draw.h"
#include "Temp_draw.h"
#include "draw_Task.h"

//hide the odometer roller
//#define HIDE_ODO 1
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_lvgl_initialized = false;
lv_ui guider_ui;

static volatile uint32_t s_tick        = 0U;
static volatile bool s_lvglTaskPending = false;

unsigned int  cnt1=0;
unsigned char image_1_flag = 0;
unsigned char image_2_flag = 0;
unsigned char update_screen_flag = 0;

volatile bool pitIsrFlag = false;
volatile bool pitIsrFlag_drawing = false;
volatile bool pitIsrFlag_drawing_date_time = false;
volatile bool pitIsrFlag_GUI = false;
volatile uint16_t pitIsrFlag_GUI_count;
volatile uint8_t first_screen;

volatile uint8_t odo_update_counter;

volatile uint32_t rider_screen;
volatile uint8_t rider_screen_inc_enable;
volatile uint8_t button_case;


volatile uint32_t drawing_screen;
volatile uint32_t drawing_screen_arc;
volatile uint32_t drawing_screen_inc_enable;
volatile uint32_t drawing_screen_arc_inc_enable;
volatile int16_t arc_value;
volatile 	uint8_t arc_clock_wise;

volatile uint32_t drawing_screen_speed_rpm ;
volatile uint32_t drawing_screen_speed_rpm_inc_enable ;

volatile uint32_t fuel_drawing_screen_arc;
volatile uint32_t fuel_drawing_screen_arc_inc_enable;
volatile int16_t  fuel_arc_value_red;
volatile int16_t  fuel_arc_value_green;
volatile	uint8_t  fuel_arc_clock_wise;
volatile uint8_t start_red_arc;
volatile uint8_t start_green_arc;

volatile uint32_t right_drawing_screen;
volatile uint32_t right_drawing_screen_inc_enable;

volatile 	uint8_t rider_screen_info = 0;
volatile 	uint8_t rider_screen_info_inc_enable = 0;


volatile uint32_t scroll_menu_screen;
volatile uint8_t scroll_menu_screen_inc_enable;
volatile uint32_t scroll_menu_screen_sub_Menu;

volatile uint32_t roller_menu_screen;
volatile uint8_t  roller_menu_screen_inc_enable;
volatile uint32_t roller_menu_screen_sub_Menu;


volatile uint8_t  left_start_drawing;
volatile uint8_t  right_start_drawing;

volatile uint8_t  battery_left_start_drawing;
volatile uint8_t  temp_start_drawing;

volatile uint8_t  draw_first_time_start = 1;

volatile uint32_t battery_drawing_screen;
volatile uint32_t battery_drawing_screen_inc_enable;

volatile uint32_t temp_drawing_screen;
volatile uint32_t temp_drawing_screen_inc_enable;


volatile uint32_t drawing_screen_timer;
volatile uint32_t drawing_screen_timer_inc_enable;


volatile bool first_start = true;

volatile uint16_t roll_ODO_0;

volatile bool RTC_update_in_while_Flag = false;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_SetupTick(void);
#if LV_USE_LOG
static void print_cb(lv_log_level_t level, const char *file, uint32_t line, const char *func, const char *buf);
#endif


//volatile unsigned char time[] = "10:28 AM";
unsigned char one_sec_Var;
unsigned char one_min_Var;
unsigned char one_hr_Var;
unsigned char AM_PM_Var;


extern volatile bool txComplete;
extern volatile bool rxComplete;
//extern flexcan_mb_transfer_t txXfer, rxXfer;
//extern flexcan_handle_t flexcanHandle;

/* Structure of initialize PIT */
pit_config_t pitConfig;
int cnt = 0, indic = 0 , dir_rotate = 0;

//extern volatile flexcan_frame_t frame;
extern volatile uint8_t button_state_pressed;
extern volatile uint8_t button_state_released;

#if LV_USE_LOG
static void print_cb(lv_log_level_t level, const char *file, uint32_t line, const char *func, const char *buf)
{
    /*Use only the file name not the path*/
    size_t p;

    for (p = strlen(file); p > 0; p--)
    {
        if (file[p] == '/' || file[p] == '\\')
        {
            p++; /*Skip the slash*/
            break;
        }
    }

    static const char *lvl_prefix[] = {"Trace", "Info", "Warn", "Error", "User"};

    PRINTF("\r%s: %s \t(%s #%d %s())\n", lvl_prefix[level], buf, &file[p], line, func);
}
#endif

static void AppTask(void *param)
{
#if LV_USE_LOG
    lv_log_register_print_cb(print_cb);
#endif

    lv_port_pre_init();
    lv_init();
    lv_port_disp_init();
}

/* 1 ms per tick. */
#ifndef LVGL_TICK_MS
#define LVGL_TICK_MS 1U
#endif

/* lv_task_handler is called every 5-tick. */
#ifndef LVGL_TASK_PERIOD_TICK
#define LVGL_TASK_PERIOD_TICK 5U
#endif

#define DEMO_PIT_BASEADDR PIT1
#define DEMO_PIT_CHANNEL  kPIT_Chnl_0
#define DEMO_PIT_GUI_CHANNEL  kPIT_Chnl_1
#define PIT_LED_HANDLER   PIT1_IRQHandler
#define PIT_IRQ_ID        PIT1_IRQn
/* Get source clock for PIT driver */
#define PIT_SOURCE_CLOCK CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)
#define LED_INIT()       USER_LED_INIT(LOGIC_LED_OFF)
#define LED_TOGGLE()     USER_LED_TOGGLE()

#define DEMO_FB_ALIGN 64
 #define DEMO_FB_SIZE \
(((DEMO_BUFFER_WIDTH * DEMO_BUFFER_HEIGHT * LCD_FB_BYTE_PER_PIXEL) + DEMO_FB_ALIGN - 1) & ~(DEMO_FB_ALIGN - 1))

/*******************************************************************************
 * Code
 ******************************************************************************/
void PIT_LED_HANDLER(void)
{
	// channel-0 check flag
	if(PIT_GetStatusFlags(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL))
	{
		/* Clear interrupt flag.*/
		PIT_ClearStatusFlags(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, kPIT_TimerFlag);
		pitIsrFlag = true;
		pitIsrFlag_drawing = true;
		pitIsrFlag_drawing_date_time = true;

		if(first_screen<5)
		{
			first_screen++;
		}

		if(rider_screen_info_inc_enable == 1)
		{
			if(rider_screen_info<8)
			{
				rider_screen_info++;
			}
		}

		if(drawing_screen_timer_inc_enable)
		{
			drawing_screen_timer++;
		}
	}
	// channel-1 check flag  --> 1 msec
	if(PIT_GetStatusFlags(DEMO_PIT_BASEADDR, DEMO_PIT_GUI_CHANNEL))
	{
		/* Clear interrupt flag.*/
		PIT_ClearStatusFlags(DEMO_PIT_BASEADDR, DEMO_PIT_GUI_CHANNEL, kPIT_TimerFlag);
		pitIsrFlag_GUI_count++;
		if(pitIsrFlag_GUI_count == 500)
		{
			pitIsrFlag_GUI = true;
			pitIsrFlag_GUI_count = 0;
		}

		if(rider_screen_inc_enable == 1)
		{
			rider_screen++;
		}

		if(scroll_menu_screen_inc_enable == 1)
		{
			scroll_menu_screen++;
		}

		if(roller_menu_screen_inc_enable == 1)
		{
			roller_menu_screen++;
		}

		if(drawing_screen_inc_enable == 1)
		{
			drawing_screen++;
//			drawing_screen_arc++;
		}
		if(drawing_screen_arc_inc_enable == 1)
		{
			drawing_screen_arc++;
		}
		if(right_drawing_screen_inc_enable == 1)
		{
			right_drawing_screen++;
		}
		if(fuel_drawing_screen_arc_inc_enable == 1)
		{
			fuel_drawing_screen_arc++;
		}
		if(drawing_screen_speed_rpm_inc_enable  == 1)
		{
			drawing_screen_speed_rpm++;
		}
		if(battery_drawing_screen_inc_enable == 1)
		{
			battery_drawing_screen++;
		}
		if(temp_drawing_screen_inc_enable == 1)
		{
			temp_drawing_screen++;
		}

	}


    /* Added for, and affects, all PIT handlers. For CPU clock which is much larger than the IP bus clock,
     * CPU can run out of the interrupt handler before the interrupt flag being cleared, resulting in the
     * CPU's entering the handler again and again. Adding DSB can prevent the issue from happening.
     */
    __DSB();
}

//function to display rider information
int display_rider_info(void)
{
	lv_scr_load_anim(guider_ui.rider_selected_screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	//lv_task_handler();  // Update the display
	rider_screen = 0;
	rider_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(rider_screen < 300)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	rider_screen_inc_enable = 0;

	rider_screen_info = 0;
	rider_screen_info_inc_enable = 1;
    // Display Logo
	while(rider_screen_info < 5)
	{
		__asm volatile ("nop");

	}

	rider_screen_info = 0;
	rider_screen_info_inc_enable = 0;

}

#define CANVAS_WIDTH  300
#define CANVAS_HEIGHT  420

void left_draw_line_2(void)
{
static lv_point_t line_points_1[] = { {39, 309}, {24, 218}};

    /*Create style*/
    static lv_style_t style_line_1;
    lv_style_init(&style_line_1);
    lv_style_set_line_width(&style_line_1, 13);
    lv_style_set_line_color(&style_line_1, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_line_rounded(&style_line_1, false);

    /*Create a line and apply the new style*/
    lv_obj_t * line2;
    line2 = lv_line_create(guider_ui.screen_drawing_container_drawing);
    lv_line_set_points(line2, line_points_1, 2);     /*Set the points*/
    lv_obj_add_style(line2, &style_line_1, 0);

}

void left_draw_line_3(void)
{

    static lv_point_t line_points_2[] = { {65, 153}, {180, 112}};

    /*Create style*/
    static lv_style_t style_line_2;
    lv_style_init(&style_line_2);
    lv_style_set_line_width(&style_line_2, 12);
    lv_style_set_line_color(&style_line_2, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_line_rounded(&style_line_2, false);

    /*Create a line and apply the new style*/
    lv_obj_t * line3;
    line3 = lv_line_create(guider_ui.screen_drawing_container_drawing);
    lv_line_set_points(line3, line_points_2, 2);     /*Set the points*/
    lv_obj_add_style(line3, &style_line_2, 0);

}

void lv_ex_line_1(void)
{
//	left_start_drawing = 1;
//	while(1)
//	{
		left_draw_line_1();
		right_draw_line_1();


//		lv_task_handler();  // Update the display
//	}


}

void draw_canvas(void)
{
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.radius = 0;
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
    rect_dsc.border_width = 0;
    rect_dsc.border_opa = LV_OPA_TRANSP;
    rect_dsc.border_color = lv_color_white();

    static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(CANVAS_WIDTH, CANVAS_HEIGHT)];

    lv_obj_t * canvas = lv_canvas_create(guider_ui.screen_drawing_container_drawing);
    //lv_obj_center(canvas);
    lv_obj_set_pos(canvas, 0, 0);
    lv_obj_set_size(canvas, CANVAS_WIDTH, CANVAS_HEIGHT);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED);  //LV_IMG_CF_TRUE_COLOR_ALPHA  // --> org --> LV_IMG_CF_TRUE_COLOR
    lv_canvas_fill_bg(canvas, LV_COLOR_CHROMA_KEY, LV_OPA_TRANSP);

//	// width = 100 , height 10
//    lv_canvas_draw_rect(canvas, 100, 145, 100, 10, &rect_dsc); //x = 100, y = 145
//    lv_canvas_draw_rect(canvas, 100, 125, 100, 10, &rect_dsc); //x = 90, y = 145

    //https://forum.lvgl.io/t/i-want-to-filled-triangle/1047
    //https://github.com/lvgl/lvgl/issues/932
    static lv_point_t points[] = { {10,10}, {200,100}, {100,220} };
    static lv_draw_line_dsc_t line_style;
    //LV_BLEND_MODE_NORMAL
    line_style.blend_mode = LV_BLEND_MODE_NORMAL;
    lv_canvas_draw_line(canvas, points, 3, &line_style);

	drawing_screen = 0;
	drawing_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(drawing_screen < 600)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	drawing_screen_inc_enable = 0;

    /*Test the rotation. It requires an other buffer where the orignal image is stored.
     *So copy the current image to buffer and rotate it to the canvas*/
    static lv_color_t cbuf_tmp[CANVAS_WIDTH * CANVAS_HEIGHT];
    memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
    lv_img_dsc_t img;
    img.data = (void *)cbuf_tmp;
    img.header.cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED;
    img.header.w = CANVAS_WIDTH;
    img.header.h = CANVAS_HEIGHT;

    lv_canvas_fill_bg(canvas, LV_COLOR_CHROMA_KEY, LV_OPA_TRANSP);
    lv_canvas_transform(canvas, &img, 450, LV_IMG_ZOOM_NONE, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, true);

	drawing_screen = 0;
	drawing_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(drawing_screen < 600)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	drawing_screen_inc_enable = 0;

//    memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
//    img.data = (void *)cbuf_tmp;
//    img.header.cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED;
//    img.header.w = CANVAS_WIDTH;
//    img.header.h = CANVAS_HEIGHT;
//
//	// width = 100 , height 10
//    lv_canvas_draw_rect(canvas, 40, 145, 100, 10, &rect_dsc); //x = 100, y = 145
//    lv_canvas_fill_bg(canvas, LV_COLOR_CHROMA_KEY, LV_OPA_TRANSP);
////    void lv_canvas_copy_buf(lv_obj_t * canvas, const void * to_copy, lv_coord_t x, lv_coord_t y, lv_coord_t w,
////                            lv_coord_t h);
//    lv_canvas_transform(canvas, &img, 0, LV_IMG_ZOOM_NONE, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, true);

}

void test_1(void)
{
	lv_draw_rect_dsc_t rect_dsc;
	lv_draw_rect_dsc_init(&rect_dsc);
	rect_dsc.radius = 10;
	rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
	rect_dsc.border_width = 2;
	rect_dsc.border_opa = LV_OPA_90;
	rect_dsc.border_color = lv_color_white();

	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)*4];

	lv_obj_t * canvas = lv_canvas_create(guider_ui.screen_drawing_container_drawing);
	lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
	lv_obj_center(canvas);
	lv_canvas_fill_bg(canvas, lv_color_make(0xff, 0xff, 0xff), 0);

	lv_canvas_draw_rect(canvas, 70, 60, 100, 70, &rect_dsc);

	static lv_color_t cbuf_tmp[CANVAS_WIDTH * CANVAS_HEIGHT];
	memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
	lv_img_dsc_t img;
	img.data = (void *)cbuf_tmp;
	img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	img.header.w = CANVAS_WIDTH;
	img.header.h = CANVAS_HEIGHT;

	lv_canvas_fill_bg(canvas, lv_color_make(0xff, 0xff, 0xff), 0);
	lv_canvas_transform(canvas, &img, 30, LV_IMG_ZOOM_NONE, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, true);
}

//start the Demo of drawing
int Set_drawing_demo(void)
{
	//lv_obj_add_flag(guider_ui.screen_drawing_cluster_image, LV_OBJ_FLAG_HIDDEN);
	lv_scr_load_anim(guider_ui.screen_drawing, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	lv_task_handler();  // Update the display


	drawing_screen = 0;
	drawing_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(drawing_screen < 600)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	drawing_screen_inc_enable = 0;

	if(draw_first_time_start)
	{
		arc_value = 0;
		arc_clock_wise = 1;

		drawing_screen = 0;
		drawing_screen_inc_enable = 1;

		drawing_screen_arc = 0;
		drawing_screen_arc_inc_enable = 1;

		fuel_drawing_screen_arc = 0;
		fuel_drawing_screen_arc_inc_enable = 1;
		fuel_arc_value_red = 19;   //working value -- 19 to 0
		fuel_arc_value_green = 64;  // working value -- 63 to 0
		fuel_arc_clock_wise = 0;
		start_red_arc = 1;
		start_green_arc = 0;

		drawing_speed_dir = 0;
		drawing_rpm_dir = 0;
		drawing_screen_speed_rpm = 0;
		drawing_screen_speed_rpm_inc_enable = 1;
		drawing_speed_val = 20;
		drawing_rpm_val = 100;

		left_start_drawing = 1;

		battery_left_start_drawing = 1;
		temp_start_drawing = 1;


		draw_first_time_start = 0;
	}

	drawing_screen_timer = 0;
	drawing_screen_timer_inc_enable = 1;
//	lv_ex_line_1();
	//draw_canvas();
	//test_1();
	while(1)
	{
		lv_ex_line_1();
		arc_line();
		fuel_arc_line();
		drawing_speed_rpm_update();
		battery_draw();
		temp_draw();

    	if(pitIsrFlag_drawing_date_time)
    	{
    		draw_RTC_Animate();
    		draw_date_Time_update();

    		pitIsrFlag_drawing_date_time = false;

    	}

    	lv_task_handler();  // Update the display

    	if(drawing_screen_timer == 10)
    	{
    		drawing_screen_timer = 0;
    		drawing_screen_timer_inc_enable = 0;
    		break;
    	}
    }
	drawing_screen_inc_enable = 0;

	return 0;
}

//start the Demo of cluster
int Set_cluster_demo(void)
{
	//lv_scr_load_anim(guider_ui.Logo_Screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 70, 0, false);
	//lv_scr_load_anim(guider_ui.screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 10, 0, false);
	lv_scr_load_anim(guider_ui.screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	lv_task_handler();  // Update the display

	rider_screen = 0;
	rider_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(rider_screen < 600)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	rider_screen_inc_enable = 0;

//	first_screen = 0;
//	while(first_screen != 2)
//	{
//		__asm volatile ("nop");
//
//	}



	if(first_start)
	{
		first_start = 0;
		//txComplete = 1;   //----> disable CAN TX
		rxComplete = 1;
	}

	__asm volatile ("nop");

//    FLexCAN_3_Init();
//    FLexCAN_3_RX_Msg();

    for (;;)
    {

		if(rxComplete)
		{
//			FLexCAN_3_RX_Msg();
////    			FLEXCAN_TransferReceiveNonBlocking(CAN3, &flexcanHandle, &rxXfer);
//			rxComplete = false;
//			__asm("nop");
//			frame.dataByte0++;
//			if(frame.dataByte0== 250)
//			{
//				frame.dataByte0 = 0;
//			}
		}

    	// 1 sec timer : Update RTC & Animat Teltail
    	if(pitIsrFlag)
    	{
    		//Odo task
//    		roll_ODO_0++;
//    		if(roll_ODO_0 == 10)
//    		{
//    			roll_ODO_0 = 0;
//    		}
			#ifndef HIDE_ODO
    		odo_update_counter++;
			if(odo_update_counter == 2)
			{
				odo_update_counter = 0;
				ODO_Animate();
				Screen_ODO_update();
			}
            #endif

            if(txComplete)
            {
        		// send CAN message
//        		FLexCAN_3_TX_Msg();
            	txComplete = false;
            	__asm("nop");
            };

    		RTC_Animate();
    		Screen_date_Time_update();
    		Screen_teltail_Animate();
    		Screen_speed_update();

    		pitIsrFlag = false;
    	}

    	// GUI update : 500 msec
    	if(pitIsrFlag_GUI)
    	{

        	if(dir_rotate == 0)  // clock wise
        	{
        		indic++;
        		if(indic == 60)
        		{
        			indic = 60;
        			dir_rotate = 1;
        		}
        	}
        	else  // counter-clock wise
        	{
        		indic--;
        		if(indic == 0)
        		{
        			indic = 0;
        			dir_rotate =0;
        		}
        	}
        	//lv_meter_set_indicator_value(screen_meter_1, screen_meter_1_scale_1_ndimg_0, indic);
        	//lv_meter_set_indicator_value(guider_ui.screen_meter_1, guider_ui.screen_meter_1_scale_1_ndimg_0, indic);
        	lv_meter_set_indicator_value(guider_ui.screen_Dial_meter, guider_ui.screen_Dial_meter_scale_1_ndimg_0, indic);

        	pitIsrFlag_GUI = false;
    	}

    	lv_task_handler();  // --> Display the content

    } /* should never get here */
}

// start the rider animation
int rider_animation(void)
{
	//lv_obj_update_snap(guider_ui.panel, LV_ANIM_ON);
	//lv_scr_load_anim(guider_ui.Rider_screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	lv_scr_load_anim(guider_ui.Rider_screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	//lv_scr_load(guider_ui.Rider_screen);
	rider_screen = 0;
	rider_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(rider_screen < 36)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}

	rider_screen = 0;
	button_case = 1;

	while(1)
	{
		if(rider_screen >= 1000)
		{
			//lv_task_handler();  // Update the display
			if(button_case == 1)
			{
				lv_obj_clear_flag(guider_ui.Rider_screen_Rider_selection_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_3, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
				button_case = 2;
			}
			else if(button_case == 2)
			{
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.Rider_screen_Rider_selection_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_3, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
				button_case = 3;
			}
			else if(button_case == 3)
			{
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.Rider_screen_Rider_selection_3, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
//				button_case = 10;
//			}
				button_case = 4;
			}
			else if(button_case == 4)
			{
				lv_obj_scroll_by(guider_ui.panel,-221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 5;
			}
			else if(button_case == 5)
			{
				lv_obj_scroll_by(guider_ui.panel,-221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 6;
			}
			else if(button_case == 6)
			{
				lv_obj_scroll_by(guider_ui.panel,-221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 7;
			}
			else if(button_case == 7)
			{
				lv_obj_scroll_by(guider_ui.panel,221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 8;
			}
			else if(button_case == 8)
			{
				lv_obj_scroll_by(guider_ui.panel,221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 9;
			}
			else if(button_case == 9)
			{
				lv_obj_scroll_by(guider_ui.panel,221,0,LV_ANIM_ON);
				__asm volatile ("nop");

				button_case = 10;
			}
			else if(button_case == 10)
			{
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.Rider_screen_Rider_selection_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.Rider_screen_Rider_selection_3, LV_OBJ_FLAG_HIDDEN);
				__asm volatile ("nop");

				button_case = 11;
			}
			else if(button_case == 11)
			{
				//lv_obj_update_snap(guider_ui.panel, LV_ANIM_OFF);
				lv_event_send(guider_ui.btn2,LV_EVENT_PRESSED,NULL);
				__asm volatile ("nop");

				button_case = 12;
			}
//			else if(button_case == 12)
//			{
//				__asm volatile ("nop");
//
//				if(button_state_pressed)
//				{
//					lv_event_send(guider_ui.btn2,LV_EVENT_RELEASED,NULL);
//					button_state_pressed = 0 ;
//					button_case = 13;
//				}
//
//
//			}
//			else if(button_case == 13)
//			{
//				__asm volatile ("nop");
//				if(button_state_released)
//				{
//					button_state_released = 0;
//					button_case = 14;
//				}
//
//
//			}
			else
			{

				if(button_state_pressed)
				{
					//lv_event_send(guider_ui.btn2,LV_EVENT_RELEASED,NULL);
					button_state_pressed = 0 ;
				}
				__asm volatile ("nop");
				rider_screen = 0;
				rider_screen_inc_enable = 0;
				return 1;
			}

			//lv_task_handler();  // Update the display
			rider_screen = 0;
		}

//		if(button_state_pressed)
//		{
//			lv_event_send(guider_ui.btn2,LV_EVENT_RELEASED,NULL);
//			button_state_pressed = 0 ;
//		}

		lv_task_handler();  // Update the display
	}
}

// start the roller menu animation
int roll_menu_animation(void)
{
	//lv_scr_load_anim(guider_ui.Rider_screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	lv_scr_load_anim(guider_ui.screen_menu_roller, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	//lv_scr_load(guider_ui.Rider_screen);
	roller_menu_screen = 0;
	roller_menu_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(roller_menu_screen < 200)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	roller_menu_screen = 0;
	button_case = 1;
	roller_menu_screen_sub_Menu = 1;

    // Display Logo
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
    lv_roller_set_selected(guider_ui.roller1,1,LV_ANIM_ON);
    roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
    // Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
    lv_roller_set_selected(guider_ui.roller1,2,LV_ANIM_ON);
    roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
    // Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
	lv_roller_set_selected(guider_ui.roller1,3,LV_ANIM_ON);
	roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	// Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
	lv_roller_set_selected(guider_ui.roller1,4,LV_ANIM_ON);
	roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	// Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
	lv_roller_set_selected(guider_ui.roller1,5,LV_ANIM_ON);
	roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	// Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	//change the screen roller
	lv_roller_set_selected(guider_ui.roller1,0,LV_ANIM_ON);
	roller_menu_screen = 0;
	while(roller_menu_screen < 800)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	// Display Logo
	roller_menu_screen = 0;
	while(roller_menu_screen < 2000)
	{
		__asm volatile ("nop");

	}

	roller_menu_screen_inc_enable = 0;

//	while(1)
//	{
//		__asm volatile ("nop");
//	}

}

// start the Scroll menu animation
int scroll_menu_animation(void)
{
	//lv_scr_load_anim(guider_ui.Rider_screen, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	lv_scr_load_anim(guider_ui.screen_menu_scroll, LV_SCR_LOAD_ANIM_FADE_ON, 10, 0, false);
	//lv_scr_load(guider_ui.Rider_screen);
	rider_screen = 0;
	scroll_menu_screen = 0;
	scroll_menu_screen_inc_enable = 1;
	// 35 msec : keep calling : lv_task_handler()
	while(scroll_menu_screen < 80)  // Minimum : delay for 10 msec --> i.e animation time --> till that time call --> lv_task_handler()
	{
		lv_task_handler();  // Update the display

	}
	scroll_menu_screen = 0;
	button_case = 1;
	scroll_menu_screen_sub_Menu = 1;

	while(1)
	{
		if(scroll_menu_screen >= 1000)
		{
			//lv_task_handler();  // Update the display
			if(button_case == 1)
			{
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_3, LV_OBJ_FLAG_HIDDEN);

				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Distance   72.5 Km");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Time   1.36 M");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_3, "Avg. Speed   60 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_4, "Max. Speed   70 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_5, "RESET");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
				button_case = 2;
			}
			else if(button_case == 2)
			{
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_3, LV_OBJ_FLAG_HIDDEN);

				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Distance   182.5 Km");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Time   4.26 M");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_3, "Avg. Speed   40 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_4, "Max. Speed   50 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_5, "RESET");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");

				if(scroll_menu_screen_sub_Menu ==1)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
				    lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if (scroll_menu_screen_sub_Menu ==2)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if(scroll_menu_screen_sub_Menu ==3)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
				    lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

					scroll_menu_screen_sub_Menu = 1;
					button_case = 3;
				}
				else
				{

				}

			}
			else if(button_case == 3)  // Mode
			{
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_3, LV_OBJ_FLAG_HIDDEN);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "SPORTS");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "ECO");
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");

				if(scroll_menu_screen_sub_Menu ==1)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
				    lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if (scroll_menu_screen_sub_Menu ==2)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if(scroll_menu_screen_sub_Menu ==3)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
				    lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

					scroll_menu_screen_sub_Menu = 1;
					button_case = 4;
				}
				else
				{

				}
//				button_case = 10;
//			}
				//button_case = 4;
			}
			else if(button_case == 4)  // Fuel
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,-135,LV_ANIM_ON);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Fuel Level    MID");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Average       60 KM/L");
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
				__asm volatile ("nop");

				button_case = 5;
				//button_case = 6;
			}
			else if(button_case == 5) //Tire
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,-135,LV_ANIM_ON);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Front    30 PSI");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Back     35 PSI");
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");

				button_case = 6;
			}
			else if(button_case == 6) // settings
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,-131,LV_ANIM_ON);

				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Default   Settings");
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
				__asm volatile ("nop");
//
				button_case = 7;
			}
			else if(button_case == 7) //tyre
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,131,LV_ANIM_ON);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Front    30 PSI");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Back     35 PSI");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
				__asm volatile ("nop");

				button_case = 8;
			}
			else if(button_case == 8) // fuel
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,135,LV_ANIM_ON);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Fuel Level    MID");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Average       60 KM/L");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
				__asm volatile ("nop");

				button_case = 9;
			}
			else if(button_case == 9) // Mode
			{
				lv_obj_scroll_by(guider_ui.panel_scroll_menu,0,135,LV_ANIM_ON);

				//text level
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "SPORTS");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "ECO");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");

				button_case = 10;
			}
			else if (button_case == 10) // trip-2
			{
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_Menu_select_3, LV_OBJ_FLAG_HIDDEN);

				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_1, "Distance   182.5 Km");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_2, "Time   4.26 M");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_3, "Avg. Speed   40 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_4, "Max. Speed   50 Km/h");
				lv_label_set_text(guider_ui.screen_menu_scroll_Menu_select_label_5, "RESET");
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_1, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(guider_ui.screen_menu_scroll_Menu_select_label_5, LV_OBJ_FLAG_HIDDEN);

				// arrow
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
				lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);

				__asm volatile ("nop");
				//button_case = 10;

				if(scroll_menu_screen_sub_Menu ==1)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if (scroll_menu_screen_sub_Menu ==2)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if(scroll_menu_screen_sub_Menu ==3)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

				}
				else if(scroll_menu_screen_sub_Menu ==4)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

				}
				else if(scroll_menu_screen_sub_Menu ==5)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

				}
				else if(scroll_menu_screen_sub_Menu ==6)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

				}
				else if(scroll_menu_screen_sub_Menu ==7)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

				}
				else if (scroll_menu_screen_sub_Menu ==8)
				{
					// arrow
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;
				}
				else if(scroll_menu_screen_sub_Menu ==9)
				{
					// arrow
					lv_obj_clear_flag(guider_ui.screen_menu_scroll_menu_item_1, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_2, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_3, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_4, LV_OBJ_FLAG_HIDDEN);
					lv_obj_add_flag(guider_ui.screen_menu_scroll_menu_item_5, LV_OBJ_FLAG_HIDDEN);
					scroll_menu_screen_sub_Menu++;

					scroll_menu_screen_sub_Menu = 1;
					button_case = 11;
				}
				else
				{

				}
			}
			else
			{
				__asm volatile ("nop");
				scroll_menu_screen_inc_enable = 1;
				return 1;
			}

			//lv_task_handler();  // Update the display
			scroll_menu_screen = 0;
		}

		lv_task_handler();  // Update the display
	}
}

#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
#define RPI_ADDR  0x45
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))
#define RPI_I2C_BASEADDR  LPI2C5

void talk_to_raspi(void)
{
	uint8_t tmp;
	uint8_t brightness = 0x80;

    BOARD_LPI2C_Init(RPI_I2C_BASEADDR, LPI2C_CLOCK_FREQUENCY);

    VIDEO_DelayMs(2000);

    //init_cmd_check
    //read_reg(RPI_ADDR, 0x80, &tmp);
    BOARD_LPI2C_Receive(RPI_I2C_BASEADDR, RPI_ADDR, 0x80, 1, &tmp, 1);
    PRINTF("reg 0x80 is 0x%x\r\n", tmp);

    //rpi_display_screen_power_up
    //write_reg(RPI_ADDR, 0x85, 0x00);
    tmp = 0;
    BOARD_LPI2C_Send(RPI_I2C_BASEADDR, RPI_ADDR, 0x85, 1, &tmp, 1);
    VIDEO_DelayMs(800);
    //write_reg(RPI_ADDR, 0x85, 0x01);
    tmp = 0x01;
    BOARD_LPI2C_Send(RPI_I2C_BASEADDR, RPI_ADDR, 0x85, 1, &tmp, 1);
    VIDEO_DelayMs(800);
    //write_reg(RPI_ADDR, 0x81, 0x04);
    tmp = 0x04;
    BOARD_LPI2C_Send(RPI_I2C_BASEADDR, RPI_ADDR, 0x81, 1, &tmp, 1);

    PRINTF("done\r\n");

    //rpi_display_set_bright
    //write_reg(RPI_ADDR, 0x86, brightness);
    BOARD_LPI2C_Send(RPI_I2C_BASEADDR, RPI_ADDR, 0x86, 1, &brightness, 1);
}
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    //BaseType_t stat;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_BootClockRUN();

    /*
     * Reset the displaymix, otherwise during debugging, the
     * debugger may not reset the display, then the behavior
     * is not right.
     */
    SRC_AssertSliceSoftwareReset(SRC, kSRC_DisplaySlice);

    BOARD_InitLpuartPins();
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
    BOARD_InitI2CPins();
#else
    BOARD_InitMipiPanelPins();
#endif
    //BOARD_MIPIPanelTouch_I2C_Init();
    BOARD_InitDebugConsole();
    // CAN pins init
    BOARD_InitPins_CAN();

//    stat = xTaskCreate(AppTask, "lvgl", configMINIMAL_STACK_SIZE + 800, NULL, tskIDLE_PRIORITY + 2, NULL);

/*    if (pdPASS != stat)
    {
        PRINTF("Failed to create lvgl task");
        while (1)
            ;
    }

    vTaskStartScheduler();

    for (;;)
    {
    }  should never get here */
    DEMO_SetupTick();

#if LV_USE_LOG
    lv_log_register_print_cb(print_cb);
#endif

    lv_port_pre_init();
    lv_init();
    lv_port_disp_init();
 //   lv_port_indev_init();

    // Set : LCD_LR, LCD_UD, LCD_RESET, LCD_STDBY
    //BOARD_InitTFTPanel_Support_Pins();

    /* Initialize and enable LED */
    //LED_INIT();  //---> Dinesh

    /********** PIT Module **********************/
    /* pitConfig.enableRunInDebug = false;  */
    PIT_GetDefaultConfig(&pitConfig);
    /* Init pit module */
    PIT_Init(DEMO_PIT_BASEADDR, &pitConfig);
    /* Set timer period for channel 0 */
    PIT_SetTimerPeriod(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, USEC_TO_COUNT(1000000U, PIT_SOURCE_CLOCK)); // 1 sec timer
    /* Enable timer interrupts for channel 0 */
    PIT_EnableInterrupts(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL, kPIT_TimerInterruptEnable);
    /* Set timer period for channel 1 */
    PIT_SetTimerPeriod(DEMO_PIT_BASEADDR, DEMO_PIT_GUI_CHANNEL, MSEC_TO_COUNT(1U, PIT_SOURCE_CLOCK));  // 1 m-sec timer
    /* Enable timer interrupts for channel 1 */
    PIT_EnableInterrupts(DEMO_PIT_BASEADDR, DEMO_PIT_GUI_CHANNEL, kPIT_TimerInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(PIT_IRQ_ID);
    /* Start channel 0 */
    PRINTF("\r\nStarting channel No.0 ...");
    PIT_StartTimer(DEMO_PIT_BASEADDR, DEMO_PIT_CHANNEL);
    /* Start channel 1 */
    PRINTF("\r\nStarting channel No.1 ...");
    PIT_StartTimer(DEMO_PIT_BASEADDR, DEMO_PIT_GUI_CHANNEL);

    // Initialize the RTC
    RTC_Init();
    draw_RTC_Init();

    // setup screen
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);
	lv_task_handler();  // Update the display


    // Display Logo
//	while(1)
	while(first_screen != 2)
	{
		__asm volatile ("nop");

	}

	//Set CAN Init & reception
    NVIC_SetPriority(CAN3_IRQn, 0);
    FLexCAN_3_Init();
    FLexCAN_3_RX_Msg();

    while(1)
    {
    	//Drawing demo
    	Set_drawing_demo();

    	//    roller1 = lv_roller_create(om, NULL);
    	//        //lv_obj_set_size(roller1,200,128);
    	//        lv_obj_add_style(roller1, LV_ROLLER_PART_BG,&bg_style);
    	//        lv_obj_add_style(roller1, LV_ROLLER_PART_SELECTED, &local_style);
    	//        lv_roller_set_options(roller1,
    	//                           LV_SYMBOL_AUDIO " 1\n"
    	//                           LV_SYMBOL_VIDEO " 2\n"
    	//            LV_SYMBOL_LIST " 3\n"
    	//            LV_SYMBOL_GPS  " 4\n"
    	//            LV_SYMBOL_WIFI " 5\n"
    	//            LV_SYMBOL_BLUETOOTH " 6\n"
    	//            LV_SYMBOL_POWER " 7\n"
    	//            LV_SYMBOL_REFRESH " 8\n"
    	//            LV_SYMBOL_VOLUME_MAX " 9\n"
    	//            LV_SYMBOL_BELL" 10\n"
    	//            LV_SYMBOL_CHARGE" 11\n"
    	//                            "12",
    	//            LV_ROLLER_MODE_NORMAL);
    	//        lv_roller_set_options();

    	//https://forum.lvgl.io/t/how-to-set-roller-default/5285/6
    	//https://docs.lvgl.io/8.0/widgets/core/roller.html#example
    	//    roll_menu_animation();
    	//
    	//	while(1)
    	//	{
    	//		__asm volatile ("nop");
    	//	}

    	// do the scroll menu animation
    	scroll_menu_animation();

    	// start the rider animation
    	rider_animation();

    	// call function to display rider infor
    	display_rider_info();

    }
	//start the Demo of cluster
//#ifdef HIDE_ODO
//    Screen_ODO_hide();
//#endif
	Set_cluster_demo();

	while(1)
	{
		__asm volatile ("nop");
	}
}

/*!
 * @brief Malloc failed hook.
 */
void vApplicationMallocFailedHook(void)
{
    for (;;)
        ;
}

/*!
 * @brief FreeRTOS tick hook.
 */
void vApplicationTickHook(void)
{
    if (s_lvgl_initialized)
    {
        lv_tick_inc(1);
    }
}

/*!
 * @brief Stack overflow hook.
 */
/*void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)pcTaskName;
    (void)xTask;

    for (;;)
        ;
}*/

static void DEMO_SetupTick(void)
{
    if (0 != SysTick_Config(SystemCoreClock / (LVGL_TICK_MS * 1000U)))
    {
        PRINTF("Tick initialization failed\r\n");
        while (1)
            ;
    }
}
#if 1
void SysTick_Handler(void)
{
    s_tick++;
    lv_tick_inc(LVGL_TICK_MS);

    if ((s_tick % LVGL_TASK_PERIOD_TICK) == 0U)
    {
        s_lvglTaskPending = true;
    }
}
#endif
