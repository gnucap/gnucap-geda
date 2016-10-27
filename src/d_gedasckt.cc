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

// workaround bug in unstable gnucap
#define USE(x) (void)x

#include "d_gedasckt.h"
#include <globals.h>
#include <u_nodemap.h>
#include "io_trace.h"
#include <boost/iterator/counting_iterator.hpp>

/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
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
	DEV_GEDA_SUBCKT(p), _defconn(p._defconn)
{
	new_subckt();
}
/*--------------------------------------------------------------------------*/
MODEL_GEDA_SUBCKT::~MODEL_GEDA_SUBCKT()
{
	trace3("destroying model", long_label(), (this), (common()));
}
/*--------------------------------------------------------------------------*/
CARD* MODEL_GEDA_SUBCKT::clone_instance()const
{
	DEV_GEDA_SUBCKT* new_instance = dynamic_cast<DEV_GEDA_SUBCKT*>(p1.clone());
	assert(new_instance);
	trace1("clone instance", _defconn);
	new_instance->set_parent(this);
	return new_instance;
}
/*--------------------------------------------------------------------------*/
void MODEL_GEDA_SUBCKT::set_port_by_index(uint_t num, std::string& ext_name,
		std::string def)
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
  }else{ untested();
    throw Exception_Too_Many(num+1, max_nodes(), 0/*offset*/);
  }
  if(_defaults.size()<=(unsigned)num){
	  _defaults.resize(num+1);
  }else{
  }
  _defaults[num] = def;
}
/*--------------------------------------------------------------------------*/
std::string MODEL_GEDA_SUBCKT::port_default(uint_t i)const
{
	if((unsigned)i<_defaults.size()){
		return _defaults[i];
	}else{
		return "";
	}
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_GEDA_SUBCKT::DEV_GEDA_SUBCKT() :
	BASE_SUBCKT(),
	_part(NULL),
	_map(NULL),
	_parent(NULL),
	_num_auto_nodes(0)
{
	attach_common(&Default_SUBCKT);
	detach_common();
	attach_common(&Default_SUBCKT);
	_n = _nodes;
	trace2("new", long_label(), (this));
}
/*--------------------------------------------------------------------------*/
DEV_GEDA_SUBCKT::DEV_GEDA_SUBCKT(DEV_GEDA_SUBCKT const& p) :
	BASE_SUBCKT(p),
	_part(NULL),
	_map(NULL),
	_parent(p._parent),
	_num_auto_nodes(0)
{
  for (uint_t ii = 0; ii < max_nodes(); ++ii) {
    _nodes[ii] = p._nodes[ii];
  }
  _n = _nodes;
  assert(!subckt());
  trace2("copy", long_label(), (this));
}
/*--------------------------------------------------------------------------*/
DEV_GEDA_SUBCKT::~DEV_GEDA_SUBCKT()
{
	trace3("destroying", long_label(), (this), (common()));
}
/*--------------------------------------------------------------------------*/
// map a node, so we can connect to it
void DEV_GEDA_SUBCKT::trysomehing(std::string x)
{

  NODE_MAP* Map = subckt()->nodes();
  NODE_MAP* ModelMap = _parent->subckt()->nodes();
  trace4("trying something", long_label(), x, Map->how_many(), ModelMap->how_many());
  assert(Map);
  if( (*Map)[x] ){
	  trace0("it's there");
	  // we have been here. probably nothing left to do.
  //}else if( NODE* N=(*_parent->scope()->nodes())[x] ){ untested();
  }else if( NODE* N=(*ModelMap)[x] ){
	  error(bDEBUG, "Preparing existing node %s in %s\n",
			  x.c_str(), long_label().c_str());
	  trace2("founda node in model", N->long_label(), N->user_number());
	  // apply the the user number, as in the model.
	  NODE* XX = Map->new_node(x);
	  XX->set_user_number(N->user_number());
  }else{
	  error(bDEBUG, "Creating new node %s in %s\n",
			  x.c_str(), long_label().c_str());
	  // the model does not know that node either. invent it.
	  NODE* XX = Map->new_node(x);
	  XX->set_user_number(ModelMap->how_many() + Map->how_many());
	  ++_num_auto_nodes;
  }
  trace3("tried something", long_label(), x, Map->how_many());
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::set_parent(const MODEL_GEDA_SUBCKT* p)
{
	COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
	PARAMETER<double> pdefcon=c->_params.deep_lookup("default_connect");
	trace3("set_parent defconn", long_label(), p->defconn(), pdefcon);
	for(unsigned i=0; i<(unsigned)param_count(); ++i){
		trace2("DEV_GEDA_SUBCKT::sp", param_name(i), param_value(i));
	}

	_parent = p;
	if(p->defconn()!=""){
		// hmm
		set_param_by_name("default_connect", p->defconn());
	}else{
		// hmm
		set_param_by_name("default_connect", p->defconn());
	}
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::apply_map(unsigned* map)
{
	trace2("apply_map", long_label(), is_device());
	CARD_LIST* cl = subckt();
	assert(cl);

	for (CARD_LIST::iterator ci = cl->begin(); ci != cl->end(); ++ci) {
		if ((**ci).is_device()) {
			for (uint_t ii=0;  ii<(**ci).net_nodes(); ++ii) {
				trace3("apply", (*ci)->long_label(), ii, (**ci).net_nodes()); //  (**ci).n_(ii).e_());
				trace3("apply", ii, _map[(**ci).n_(ii).e_()], (**ci).n_(ii).e_());
				(**ci).n_(ii).map_subckt_node((uint_t*)map, this); //  _ttt = map[e_()];
			}
		}else{
//			assert(dynamic_cast<MODEL_CARD*>(*ci));
		}
	}
	trace1("apply_map done", long_label());
}
/*--------------------------------------------------------------------------*/
std::string DEV_GEDA_SUBCKT::port_default(uint_t i)const
{
	if (_parent) {
		if (i<_parent->net_nodes()){
			return _parent->port_default(i);
		}else{ untested();
			return "";
		}
	}else{untested();
		return "";
	}
}
/*--------------------------------------------------------------------------*/
std::string DEV_GEDA_SUBCKT::port_name(uint_t i)const {
	if (_parent) {
		if (i<_parent->net_nodes()){
			return _parent->port_value(i);
		}else{
			return "";
		}
	}else{itested();
		return "";
	}
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::default_connect(const CARD* model)
{
	COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
	PARAMETER<double> pdefcon=c->_params.deep_lookup("default_connect");
	std::string defcon=pdefcon.string();
	trace2("defcon", long_label(), defcon);
	// _map maps nodes to nets.
	// some of them are external
	// namely model->n_(i).t_() for i < net_nodes()
	for (unsigned i=1; i <= (unsigned)model->net_nodes(); ++i) {
		trace4("model port", long_label(), i, model->n_(i-1).t_(), n_(i-1).t_());
		if(n_(i-1).t_()!=INVALID_NODE){
			// already connected.
			trace1("..", n_(i-1).e_());
		}else{ incomplete();
			error(bDEBUG, "%s: port #%d (%s) never connected, strategy %s\n",
					long_label().c_str(), i,
					port_name(i-1).c_str(), defcon.c_str());
			std::string dp=port_default(i-1);
			trace2("..", defcon, dp);

			if(defcon==""){ untested();
			}else if(defcon=="open"){ untested();
			}else if(defcon=="auto" || defcon=="promiscuous"){
				trace1("auto", dp);
				if(dp!=""){
					error(bDEBUG, "%s: port #%d (%s) autoconnect to %s\n",
							long_label().c_str(), i,
							port_name(i-1).c_str(),
							dp.c_str());
					// hmm only works if that node already exists...
					trace1("", port_value(i-1));
					incomplete();
					if(owner()==NULL){
						// this creates a new top level node and connects
						// to the dangling port.
						set_port_by_index(i-1, dp);
					}else if(DEV_GEDA_SUBCKT* o=dynamic_cast<DEV_GEDA_SUBCKT*>(owner())){ incomplete();
						// now what are the net names in o??
						o->trysomehing(dp);
						set_port_by_index(i-1, dp);
					}else{ incomplete();
						error(bWARNING, "i cannot help you\n");
					}
				}else{
					error(bPICKY, "%s: port #%d (%s) autoconnect but no name\n",
							long_label().c_str(), i,
							port_name(i-1).c_str());
				}
			}else{
				error(bPICKY, "%s: port #%d (%s) never connected\n",
						long_label().c_str(), i,
						port_name(i-1).c_str());
			}
		}
		trace2("model port", (this), (model));
//		unsigned usernumber = model->n_(i-1).t_();
//		port[usernumber] = n_(i-1).t_();
	}
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::map_subckt_nodes(const CARD* model)
{
	for(unsigned i=0; i<(unsigned)param_count(); ++i){
		trace2("DEV_GEDA_SUBCKT::msn", param_name(i), param_value(i));
	}
	assert(model);
	assert(model->subckt());
	assert(model->subckt()->nodes());
	unsigned num_nodes_in_model = _parent->subckt()->nodes()->how_many();
	typedef boost::counting_iterator<unsigned> elt_iter;
	error(bTRACE, "%s: map_subckt_nodes net: %d sckt: %d\n",
			long_label().c_str(),
			(unsigned)model->net_nodes(),
			num_nodes_in_model);

	unsigned *port=new unsigned[num_nodes_in_model+1];
	std::fill(port, port+num_nodes_in_model+1, INVALID_NODE);

	for (unsigned i=1; i <= num_nodes_in_model; ++i) {
		trace3("collapse map", i, _part->find_set(i), port[i]);
	}
	assert(_part);
	assert(_map);
	_part->compress_sets(elt_iter(1), elt_iter(num_nodes_in_model));
	_part->normalize_sets(elt_iter(1), elt_iter(num_nodes_in_model));

	for (unsigned i=1; i <= num_nodes_in_model; ++i) {
		trace3("normalized map", i, _part->find_set(i), port[i]);
	}

	unsigned connected_ports=0;
	for(unsigned i=1; i<=(unsigned)model->net_nodes(); ++i){
		unsigned usernumber = model->n_(i-1).t_();
		unsigned partno=_part->find_set(usernumber);
		assert(model->n_(i-1).e_() == model->n_(i-1).t_());
		trace3("port", i, usernumber, partno);

		if(port[partno]!=(unsigned)INVALID_NODE) {
			delete port;
			throw Exception(long_label() + ": cannot connect ports \"" +
					_parent->n_(partno-1).n_()->short_label() + "\" and \"" +
					_parent->n_(i-1).n_()->short_label() + "\"");
		}else if(n_(i-1).t_() != INVALID_NODE){
			++connected_ports;
			port[partno] = n_(i-1).t_();
			trace2("port conn", i, _parent->n_(i-1).n_()->short_label());
		}else{
		}
	}

	for(unsigned i=0; i<=num_nodes_in_model; ++i){
		trace4("portsconn", i, _part->find_set(i), port[i], port[_part->find_set(i)]);
	}

	{
		assert(_part->find_set(0)==0);
		// self test: verify that port node numbering is correct
		for (unsigned j = 0; j < (unsigned)model->net_nodes(); ++j) {
			assert(model->n_(j).e_() <= (uint_t)num_nodes_in_model);
			//assert(model->n_(j).e_() == j+1);
			trace4("ports", j, model->n_(j).short_label(),
					model->n_(j).e_(), n_(j).t_());
		}
		{
			// fill _map. local nodes in model to user_number
			// _map[0] = 0 (ground)
			unsigned seek = 0; // model->net_nodes();
			_num_cc = connected_ports;
			trace1("seek", seek);
			unsigned i;
			for (i=1; i <= num_nodes_in_model; ++i) {
				trace3("num", i, _part->find_set(i), port[_part->find_set(i)]);
				assert(_part->find_set(i)); // no gnd?
				if(port[_part->find_set(i)]!=(unsigned)INVALID_NODE){
					_map[i] = port[_part->find_set(i)];
					trace3("port", i, _part->find_set(i), _map[i]);
				}else if(_part->find_set(i)<=seek){
					trace3("internal, exists", i, _map[i], seek);
					_map[i] = _map[_part->find_set(i)];
				}else{
					assert(i==_map[i]);
					seek = _map[i];
					_map[i] = CKT_BASE::_sim->newnode_subckt();
					trace4("internal, new", i, _map[i], seek, _sim->_total_nodes);
					++_num_cc;
				}
			}
			for (; i <= _num_auto_nodes+num_nodes_in_model; ++i) { untested();
				_map[i] = CKT_BASE::_sim->newnode_subckt();
				trace4("auto, new", i, _map[i], seek, _sim->_total_nodes);
				++_num_cc;
			}
		}
	}
	trace2("ncc", _num_cc, _part->count_sets(elt_iter(1), elt_iter(num_nodes_in_model+1)));
	assert(_num_cc == _num_auto_nodes+_part->count_sets(elt_iter(1), elt_iter(num_nodes_in_model+1)));

	error(bTRACE, "%s: map_subckt_nodes connected components %d\n",
			long_label().c_str(), _num_cc);

	delete[] port;
	// "map" now contains a translation list,
	// from subckt local numbers to matrix index numbers
	
	for(unsigned i=0; i<=num_nodes_in_model; ++i){
		trace3("preapply", long_label(), i, _map[i]);
	}

	// scan the list, map the nodes
	apply_map(_map);
}
/*--------------------------------------------------------------------------*/
// a net (=set of nodes) is a cycle in the permutation defined by _map.
void DEV_GEDA_SUBCKT::collapse_nodes(const NODE* a, const NODE* b)
{
	unsigned i=a->user_number();
	unsigned j=b->user_number();
	unsigned num_nodes_in_subckt = _parent->subckt()->nodes()->how_many();
	error(bTRACE, "collapse %s: %s (%d), %s(%d)\n", long_label().c_str(),
			a->short_label().c_str(), i, b->short_label().c_str(), j);

	assert(i<num_nodes_in_subckt+1);
	assert(j<num_nodes_in_subckt+2);

	trace2("collapse", i, j);
	assert(_part);
	_part->union_set(i, j);
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::precalc_first()
{
  trace1("DEV_GEDA_SUBCKT::precalc_first", long_label());
  BASE_SUBCKT::precalc_first();

  if (subckt()) {
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    subckt()->attach_params(&(c->_params), scope());
    subckt()->precalc_first();
  }else{
  }
  assert(!is_constant()); /* because I have more work to do */


    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());

//---------------hmmm
	if (!_parent) { untested();
		const CARD* model = find_looking_out(c->modelname());
		if(!dynamic_cast<const BASE_SUBCKT*>(model)) { untested();
			throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt");
		}else{ untested();
			_parent = prechecked_cast<const MODEL_GEDA_SUBCKT*>(model);
		}
	}else{
		//  assert(find_looking_out(c->modelname()) == _parent);
	}

	trace1("DEV_GEDA_SUBCKT::pf", long_label());
	for(unsigned i=0; i<(unsigned)param_count(); ++i){
		trace2("DEV_GEDA_SUBCKT::pf", param_name(i), param_value(i));
	}

	default_connect(_parent);
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::expand()
{
	trace2("DEV_GEDA_SUBCKT::expand", long_label(), param_count());
	for(unsigned i=0; i<(unsigned)param_count(); ++i){
		trace2("DEV_GEDA_SUBCKT::expand", param_name(i), param_value(i));
	}
	BASE_SUBCKT::expand();
	COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
	assert(c);
	if (!_parent) { untested();
		unreachable();
		const CARD* model = find_looking_out(c->modelname());
		if(!dynamic_cast<const BASE_SUBCKT*>(model)) { untested();
			throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt");
		}else{ untested();
			_parent = prechecked_cast<const MODEL_GEDA_SUBCKT*>(model);
		}
	}else{
		trace1("DEV_GEDA_SUBCKT::expand", c->modelname());
		// assert(find_looking_out(c->modelname()) == _parent); nope.
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

	unsigned num_nodes_in_model = _parent->subckt()->nodes()->how_many() + 1;
	unsigned num_nodes_in_subckt = num_nodes_in_model + _num_auto_nodes;
	if(_map){
		incomplete();
		delete[] _map;
	}else{
	}
	_map = new unsigned[num_nodes_in_subckt];
	for(unsigned i=0; i<num_nodes_in_subckt; ++i){
		_map[i] = i;
	}
	_part = new PARTITION(num_nodes_in_model);
	subckt()->precalc_first(); // collapses nodes (HACK).
	// maybe
	// for i in subckt{
	//   if net
	//     some_callback
	// }

	try{
		map_subckt_nodes(_parent);
	}catch(Exception){
		incomplete();
		error(bDEBUG, "something went wrong in map_sckt_nodes %s\n", long_label().c_str());
//		delete[] _subckt;
//		_subckt = NULL; hmm.
		delete[] _map;
		_map = NULL;
		throw;
	}
	subckt()->expand();

	delete[] _map;
	_map = NULL;
}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::precalc_last()
{
  BASE_SUBCKT::precalc_last();

  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);
  subckt()->attach_params(&(c->_params), scope());
  subckt()->precalc_last();

  assert(!is_constant()); /* because I have more work to do */

}
/*--------------------------------------------------------------------------*/
void DEV_GEDA_SUBCKT::set_port_by_index(uint_t num, std::string& ext_name)
{
  trace3("DEV_GEDA_SUBCKT::set_port_by_index", long_label(), num, ext_name);
  COMPONENT::set_port_by_index(num, ext_name);
  trace2("DEV_GEDA_SUBCKT::set_port_by_index", _n[num].t_(), _n[num].e_());
  trace1("DEV_GEDA_SUBCKT::set_port_by_index", scope()->nodes()->how_many());
}
/*--------------------------------------------------------------------------*/
double DEV_GEDA_SUBCKT::tr_probe_num(const std::string& x)const
{
  if (Umatch(x, "p ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci, "P");
    }
    return power;
  }else if (Umatch(x, "pd ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci, "PD");
    }
    return power;
  }else if (Umatch(x, "ps ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci, "PS");
    }
    return power;
  }else if (Umatch(x, "ncc ")) {
	  return _num_cc;
  }else{itested();
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
/*--------------------------------------------------------------------------*/
