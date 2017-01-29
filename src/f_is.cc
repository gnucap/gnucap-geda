/*                             -*- C++ -*-
 * Copyright (C) 2016 Felix Salfelder
 * Author: same
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

// simulation type functions

#include <u_parameter.h>
#include <u_function.h>
#include <u_sim_data.h>
#include <globals.h>

// -uf compat hack
#ifndef _U_FUNC
typedef std::string fun_t;
#define to_fun_t to_string
#endif

namespace{

class is : public FUNCTION { //
public:
	is(SIM_MODE x) : _mode(x) {}
public:
	fun_t eval(CS& Cmd, const CARD_LIST* Scope)const
	{
		double ret = _sim->sim_mode()==_mode;
		return to_fun_t(ret);
	}
private:
	SIM_MODE _mode;
};

static is is_ac(s_AC);
DISPATCHER<FUNCTION>::INSTALL f_is_ac(&function_dispatcher, "is_ac", &is_ac);

static is is_dc(s_DC);
DISPATCHER<FUNCTION>::INSTALL f_is_dc(&function_dispatcher, "is_dc", &is_dc);

static is is_op(s_OP);
DISPATCHER<FUNCTION>::INSTALL f_is_op(&function_dispatcher, "is_op", &is_op);

}
