/* (c) 2012 Felix Salfelder
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * base class for places
 */
#ifndef D_PLACE_H__
#define D_PLACE_H__

#include <md.h>

#ifndef HAVE_UINT_T
#define HAVE_UINT_T
typedef int uint_t;
#endif

namespace place{

class DEV_PLACE : public COMPONENT {
	public:
		explicit DEV_PLACE() :
		    COMPONENT(),
		    _x(0),
		    _y(0)
  		{ _n=_nodes; }
		explicit DEV_PLACE(const DEV_PLACE& p) :
		    COMPONENT(p),
		    _x(p._x),
		    _y(p._y)
		{
			_n = _nodes;
		}
		~DEV_PLACE(){}
	private:
		bool param_is_printable(int)const;
	protected:
		int _x;
		int _y;
	private:
		std::string param_name(int) const;
		std::string param_name(int, int) const;
		void set_param_by_name(std::string, std::string);
		void set_param_by_index(int, std::string&, int);
		std::string param_value(int) const;
	private:
		char id_letter()const {return 'P';}
		std::string value_name()const {return "";}
		std::string dev_type()const {return "place";}
		uint_t max_nodes() const {return 1;}
		uint_t min_nodes()const {return 1;}
		uint_t matrix_nodes()const {return 1;}
		uint_t net_nodes()const {return 1;}
		bool has_iv_probe()const {return true;}
		bool print_type_in_spice()const {return true;}
		bool is_device() const {return false;}
		CARD* clone()const {return new DEV_PLACE(*this);}
		int param_count()const {return 2;}
		std::string port_name(uint_t i)const{
			assert(i==0);
			return "port";
		}
	public:
		node_t _nodes[NODES_PER_BRANCH];
	public: // geda stuff
		int x()const { return _x; }
		int y()const { return _y; }
		int& x(){ return _x; }
		int& y(){ return _y; }
};

}
#endif
