/*                     -*- C++ -*-
 * Copyright (C) 2015 Albert Davis
 * Author: Felix Salfelder
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
 *------------------------------------------------------------------
 * data structures for subcircuits from geda schematics
 */
#include <d_subckt.h>

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
class DEV_GEDA_SUBCKT;
/*--------------------------------------------------------------------------*/
class INTERFACE MODEL_GEDA_SUBCKT : public MODEL_SUBCKT {
private:
  explicit	MODEL_GEDA_SUBCKT(const MODEL_GEDA_SUBCKT&p);
public:
  explicit	MODEL_GEDA_SUBCKT();
		~MODEL_GEDA_SUBCKT() {}
public: // override virtual
  char id_letter()const	{untested();return '\0';}
  CARD* clone_instance()const;
  CARD* clone()const		{return new MODEL_GEDA_SUBCKT(*this);}
  //void		map_nodes()		{}
private:
  void set_port_by_index(uint_t num, std::string& ext_name);
};
/*--------------------------------------------------------------------------*/
class DEV_GEDA_SUBCKT : public DEV_SUBCKT {
  friend class MODEL_GEDA_SUBCKT;
private:
  explicit	DEV_GEDA_SUBCKT(const DEV_GEDA_SUBCKT&);
public:
  explicit	DEV_GEDA_SUBCKT() : _map(NULL) {}
		~DEV_GEDA_SUBCKT() {}
private: // override virtual
  CARD*	clone()const		{return new DEV_GEDA_SUBCKT(*this);}
  void set_port_by_index(uint_t num, std::string& ext_name);
  void expand();
private: // node stuff
  void map_subckt_nodes(const CARD* model);
public:
  void collapse_nodes(const NODE* a, const NODE* b);
  void orbit_number(unsigned* map, unsigned len, unsigned* port);
//  void map_net(unsigned rep, unsigned to, unsigned* map);
  void apply_map(unsigned* map);

#if 0
  std::string port_name(int i)const {itested();
    if (_parent) {itested();
      return _parent->port_value(i);
    }else{itested();
      return "";
    }
  }
#endif
private:
  void set_parent(const MODEL_GEDA_SUBCKT* p);
  unsigned *_map;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
