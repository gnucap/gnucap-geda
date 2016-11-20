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
// #include <d_subckt.h>
#include "d_gedasckt.h"
#include "d_net.h"
#ifdef HAVE_BOOST_ASSIGN
# include <boost/assign.hpp>
#endif
#include "io_trace.h"

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
		std::string param_name(int i)const;
		std::string param_name(int i, int j)const;
		void set_param_by_name(std::string Name, std::string Value);
		bool param_is_printable(int i)const;
		std::string param_value(int i)const;
		void set_port_by_index(uint_t, std::string&);
	private:
		char id_letter()const {return '\0';}
		std::string value_name()const   {return "direction";}
		std::string dev_type()const {return "port";}
		uint_t max_nodes()const {return 2;}
		uint_t min_nodes()const {return 1;}
		uint_t matrix_nodes()const {return 2;}
		uint_t net_nodes()const {return _net_nodes;}
		bool has_iv_probe()const {return true;}
		CARD* clone()const     {return new DEV_PORT(*this);}
		bool print_type_in_spice()const {return false;}
		int param_count()const  {return DEV_NET::param_count()+7;}
		std::string port_name(uint_t i)const{
			assert(i>=0);
			assert(i<2);
			static std::string names[]={"int","ext"};
			return names[i];
		}
//		bool do_tr();
//		void tr_begin(); //DEV_NET
	public:
		node_t _nodes[2];
		// void expand(); DEV_NET
	private:
		PARAMETER<double> _basename;
		PARAMETER<double> _net;
		PARAMETER<double> _pinseq; // int?!
		PARAMETER<double> _pinnumber;
		PARAMETER<double> _symversion;
		PARAMETER<double> _pinlabel; // string?!
		PARAMETER<double> _default_value; // string?!
		static map<std::string, PARA_BASE DEV_PORT::*> _param_dict;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_PORT p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher,"bond|port|none|OUTPUT|INPUT|spice-IO",&p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
map<std::string, PARA_BASE DEV_PORT::*> DEV_PORT::_param_dict
= boost::assign::map_list_of
("basename",  (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_basename))
("net",       (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_net))
("pinseq",    (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_pinseq))
("pinnumber", (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_pinnumber))
("symversion",(PARA_BASE DEV_PORT::*)  (&DEV_PORT::_symversion))
("pinlabel",  (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_pinlabel))
("default_value",
              (PARA_BASE DEV_PORT::*)  (&DEV_PORT::_default_value));
/*--------------------------------------------------------------------------*/
bool DEV_PORT::param_is_printable(int i)const{
	switch(param_count()-1-i) {
		case 0: return _basename.has_hard_value();
		case 1: return _net.has_hard_value();
		case 2: return _pinseq.has_hard_value();
		case 3: return _pinnumber.has_hard_value();
		case 4: return _symversion.has_hard_value();
		case 5: return _pinlabel.has_hard_value();
		case 6: return _default_value.has_hard_value();
		default: return false;
	}
}
/*--------------------------------------------------------------------------*/
void DEV_PORT::set_param_by_name(std::string Name, std::string Value)
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
std::string DEV_PORT::param_name(int i)const
{
	switch(param_count()-i-1) {
		case 0: return("basename");
		case 1: return("net");
		case 2: return("pinseq");
		case 3: return("pinnumber");
		case 4: return("symversion");
		case 5: return("pinlabel");
		case 6: return("default_value");
		default: return COMPONENT::param_name(i);
	}
}
/*--------------------------------------------------------------------------*/
std::string DEV_PORT::param_name(int i, int j)const
{untested();
	if (j == 0) {itested();
		return param_name(i);
	}else{ incomplete();
		return "";
	}
}
/*--------------------------------------------------------------------------*/
std::string DEV_PORT::param_value(int i)const
{
	switch(param_count()-1-i) {
		case 0: return _basename.string();
		case 1: return _net.string();
		case 2: return _pinseq.string();
		case 3: return _pinnumber.string();
		case 4: return _symversion.string();
		case 5: return _pinlabel.string();
		case 6: return _default_value.string();
		default: return COMPONENT::param_value(i);
	}
}
/*--------------------------------------------------------------------------*/
void DEV_PORT::set_port_by_index(uint_t i, std::string& name)
{
	trace2("DEV_PORT::set_port_by_index", i, name);

	if(i){
		// reachable when parsing non-gschem netlist
		// containing ports with 2 connections.
		return;
	}else{
	}

	DEV_NET::set_port_by_index(i, name);

	std::string portname;
	if(_pinlabel.has_hard_value()){
		portname = _pinlabel.string();
	}else if(_net.has_hard_value()){
		portname = _net.string();
	}else{
		portname = short_label();
	}

	// register port during model building
	// hmm: why is this a *DEV*
	if(DEV_GEDA_SUBCKT* o = dynamic_cast<DEV_GEDA_SUBCKT*>(owner())){
		unsigned portpos = o->net_nodes();
		CARD_LIST empty;

		if(_pinseq.has_hard_value()){
			// ouch.
			portpos = atoi(_pinseq.string().c_str())-1;
		} else {
			// rely on pinlabel?
		}

		trace3("DEV_PORT::set_port_by_index", portpos, portname, _pinseq);

		bool done=false;
		for (uint_t i=0; i<o->net_nodes(); ++i) {
			if (portname == o->port_value(i)) {
				o->set_port_by_index(i, portname);
				done = true;
				break;
			}else{
			}
		}
		if(done){
		}else{
			// new port
			if(MODEL_GEDA_SUBCKT* o = dynamic_cast<MODEL_GEDA_SUBCKT*>(owner())){
				std::string default_value="";
				if(_default_value.has_hard_value()){
					default_value=_default_value.string();
				}else{
				}
				trace2("port", _default_value, default_value);
				o->set_port_by_index(portpos, portname, default_value);
			}else{ untested();
			incomplete(); //??
				o->set_port_by_index(portpos, portname);
			}

		}

		// DEV_NET::set_port_by_index(1, portname);
		DEV_NET::set_port_by_index(1, portname);

	}else if(MODEL_GEDA_SUBCKT* o = dynamic_cast<MODEL_GEDA_SUBCKT*>(owner())){ untested();
		USE(o);
	}else if(BASE_SUBCKT* o = dynamic_cast<BASE_SUBCKT*>(owner())){ untested();
		(void) o;
		// using incompatible sckt type?
		untested(); incomplete();
	}else if(owner()){ unreachable();
	}else{
		// DEV_NET::set_port_by_index(1, portname);
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
