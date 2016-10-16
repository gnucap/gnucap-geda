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
#include <e_node.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <boost/pending/disjoint_sets.hpp>
//#include <m_part.h>

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

#undef PORTS_PER_SUBCKT
#define PORTS_PER_SUBCKT 100
/*--------------------------------------------------------------------------*/
class MODEL_GEDA_SUBCKT;
/*--------------------------------------------------------------------------*/
class DEV_GEDA_SUBCKT : public BASE_SUBCKT {
  friend class MODEL_GEDA_SUBCKT;
private:
  explicit	DEV_GEDA_SUBCKT(const DEV_GEDA_SUBCKT&);
public:
  typedef boost::disjoint_sets_with_storage<> PARTITION;
public:
  explicit	DEV_GEDA_SUBCKT();
		~DEV_GEDA_SUBCKT();
private: // from DEV_SUBCKT
  char		id_letter()const	{return 'X';}
  bool		print_type_in_spice()const {return true;}
  std::string   value_name()const	{return "#";}
  // std::string   dev_type()const
  uint_t	max_nodes()const	{return PORTS_PER_SUBCKT;}
#ifdef NDEBUG
  void map_nodes(){
    untested();
    trace3("map_nodes", long_label(), net_nodes(), matrix_nodes());
    BASE_SUBCKT::map_nodes();
  }
#endif
  uint_t	min_nodes()const	{return 0;}
  uint_t	matrix_nodes()const	{return 0;}
public: //HACK
  uint_t	net_nodes()const	{return _net_nodes;}
private:
//  CARD*	clone_instance()const;
  void		precalc_first();
  bool		makes_own_scope()const  {itested(); return false;}

  void		expand();
private:
  void		precalc_last();
  double	tr_probe_num(const std::string&)const;
  int param_count_dont_print()const {return common()->COMMON_COMPONENT::param_count();}

  std::string port_name(uint_t i)const;
  CARD*	clone()const		{return new DEV_GEDA_SUBCKT(*this);}
public:
  void set_port_by_index(uint_t num, std::string& ext_name);
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
  PARTITION *_part;
  unsigned *_map;
  const MODEL_GEDA_SUBCKT* _parent;
  node_t _nodes[PORTS_PER_SUBCKT];
};
/*--------------------------------------------------------------------------*/
class INTERFACE MODEL_GEDA_SUBCKT : public DEV_GEDA_SUBCKT {
private:
  explicit	MODEL_GEDA_SUBCKT(const MODEL_GEDA_SUBCKT&p);
public:
  explicit	MODEL_GEDA_SUBCKT();
		~MODEL_GEDA_SUBCKT();
public: // override virtual
  char id_letter()const	{untested();return '\0';}
  CARD* clone_instance()const;
  CARD* clone()const		{return new MODEL_GEDA_SUBCKT(*this);}
  void set_port_by_index(uint_t num, std::string& ext_name);
  bool		makes_own_scope()const  {return true;}
  CARD_LIST*	   scope()		{assert(subckt()); return subckt();}
  const CARD_LIST* scope()const		{assert(subckt()); return subckt();}
private: // no-ops for prototype
  bool is_device()const	{return false;}
  void precalc_first(){}
  void expand(){}
  void precalc_last(){}
  void map_nodes(){
    trace1("map_nodes base", long_label());
  }
  void tr_begin(){}
  void tr_load(){}
  TIME_PAIR tr_review(){return TIME_PAIR(NEVER, NEVER);}
  void tr_accept(){}
  void tr_advance(){}
  void tr_restore(){untested();}
  void tr_regress(){}
  void dc_advance(){}
  void ac_begin(){untested();}
  void do_ac(){untested();}
  void ac_load(){untested();}
  bool do_tr(){return true;}
  bool tr_needs_eval()const{untested(); return false;}
  void tr_queue_eval(){}
  std::string port_name(uint_t)const {return "";}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
