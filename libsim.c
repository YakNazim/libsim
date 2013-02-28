/**
 * @file
 * @author  Nathan Bergey <nathan.bergey@gmail.com>
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @brief Main program
 *
 * @section DESCRIPTION
 *
 * Calls to the rest of the program are handled through here.
 */

#include <stdio.h>
#include <stdlib.h>
#include "libsim_types.h"
#include "physics/physics.h"
#include "physics/gravity.h"
#include "libsim.h"

/// Number of ODE's (3 position, 3 velocity, mass)
#define n 7

// Local functions
static void deriv(double *y ,double *dydx, double t);
static void integrate(state *yp, double *xp, double x1, double x2, int *steps);

// Globals
physics_model_strategy physics_model;
void Init_Model(void)
{
	physics_model.gravity_model = gravity_sphere;
}

state Integrate_Rocket(rocket r, state initial_conditions, state_history *history)
{
	/// Set the current state from the incoming initial state
	state current_state = initial_conditions;

	/// Memory for integrator
	state *yp;
	double *xp;
	yp = malloc(sizeof(state) * MAXSTEPS);
	xp = malloc(sizeof(double) * MAXSTEPS);
	int steps_taken = 0;

	integrate(yp, xp, 0, 1, &steps_taken);

	/// Return the final state after integrating
	return current_state;
}

static void integrate(state *yp, double *xp, double x1, double x2, int *steps)
{
	int stepnum = 0; // Track number of steps the integrator has run

	double x = x1; // Current time, begin at x1

	// Integrator memory, each position in the array is a DOF of the system
	double y[n];     // array of integrator outputs, y = integral(y' dx)
	double dydx[n];  // array of RHS, dy/dx

	while (stepnum <= MAXSTEPS)
	{
		// First RHS call
		deriv(y, dydx, x);
		stepnum++;
	}

	(*steps) = stepnum;
	return;
}

void deriv(double *y ,double *dydx, double t)
{
	state current_state;

	// To solve for acceleration build a state struct to send to physics engine
	current_state.x.v.i = y[0];
	current_state.v.v.i = y[1];
	current_state.x.v.j = y[2];
	current_state.v.v.j = y[3];
	current_state.x.v.k = y[4];
	current_state.v.v.k = y[5];
	current_state.a.v.i = dydx[1];
	current_state.a.v.j = dydx[3];
	current_state.a.v.k = dydx[5];
	current_state.m = y[6];

	// Do Physics to current state:
	state_change deriv_state = physics(current_state, t, physics_model);

	// Build RK vectors from state change
	// Velocity is single integration of acceleration
	dydx[0] = y[1];
	dydx[2] = y[3];
	dydx[4] = y[5];
	// Acceleration is from physics model
	dydx[1] = deriv_state.acc.v.i;
	dydx[3] = deriv_state.acc.v.j;
	dydx[5] = deriv_state.acc.v.k;
	dydx[6] = deriv_state.m_dot;
}
