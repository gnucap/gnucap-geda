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
 * This is the device 'place' : a schematic node location.
 */
#include <globals.h>
#include <e_compon.h>
#include <e_node.h>
#include "d_place.h"
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
namespace place {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
std::string DEV_PLACE::param_name(int i) const
{
    switch(DEV_PLACE::param_count()-i-1) {
    case 0: return "x";
    case 1: return "y";
    default : return "";
    }
}
/*--------------------------------------------------------------------------*/
std::string DEV_PLACE::param_name(int i,int j) const{
    if (j==0){
        return param_name(i);
    }else{
        return "";
    }
}
/*--------------------------------------------------------------------------*/
void DEV_PLACE::set_param_by_name(std::string Name,std::string Value)
{
    for (int i=DEV_PLACE::param_count()-1; i>=0; --i) {
        for (int j=0; DEV_PLACE::param_name(i,j)!=""; ++j) {
            if(Name==DEV_PLACE::param_name(i,j)) {
                DEV_PLACE::set_param_by_index(i,Value,0);
                return;
            }else{
            }
        }
    }
    throw Exception_No_Match(Name+" "+Value);
}
/*--------------------------------------------------------------------------*/
void DEV_PLACE::set_param_by_index(int i, std::string& Value, int offset)
{
    switch(DEV_PLACE::param_count()-1-i) {
    case 0: _x = atoi(Value.c_str()); break;
    case 1: _y = atoi(Value.c_str()); break;
    default: throw Exception_Too_Many(i,2,offset);
    }
}
/*--------------------------------------------------------------------------*/
bool DEV_PLACE::param_is_printable(int i)const
{
    switch(DEV_PLACE::param_count()-1-i) {
    case 0: return true;
    case 1: return true;
    default: return COMPONENT::param_is_printable(i);
    }
}
/*--------------------------------------------------------------------------*/
std::string DEV_PLACE::param_value(int i)const
{
    switch(DEV_PLACE::param_count()-1-i) {
        case 0: return to_string(_x);
        case 1: return to_string(_y);
    }
    unreachable();
    return "errorvalue";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_PLACE p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher,"place",&p1);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=4:et
