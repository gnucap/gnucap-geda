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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * base class for net devices, implementation.
 * collapses nodes. may be overriden by loading another net.
 */

#include <globals.h>
#include <u_sim_data.h>
#include "d_net.h"
#include "d_gedasckt.h"
#include "io_trace.h"

#undef HAVE_COLLAPSE
/*--------------------------------------------------------------------------*/
DEV_NET::DEV_NET(const DEV_NET& p) : COMPONENT(p)
{
	_n = _nodes;
	if (p._n == p._nodes) {
		for (int ii = 0;  ii < NODES_PER_BRANCH;  ++ii) {
			_n[ii] = p._n[ii];
		}
	}else{ untested();
		assert(p._nodes);
		// the constructor for a derived class will take care of it
	}
}
/*--------------------------------------------------------------------------*/
void DEV_NET::tr_iwant_matrix()
{
	trace4("DEV_NET::tr_iwant_matrix", long_label(), _n[0].m_(), _n[1].m_(), net_nodes());
	for( unsigned i=net_nodes(); --i>0; ){
		trace1("DEV_NET::tr_iwant_matrix", i);
		assert(_n[i].m_() != INVALID_NODE);
	}
	if(net_nodes()<2){
	}else if(net_nodes()==2){
		_sim->_aa.iwant(_n[0].m_(),_n[1].m_());
		_sim->_lu.iwant(_n[0].m_(),_n[1].m_());
	}else{ incomplete();
	}

	//_sim->_aa.iwant(_n[OUT1].m_(),_n[OUT2].m_());
	//_sim->_lu.iwant(_n[OUT1].m_(),_n[OUT2].m_());
}
/*--------------------------------------------------------------------------*/
void DEV_NET::ac_iwant_matrix()
{
	if(net_nodes()<2){
	}else if(net_nodes()==2){
		// reachable for top level nets.
		// ac simulation possibly still broken due
		// to missing ac_load...?
		_sim->_acx.iwant(_n[0].m_(),_n[1].m_());
	}else{ incomplete();
	}
}
/*--------------------------------------------------------------------------*/
void DEV_NET::precalc_first()
{
	trace2("DEV_NET::precalc_first", long_label(), net_nodes());
	const CARD_LIST* par_scope = scope();
	assert(par_scope);
	COMPONENT::precalc_first();
	e_val(&_resistance, 0., par_scope);

	if(_resistance!=0.){ incomplete();

	}else if(DEV_GEDA_SUBCKT* o=dynamic_cast<DEV_GEDA_SUBCKT*>(owner())){
		for( unsigned i=net_nodes(); --i>0; ){
			trace2("DEV_NET::precalc_first nets", _n[0].e_(), _n[i].e_());
			o->collapse_nodes(_n[0].n_(), _n[i].n_());
		}
	}
}
/*--------------------------------------------------------------------------*/
void DEV_NET::expand()
{
}
/*--------------------------------------------------------------------------*/
void DEV_NET::tr_begin()
{
	trace3("DEV_NET::tr_begin", long_label(), _n[0].m_(), _n[1].m_());
	trace3("DEV_NET::tr_begin", long_label(), _n[0].t_(), _n[1].t_());
	// trace3("DEV_NET::tr_begin", long_label(), _n[0].e_(), _n[1].e_());
	for( unsigned i=net_nodes(); --i>0; ){
	}

	_g0 = 1./OPT::shortckt;
	_g1 = 0;
// 	if(net_nodes()>1){ untested();
// 		q_load();
// 	}else{untested();
// 	}
}
/*--------------------------------------------------------------------------*/
void DEV_NET::tr_load()
{
	if(net_nodes()==1){
		// hmm should not get here...?
		return;
	}else{
	}
	double d = _g0 - _g1;
	_g1 = _g0;
	if (d != 0.) {
		_sim->_aa.load_symmetric(_n[0].m_(), _n[1].m_(), d);
	}else{
	}
}
/*--------------------------------------------------------------------------*/
void DEV_NET::ac_begin()
{
}
/*--------------------------------------------------------------------------*/
void DEV_NET::ac_load()
{
	if(net_nodes()==1){
		// hmm should not get here...?
		return;
	}else{
	}
	double d = 1./OPT::shortckt;
	_sim->_acx.load_symmetric(_n[0].m_(), _n[1].m_(), d);
}
/*--------------------------------------------------------------------------*/
double DEV_NET::tr_probe_num(const std::string& x)const
{
  if (Umatch(x, "v{out} ")) {
    return _n[0].v0();
  } else if (Umatch(x, "dv{out} ")) { untested();
	 incomplete();
    return 18;
  }
  return COMPONENT::tr_probe_num(x);
}
/*--------------------------------------------------------------------------*/
XPROBE DEV_NET::ac_probe_ext(const std::string& x)const
{
	if (Umatch(x, "v ")) {
		return XPROBE(_n[0].vac());
	}else if (Umatch(x, "z ")) { incomplete();
		return XPROBE(.0);
	}else{itested();
		return CKT_BASE::ac_probe_ext(x);
	}
}
/*--------------------------------------------------------------------------*/
namespace {
DEV_NET p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher,"net",&p1);
/*--------------------------------------------------------------------------*/
}
