/*
 * (C) 2012 Felix Salfelder
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
 *
 * a port connects a net to the pin of a sckt
 * this is meant to make i/o port symbols work.
 */
#include <globals.h>
#include <e_compon.h>
#include <e_node.h>
#include <u_nodemap.h>
#include <d_subckt.h>
#include "d_net.h"
#ifdef HAVE_BOOST_ASSIGN
# include <boost/assign.hpp>
#endif
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
using namespace std;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_PORT : public DEV_NET {
	public:
		explicit DEV_PORT() : DEV_NET() {}
		explicit DEV_PORT(const DEV_PORT& p) : DEV_NET(p){}
		~DEV_PORT(){}
	private:
		string param_name(int i)const;
		void set_param_by_name(string Name, string Value);
		bool param_is_printable(int i)const;
		string param_value(int i)const;
		void set_port_by_index(uint_t, string&);
	private:
		char id_letter()const {return '\0';}
		string value_name()const   {return "direction";}
		string dev_type()const {return "port";}
		uint_t max_nodes()const {return 2;}
		uint_t min_nodes()const {return 1;}
		uint_t matrix_nodes()const {return 2;}
		uint_t net_nodes()const {return _net_nodes;}
		bool has_iv_probe()const {return true;}
		CARD* clone()const     {return new DEV_PORT(*this);}
		bool print_type_in_spice()const {return false;}
		int param_count()const  {return DEV_NET::param_count()+6;}
		string port_name(uint_t i)const{
			assert(i>=0);
			assert(i<2);
			static string names[]={"int","ext"};
			return names[i];
		}
//		bool do_tr();
//		void tr_begin(); //DEV_NET
	public:
		node_t _nodes[2];
		// void expand(); DEV_NET
	private:
		PARAMETER<double> basename;
		PARAMETER<double> net;
		PARAMETER<double> pinseq; // int?!
		PARAMETER<double> pinnumber;
		PARAMETER<double> symversion;
		PARAMETER<double> pinlabel; // string?!
		static map<string, PARA_BASE DEV_PORT::*> _param_dict;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_PORT p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher,"bond|port|none|OUTPUT|INPUT|spice-IO",&p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
map<string, PARA_BASE DEV_PORT::*> DEV_PORT::_param_dict
= boost::assign::map_list_of
("basename",  (PARA_BASE DEV_PORT::*)  (&DEV_PORT::basename))
("net",       (PARA_BASE DEV_PORT::*)  (&DEV_PORT::net))
("pinseq",    (PARA_BASE DEV_PORT::*)  (&DEV_PORT::pinseq))
("pinnumber", (PARA_BASE DEV_PORT::*)  (&DEV_PORT::pinnumber))
("symversion",(PARA_BASE DEV_PORT::*)  (&DEV_PORT::symversion))
("pinlabel",  (PARA_BASE DEV_PORT::*)  (&DEV_PORT::pinlabel));
/*--------------------------------------------------------------------------*/
bool DEV_PORT::param_is_printable(int i)const{
	switch(param_count()-1-i) {
		case 0: return basename.has_hard_value();
		case 1: return net.has_hard_value();
		case 2: return pinseq.has_hard_value();
		case 3: return pinnumber.has_hard_value();
		case 4: return symversion.has_hard_value();
		case 5: return pinlabel.has_hard_value();
		default: return false;
	}
}
/*--------------------------------------------------------------------------*/
void DEV_PORT::set_param_by_name(string Name, string Value)
{
	PARA_BASE DEV_PORT::* x = _param_dict[Name];
	trace3("DEV_PORT::set_param_by_name", Name, OPT::case_insensitive, x);
	if(x) {
		PARA_BASE* p = &(this->*x);
		*p = Value;
		return;
	}
	throw Exception_No_Match(Name);
}
/*--------------------------------------------------------------------------*/
string DEV_PORT::param_name(int i)const
{
	switch(param_count()-i-1) {
		case 0: return("basename");
		case 1: return("net");
		case 2: return("pinseq");
		case 3: return("pinnumber");
		case 4: return("symversion");
		case 5: return("pinlabel");
		default: return COMPONENT::param_name(i);
	}
}
/*--------------------------------------------------------------------------*/
string DEV_PORT::param_value(int i)const
{
	switch(param_count()-1-i) {
		case 0: return basename.string();
		case 1: return net.string();
		case 2: return pinseq.string();
		case 3: return pinnumber.string();
		case 4: return symversion.string();
		case 5: return pinlabel.string();
		default: return COMPONENT::param_value(i);
	}
}
/*--------------------------------------------------------------------------*/
void DEV_PORT::set_port_by_index(uint_t i, string& name){
	if(i){
		trace2("DEV_PORT::set_port_by_index", i, name);
		// reachable when parsing non-gschem netlist
		// containing ports with 2 connections.
		incomplete();
		return;
	}
	DEV_NET::set_port_by_index(i, name);
	if(MODEL_SUBCKT* o = dynamic_cast<MODEL_SUBCKT*>(owner())){
		string portname = name;
		if(pinlabel.has_hard_value()){
			portname = pinlabel.string();
		}else if(net.has_hard_value()){
			portname = net.string();
		}
		unsigned portpos = o->net_nodes();
		CARD_LIST empty;

		if(pinseq.has_hard_value()){
			// ouch.
			portpos = atoi(pinseq.string().c_str())-1;
		} else {
			// rely on pinlabel?
			untested();
		}

		trace3("DEV_PORT::set_port_by_index", portpos, portname, pinseq);
		o->set_port_by_index(portpos, portname);
		DEV_NET::set_port_by_index(1, portname);
	}
}
/*---------------------------------------------------------------------------*/
// collapse happens after the parents map_sckt_nodes.
// n[1] is external
#if 0
void DEV_PORT::expand()
{
#ifdef HAVE_COLLAPSE
	trace3("DEV_PORT::collapse", long_label(), _n[0].e_(), _n[1].e_());
	_n[0].collapse(this, _n[1]);
#else
	incomplete();
#endif
 	assert(_n[0].e_() == _n[1].e_());
}
#endif
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
}
