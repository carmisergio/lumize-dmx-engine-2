
/*
 * Filename: lightstates.h
 * Description: interface for the LightStates struct
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 30/08/2022
 */

#pragma once

struct LightStates
{
   // Outward-facing states
   bool outward_state[512];
   int outward_brightness[512];

   // Fade states
   double fade_progress[512];
   double fade_delta[512];
   int fade_start[512];
   int fade_end[512];
   int fade_current[512];

   // Pushbutton dimming states
   bool pushbutton_fade[512];
   bool pushbutton_fade_up[512];
   double pushbutton_fade_current[512];
};