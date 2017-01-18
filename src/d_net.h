/* 
 * Written by Savant Krishna <savant.2020@gmail.com>
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
 * This is the device 'net' : a connection between nodes.
 */
#ifndef D_NET__H
#define D_NET__H
#ifdef COMPLEX
# error COMPLEX mess
#endif
#include <md.h>
#include <e_compon.h>
#include <e_node.h>
#include <u_xprobe.h>
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

#undef HAVE_COLLAPSE
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_NET : public COMPONENT {

public:
  explicit DEV_NET() : COMPONENT() { untested();
    _n=_nodes;
  }
  explicit DEV_NET(const DEV_NET& p);
  ~DEV_NET(){}
private:
  void precalc_first();
  void expand();
  // void precalc_last();
private:
  void tr_iwant_matrix();
// void tr_accept();
  double tr_probe_num(const std::string&)const;
  XPROBE ac_probe_ext(const std::string& x)const;
  void ac_iwant_matrix();
private:
  bool      param_is_printable(int)const {return false;}
private:
  char      id_letter()const {return 'N';}
  std::string value_name()const   {return "color";}
  std::string dev_type()const { return "net";}
  uint_t    max_nodes()const {return 2;}
  uint_t    min_nodes()const {return 2;}
  uint_t    matrix_nodes()const {return _net_nodes;}
  uint_t    net_nodes()const {return 2;}

  bool      has_iv_probe()const {return true;}
  CARD*     clone()const     {return new DEV_NET(*this);}
  bool      print_type_in_spice()const {return false;}
//		bool do_tr();
  void tr_begin();
  void tr_load();
  void ac_begin();
  void ac_load();
protected:
  int param_count()const  {return COMPONENT::param_count();}
private:
  std::string port_name(uint_t i)const{ untested();
    assert(i>=0);
    assert(i<2);
    static std::string names[]={"p","n"};
    return names[i];
  }
  PARAMETER<double> _resistance;
public:
  node_t    _nodes[NODES_PER_BRANCH];
private:
#ifndef HAVE_COLLAPSE
  double _g0, _g1;
#endif
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:et
