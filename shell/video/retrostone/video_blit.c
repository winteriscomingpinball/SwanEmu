/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL/SDL.h>
#include <sys/time.h>
#include <sys/types.h>
#include "mednafen.h"
#include "mempatcher.h"
#include "wswan/gfx.h"
#include "wswan/wswan-memory.h"
#include "wswan/sound.h"
#include "wswan/interrupt.h"
#include "wswan/v30mz.h"
#include "wswan/rtc.h"
#include "wswan/gfx.h"
#include "wswan/eeprom.h"

#include "video_blit.h"
#include "scaler.h"
#include "config.h"

SDL_Surface *sdl_screen, *backbuffer, *wswan_vs, *wswan_vs_rot;

uint32_t width_of_surface;
uint32_t* Draw_to_Virtual_Screen;

void Init_Video()
{
	SDL_Init( SDL_INIT_VIDEO );
	
	SDL_ShowCursor(0);
	
	sdl_screen = SDL_SetVideoMode(0, 0, 16, SDL_HWSURFACE);
	
	backbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, HOST_WIDTH_RESOLUTION, HOST_HEIGHT_RESOLUTION, 16, 0,0,0,0);
	
	wswan_vs = SDL_CreateRGBSurface(SDL_SWSURFACE, INTERNAL_WSWAN_WIDTH, INTERNAL_WSWAN_HEIGHT, 16, 0,0,0,0);
	
	wswan_vs_rot = SDL_CreateRGBSurface(SDL_SWSURFACE, INTERNAL_WSWAN_HEIGHT, INTERNAL_WSWAN_WIDTH, 16, 0,0,0,0);

	Set_Video_InGame();
}

void Set_Video_Menu()
{
	if (sdl_screen->w != HOST_WIDTH_RESOLUTION)
	{
		memcpy(wswan_vs->pixels, sdl_screen->pixels, (INTERNAL_WSWAN_WIDTH * INTERNAL_WSWAN_HEIGHT)*2);
		sdl_screen = SDL_SetVideoMode(HOST_WIDTH_RESOLUTION, HOST_HEIGHT_RESOLUTION, 16, SDL_HWSURFACE);
	}
}

void Set_Video_InGame()
{
	if (sdl_screen->w != HOST_WIDTH_RESOLUTION) sdl_screen = SDL_SetVideoMode(HOST_WIDTH_RESOLUTION, HOST_HEIGHT_RESOLUTION, 16, SDL_HWSURFACE);
	Draw_to_Virtual_Screen = wswan_vs->pixels;
	width_of_surface = INTERNAL_WSWAN_WIDTH;
}

void Close_Video()
{
	if (sdl_screen) SDL_FreeSurface(sdl_screen);
	if (backbuffer) SDL_FreeSurface(backbuffer);
	if (wswan_vs) SDL_FreeSurface(wswan_vs);
	if (wswan_vs_rot) SDL_FreeSurface(wswan_vs_rot);
	SDL_Quit();
}

void Update_Video_Menu()
{
	SDL_Flip(sdl_screen);
}

void Set_Video_Menu_Quit()
{
}

static void rotate_90_ccw(uint16_t* restrict dst, uint16_t* restrict src)
{
    int32_t h = 224, w = 144;
    src += w * h - 1;
    for (int32_t col = w - 1; col >= 0; --col)
    {
        uint16_t *outcol = dst + col;
        for(int32_t row = 0; row < h; ++row, outcol += w)
        {
            *outcol = *src--;
		}
    }
}

void Update_Video_Ingame()
{
	uint32_t internal_width, internal_height, keep_aspect_width, keep_aspect_height;
	uint16_t* restrict source_graph;
	
	if ((Wswan_IsVertical() == 1 && option.orientation_settings != 2) || option.orientation_settings == 1)
	{
		rotate_90_ccw((uint16_t* restrict)wswan_vs_rot->pixels, (uint16_t* restrict)wswan_vs->pixels);
		internal_width = INTERNAL_WSWAN_HEIGHT;
		internal_height = INTERNAL_WSWAN_WIDTH;
		source_graph = (uint16_t* restrict)wswan_vs_rot->pixels;
		keep_aspect_width =  ((HOST_HEIGHT_RESOLUTION / INTERNAL_WSWAN_HEIGHT) * INTERNAL_WSWAN_WIDTH) / 2;
		if (keep_aspect_width > HOST_WIDTH_RESOLUTION) keep_aspect_width -= HOST_WIDTH_RESOLUTION/2;
		keep_aspect_height = HOST_HEIGHT_RESOLUTION;
	}
	else
	{
		internal_width = INTERNAL_WSWAN_WIDTH;
		internal_height = INTERNAL_WSWAN_HEIGHT;
		source_graph = (uint16_t* restrict)wswan_vs->pixels;
		keep_aspect_width = HOST_WIDTH_RESOLUTION;
		keep_aspect_height = ((HOST_WIDTH_RESOLUTION / INTERNAL_WSWAN_WIDTH) * INTERNAL_WSWAN_HEIGHT);
		if (keep_aspect_height > HOST_HEIGHT_RESOLUTION) keep_aspect_height -= HOST_HEIGHT_RESOLUTION/4;
	}
	
	if (SDL_LockSurface(sdl_screen) == 0)
	{
		switch(option.fullscreen) 
		{
			// Fullscreen
			case 0:
				bitmap_scale(0, 0, internal_width, internal_height, HOST_WIDTH_RESOLUTION, HOST_HEIGHT_RESOLUTION, internal_width, 0, (uint16_t* restrict)source_graph, (uint16_t* restrict)sdl_screen->pixels);
			break;
			case 1:
				bitmap_scale(0,0,internal_width,internal_height,keep_aspect_width,keep_aspect_height,internal_width, HOST_WIDTH_RESOLUTION - keep_aspect_width,(uint16_t* restrict)source_graph,(uint16_t* restrict)sdl_screen->pixels+(HOST_WIDTH_RESOLUTION-keep_aspect_width)/2+(HOST_HEIGHT_RESOLUTION-keep_aspect_height)/2*HOST_WIDTH_RESOLUTION);
			break;
			// Hqx
			case 2:
			break;
			// Native
			case 3:
				bitmap_scale(0,0,internal_width,internal_height,internal_width*2,internal_height*2,internal_width, HOST_WIDTH_RESOLUTION - (internal_width*2),(uint16_t* restrict)source_graph,(uint16_t* restrict)sdl_screen->pixels+(HOST_WIDTH_RESOLUTION-(internal_width*2))/2+(HOST_HEIGHT_RESOLUTION-(internal_height*2))/2*HOST_WIDTH_RESOLUTION);
			break;
		}
		SDL_UnlockSurface(sdl_screen);	
	}
	SDL_Flip(sdl_screen);
}
