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

#include "d_gedasckt.h"
#include <globals.h>
#include <u_nodemap.h>
#include "io_trace.h"
/*--------------------------------------------------------------------------*/
static COMMON_SUBCKT Default_SUBCKT(CC_STATIC);
static DEV_GEDA_SUBCKT p1;
static MODEL_GEDA_SUBCKT p2;
static DISPATCHER<CARD>::INSTALL
//  d1(&device_dispatcher, "X|dev_subckt", &p1),
  d2(&device_dispatcher, "symbol|subckt", &p2);
/*--------------------------------------------------------------------------*/
MODEL_GEDA_SUBCKT::MODEL_GEDA_SUBCKT() : DEV_GEDA_SUBCKT()
{
	new_subckt();
}
/*--------------------------------------------------------------------------*/
MODEL_GEDA_SUBCKT::MODEL_GEDA_SUBCKT(MODEL_GEDA_SUBCKT const& p) :
	DEV_GEDA_SUBCKT(p)
{
	new_subckt();
}
/*--------------------------------------------------------------------------*/
CARD* MODEL_GEDA_SUBCKT::clone_instance()const
{ untested();
	DEV_GEDA_SUBCKT* new_instance = dynamic_cast<DEV_GEDA_SUBCKT*>(p1.clone());
	new_instance->set_parent(this);
	return new_instance;
}
/*--------------------------------------------------------------------------*/
void MODEL_GEDA_SUBCKT::set_port_by_index(uint_t num, std::string& ext_name)
{
  if (num < max_nodes()) {
    _n[num].new_node(ext_name, this);
	 trace4("MODEL_GEDA_SUBCKT::set_port_by_index", long_label(), num, ext_name, _n[num].t_());


    if (num+1 > _net_nodes) {
      // make the list bigger
      _net_nodes = num+1;
    }else{
      // it's already big enough, probably assigning out of order
    }
  }else{
    throw Exception_Too_Many(num+1, max_nodes(), 0/*offset*/);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_GEDA_SUBCKT::DEV_GEDA_SUBCKT() :
	BASE_SUBCKT(), _map(NULL), _parent(NULL)
{
	attach_common(&Default_SUBCKT);
	_n = _nodes;
}
/*--------------------------------------------------------------------------*/
DEV_GEDA_SUBCKT::DEV_GEDA_SUBCKT(DEV_GEDA_SUBCKT const& p) :
	BASE_SUBCKT(p), _map(NULL), _parent(p._parent)
{
  for (uint_t ii = 0; ii < max_nodes(); ++ii) {
    _nodes[ii] = p._nodes[ii];
  }
  _n = _nodes;
  assert(!subckt());
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::set_parent(const MODEL_GEDA_SUBCKT* p)
{ untested();
	_parent = p;
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::apply_map(unsigned* map)
{ untested();
	CARD_LIST* cl = subckt();
	assert(cl);

	unsigned num_nodes_in_subckt = _parent->subckt()->nodes()->how_many();
	for(unsigned i=0; i<=num_nodes_in_subckt; ++i){
		trace3("preapply", long_label(), i, _map[i]);
	}

	for (CARD_LIST::iterator ci = cl->begin(); ci != cl->end(); ++ci) {
		if ((**ci).is_device()) {
			trace1("apply", (*ci)->long_label());
			for (uint_t ii=0;  ii<(**ci).net_nodes(); ++ii) {
				trace2("apply", ii, (**ci).n_(ii).e_());
				(**ci).n_(ii).map_subckt_node((uint_t*)map, this); //  _ttt = map[e_()];
			}
		}else{
//			assert(dynamic_cast<MODEL_CARD*>(*ci));
		}
	}
}
/*--------------------------------------------------------------------------*/
// map is a permutation
// a permutation has orbits, hence induces a partition
// the minimal element of a partition is the partition number
// assign to each element the number of its partition
// e.g. turn 0 1 4 3 2 5 into
//           0 1 2 3 2 5
void DEV_GEDA_SUBCKT::orbit_number(unsigned* map, unsigned len, unsigned* port)
{
	for(unsigned i=0; i<len; ++i) {
		unsigned j=i;
		while(map[j]>i){
			unsigned tmp = map[j];
			map[j] = i;
			j = tmp;

			if (port[j] == (unsigned)INVALID_NODE){
			}else if(port[i] == (unsigned)INVALID_NODE){
				port[i] = port[j];
			}else{
				delete _map; // FIXME: use proper container.
				_map = NULL;
				throw Exception(long_label() + ": cannot connect ports \"" +
							_parent->n_(i-1).n_()->short_label() + "\" and \"" +
							_parent->n_(j-1).n_()->short_label() + "\"");
			}

		}
	}
}
/*--------------------------------------------------------------------------*/
std::string DEV_GEDA_SUBCKT::port_name(uint_t i)const {itested();
	if (_parent) {itested();
		if (i<_parent->net_nodes()){ untested();
			return _parent->port_value(i);
		}else{ untested();
			return "";
		}
	}else{itested();
		return "";
	}
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::map_subckt_nodes(const CARD* model)
{ untested();
	assert(model);
	assert(model->subckt());
	assert(model->subckt()->nodes());
	unsigned num_nodes_in_subckt = _parent->subckt()->nodes()->how_many();

	unsigned *port=new unsigned[num_nodes_in_subckt+1];
	std::fill(port, port+num_nodes_in_subckt+1, INVALID_NODE);

	// _map maps nodes to nets.
	// some of them are external
	// namely model->n_(i).t_() for i < net_nodes()
	for (unsigned i=1; i <= (unsigned)model->net_nodes(); ++i) {
		trace5("model port", long_label(), i, model->n_(i-1).t_(), n_(i-1).t_(), n_(i-1).e_());
		trace2("model port", hp(this), hp(model));
		assert(model->n_(i-1).e_() == model->n_(i-1).t_());
		unsigned usernumber = model->n_(i-1).t_();
		port[usernumber] = n_(i-1).t_();
	}
	for (unsigned i=1; i <= num_nodes_in_subckt; ++i) {
		trace3("collapse map", i, _map[i], port[i]);
	}
	assert(_map);
	orbit_number(_map, num_nodes_in_subckt, port);
	for(unsigned i=0; i<=num_nodes_in_subckt; ++i){
		trace4("orbit", long_label(), i, _map[i], port[i]);
	}

	{
		assert(_map[0]==0);
		// self test: verify that port node numbering is correct
		for (unsigned port = 0; port < (unsigned)model->net_nodes(); ++port) {
			assert(model->n_(port).e_() <= (uint_t)num_nodes_in_subckt);
			//assert(model->n_(port).e_() == port+1);
			trace3("ports", port, model->n_(port).e_(), n_(port).t_());
		}
		{
			unsigned seek=model->net_nodes();
			for (unsigned i=1; i <= num_nodes_in_subckt; ++i) {
				if(port[_map[i]]!=(unsigned)INVALID_NODE){
					trace2("port", i, _map[i]);
					_map[i] = port[_map[i]];
				}else if(_map[i]<=seek){ untested();
					trace3("internal, exists", i, _map[i], seek);
					_map[i] = _map[_map[i]];
				}else{ untested();
					trace3("internal, new", i, _map[i], seek);
					seek = _map[i];
					_map[i] = CKT_BASE::_sim->newnode_subckt();
				}
			}
			delete[] port;
		}
	}
	// "map" now contains a translation list,
	// from subckt local numbers to matrix index numbers

	// scan the list, map the nodes
	apply_map(_map);
}
/*--------------------------------------------------------------------------*/
// a net (=set of nodes) is a cycle in the permutation defined by _map.
void DEV_GEDA_SUBCKT::collapse_nodes(const NODE* a, const NODE* b)
{
	unsigned num_nodes_in_subckt = _parent->subckt()->nodes()->how_many();
	trace4("DEV_GEDA_SUBCKT::collapse_nodes", long_label(), a->user_number(), b->user_number(), num_nodes_in_subckt);
	trace2("DEV_GEDA_SUBCKT::collapse_nodes", a->short_label(), b->short_label());
	unsigned i=a->user_number();
	assert(i<num_nodes_in_subckt+1);
	unsigned j=b->user_number();
	assert(j<num_nodes_in_subckt+2);

	if(_map){
		std::swap(_map[i], _map[j]);
	}else{
	}
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::precalc_first()
{
  BASE_SUBCKT::precalc_first();

  if (subckt()) { untested();
    COMMON_SUBCKT* c = prechecked_cast<COMMON_SUBCKT*>(mutable_common());
    assert(c);
    subckt()->attach_params(&(c->_params), scope());
    subckt()->precalc_first();
  }else{ untested();
  }
  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::expand()
{
	trace1("DEV_GEDA_SUBCKT::expand", long_label());
	BASE_SUBCKT::expand();
	COMMON_SUBCKT* c = prechecked_cast<COMMON_SUBCKT*>(mutable_common());
	assert(c);
	if (!_parent) { untested();
		const CARD* model = find_looking_out(c->modelname());
		if(!dynamic_cast<const BASE_SUBCKT*>(model)) {
			throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt");
		}else{ untested();
			_parent = prechecked_cast<const MODEL_GEDA_SUBCKT*>(model);
		}
	}else{
		assert(find_looking_out(c->modelname()) == _parent);
	}

	assert(_parent->subckt());
	assert(_parent->subckt()->params());
	PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
	assert(pl);
	c->_params.set_try_again(pl);

	// renew_subckt(newparent, &(c->_params)/*, map?*/); // allocates internal nodes globally.


	new_subckt();
	subckt()->attach_params(&(c->_params), scope());
	subckt()->shallow_copy(_parent->subckt());
	subckt()->set_owner(this);

	unsigned num_nodes_in_subckt = _parent->subckt()->nodes()->how_many() + 1;
	assert(!_map);
	_map = new unsigned[num_nodes_in_subckt];
	for(unsigned i=0; i<num_nodes_in_subckt; ++i){
		_map[i] = i;
	}
	subckt()->precalc_first(); // collapses nodes (HACK).
	// maybe
	// for i in subckt{
	//   if net
	//     some_callback
	// }

	map_subckt_nodes(_parent);
	subckt()->expand();

	delete[] _map;
	_map = NULL;
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::precalc_last()
{
  BASE_SUBCKT::precalc_last();

  COMMON_SUBCKT* c = prechecked_cast<COMMON_SUBCKT*>(mutable_common());
  assert(c);
  subckt()->attach_params(&(c->_params), scope());
  subckt()->precalc_last();

  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::set_port_by_index(uint_t num, std::string& ext_name)
{ untested();
  trace3("DEV_GEDA_SUBCKT::set_port_by_index", long_label(), num, ext_name);
  COMPONENT::set_port_by_index(num, ext_name);
  trace2("DEV_GEDA_SUBCKT::set_port_by_index", hp(this), _n[num].t_());
}
/*--------------------------------------------------------------------------*/
double DEV_GEDA_SUBCKT::tr_probe_num(const std::string& x)const
{itested();
  if (Umatch(x, "p ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"P");
    }
    return power;
  }else if (Umatch(x, "pd ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PD");
    }
    return power;
  }else if (Umatch(x, "ps ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PS");
    }
    return power;
  }else{itested();
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
/*--------------------------------------------------------------------------*/
