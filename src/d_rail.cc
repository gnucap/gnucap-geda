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
#include <e_compon.h>
#include <e_node.h>
#include <u_nodemap.h>
#include <d_subckt.h>
#include "d_net.h"
#include <boost/assign.hpp>
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
using namespace std;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_RAIL : public DEV_NET {
	public:
		explicit DEV_RAIL() : DEV_NET() {}
		explicit DEV_RAIL(const DEV_RAIL& p) : DEV_NET(p){}
		~DEV_RAIL(){}
	private:
		string param_name(int i)const;
		void set_param_by_name(string Name, string Value);
		bool param_is_printable(int i)const;
		string param_value(int i)const;
		void set_port_by_index(uint_t, string&);
	private:
		char id_letter()const {return '\0';}
		string value_name()const   {return "direction";}
		string dev_type()const {return "rail";}
		uint_t max_nodes()const {return 2;}
		uint_t min_nodes()const {return 1;}
		uint_t matrix_nodes()const {return 2;}
		uint_t net_nodes()const {return _net_nodes;}
		bool has_iv_probe()const {return true;}
		CARD* clone()const     {return new DEV_RAIL(*this);}
		bool print_type_in_spice()const {return false;}
		int param_count()const  {return DEV_NET::param_count()+5;}
		string port_name(uint_t i)const{
			assert(i>=0);
			assert(i<2);
			static string names[]={"pin","rail"};
			return names[i];
		}
	public:
		node_t    _nodes[2];
		//void expand(); DEV_NET
	private:
		PARAMETER<double> basename;
		PARAMETER<double> net;
		PARAMETER<double> pinseq;
		PARAMETER<double> pinnumber;
		PARAMETER<double> symversion;
#ifdef HAVE_PARA_BASE
		static map<string, PARA_BASE DEV_RAIL::*> param_dict;
#endif
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_RAIL p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher,"rail",&p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#ifdef HAVE_PARA_BASE
map<string, PARA_BASE DEV_RAIL::*> DEV_RAIL::param_dict
= boost::assign::map_list_of
("basename",  (PARA_BASE DEV_RAIL::*)  (&DEV_RAIL::basename))
("net",       (PARA_BASE DEV_RAIL::*)  (&DEV_RAIL::net))
("pinseq",    (PARA_BASE DEV_RAIL::*)  (&DEV_RAIL::pinseq))
("pinnumber", (PARA_BASE DEV_RAIL::*)  (&DEV_RAIL::pinnumber))
("symversion",(PARA_BASE DEV_RAIL::*)  (&DEV_RAIL::symversion));
#endif
/*--------------------------------------------------------------------------*/
bool DEV_RAIL::param_is_printable(int i)const{
	switch(param_count()-1-i) {
		case 0: return basename.has_hard_value();
		case 1: return net.has_hard_value();
		case 2: return pinseq.has_hard_value();
		case 3: return pinnumber.has_hard_value();
		case 4: return symversion.has_hard_value();
		default: return false;
	}
}
/*--------------------------------------------------------------------------*/
void DEV_RAIL::set_param_by_name(string Name, string Value)
{
#ifdef HAVE_PARA_BASE
	PARA_BASE DEV_RAIL::* x = param_dict[Name];
	trace3("DEV_RAIL::set_param_by_name", Name, OPT::case_insensitive, x);
	if(x) {
		PARA_BASE* p = &(this->*x);
		*p = Value;
		return;
	}
	throw Exception_No_Match(Name);
#else
	incomplete();
#endif
}
/*--------------------------------------------------------------------------*/
string DEV_RAIL::param_name(int i)const
{
	switch(param_count()-i-1) {
		case 0: return("basename");
		case 1: return("net");
		case 2: return("pinseq");
		case 3: return("pinnumber");
		case 4: return("symversion");
		default: return "";
	}
}
/*--------------------------------------------------------------------------*/
string DEV_RAIL::param_value(int i)const
{
	switch(param_count()-1-i) {
		case 0: return basename.string();
		case 1: return net.string();
		case 2: return pinseq.string();
		case 3: return pinnumber.string();
		case 4: return symversion.string();
		default: return "";
	}
}
/*--------------------------------------------------------------------------*/
void DEV_RAIL::set_port_by_index(uint_t i, string& name){
	if(i){
		trace2("DEV_RAIL::set_port_by_index", i, name);
		// reachable when parsing non-gschem netlist
		// containing ports with 2 connections.
		incomplete();
		return;
	}
	DEV_NET::set_port_by_index(i, name);
	trace3("DEV_RAIL::set_port_by_index", i, name, net);
	string portname = name;
	if(net.has_hard_value()){
		portname = net.string();
	}

	// hmm better connect rails to the outside (optional ports?)
// 	portname = string(INT_PREFIX) + portname;
	// n_(1).new_sckt_node( portname, scope());
	DEV_NET::set_port_by_index(1, portname);
}
/*--------------------------------------------------------------------------*/
}
