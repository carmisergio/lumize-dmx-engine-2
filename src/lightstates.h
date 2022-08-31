
/*
 * Filename: lightstates.h
 * Description: interface for the LightStates struct
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 30/08/2022
 */

#pragma once

struct LightStates
{
   int test[512];
   int test2[512];

   // Outward-facing states
   bool outward_state[512];
   int outward_brightness[512];
};