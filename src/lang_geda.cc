/* This file is the plugin for the gEDA-gschem plugin
 * The documentation of how this should work is at bit.ly(slash)gnucapwiki
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package.
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
 *
 */

#include <l_lib.h>
#include <c_comand.h>
#include <d_dot.h>
#include <d_coment.h>
#include <d_subckt.h>
#include <e_model.h>
#include <u_lang.h>
#include <u_nodemap.h>
#include <fts.h>
#include <io_trace.h>
#include <gmpxx.h> // to workaround bug in gmp header about __cplusplus
#define COMPLEX NOCOMPLEX // COMPLEX already came from md.h
extern "C"{
# include <libgeda/libgeda.h>
}
#include "symbol.h"
#include "d_net.h"
#include "d_place.h"
#undef COMPLEX
/*--------------------------------------------------------------------------*/
#define DUMMY_PREFIX string("!_")
#define INT_PREFIX string("x_")
/*--------------------------------------------------------------------------*/
// using namespace std;
using std::string;
using std::vector;
using std::pair;
/*--------------------------------------------------------------------------*/
namespace geda{
/*--------------------------------------------------------------------------*/
class LANG_GEDA : public LANGUAGE {
   friend class CMD_GSCHEM;
   TOPLEVEL* pr_current;

   struct portinfo {
		portinfo(string n, int a, int b): name(n), x(a), y(b) {}
      string name;
      int x,y;
   };
   mutable std::queue<portinfo> _placeq;
   struct netinfo {
		netinfo(int _x0, int _y0, int _x1, int _y1, unsigned c):
		    x0(_x0), y0(_y0), x1(_x1), y1(_y1), color(c) {}
      int x0, y0, x1, y1;
      unsigned color;
   };
   mutable std::queue<netinfo> _netq;
   mutable GEDA_SYMBOL* _C; //stashes C command and body (HACK/workaround)
   public:
   LANG_GEDA() : LANGUAGE(), _C(NULL){
      trace0("gedainit");
      scm_init_guile(); // urghs why?
      libgeda_init();
      pr_current = s_toplevel_new ();
      g_rc_parse(pr_current, "gschemrc", NULL);
      // i_vars_set (pr_current); // why?
   }

   private:
   mutable bool _gotline_sym;
   public:
   enum MODE {mATTRIBUTE, mCOMMENT} _mode;
   mutable int _no_of_lines;
   mutable bool _gotline;
   mutable std::string _componentname;
   std::string name()const {return "gschem";}
   bool case_insensitive()const {return false;}
   UNITS units()const {return uSI;}

   public: //functions to be declared
   std::string arg_front()const {
      return " "; //arbitrary
   }
   std::string arg_mid()const {
      return "="; //arbitrary
   }
   std::string arg_back()const {
      return "";  //arbitrary
   }

   public:
   void		  parse_top_item(CS&, CARD_LIST*);
   void parse_item_(CS& cmd, CARD* owner, CARD_LIST*) const;
   DEV_COMMENT*  parse_comment(CS&, DEV_COMMENT*);
   DEV_DOT*	  parse_command(CS&, DEV_DOT*);
   MODEL_CARD*	  parse_paramset(CS&, MODEL_CARD*);
   MODEL_SUBCKT* parse_module(CS&, MODEL_SUBCKT*);
   COMPONENT*    parse_componmod(CS&, COMPONENT*);
   COMPONENT*	  parse_instance(CS&, COMPONENT*);
   std::string*  parse_pin(CS& cmd, COMPONENT* x, int index, bool ismodel)const;
   std::string	  find_type_in_string(CS&) const;
   // gnucap backwards compatibility
   std::string	  find_type_in_string(CS&x) {return const_cast<const LANG_GEDA*>(this)->find_type_in_string(x);}
   void parse_net(CS& cmd, COMPONENT* x) const;
   void parse_place(CS& cmd, COMPONENT* x);

   private:
   void print_paramset(OMSTREAM&, const MODEL_CARD*);
   void print_module(OMSTREAM&, const MODEL_SUBCKT*);
   void print_instance(OMSTREAM&, const COMPONENT*);
   void print_comment(OMSTREAM&, const DEV_COMMENT*);
   void print_command(OMSTREAM& o, const DEV_DOT* c);
   void print_component(OMSTREAM& o, const COMPONENT* x);

   GEDA_SYMBOL* parse_C(CS& cmd)const;
   void parse_component(CS& cmd,COMPONENT* x);
   std::vector<std::string*> parse_symbol_file(CARD* x, std::string basename) const;
   DEV_DOT* parse_symbol_file(DEV_DOT*, const GEDA_SYMBOL&)const;
   const place::DEV_PLACE* find_place(const CARD* x, int xco, int yco)const;
   const place::DEV_PLACE* find_place(const CARD* x, std::string name)const;
   const place::DEV_PLACE* find_place(const CARD* x, std::string xco, std::string yco)const;
   pair<int,int> find_place_(const CARD* x, std::string name)const;
   string* find_place_string(const CARD* x, std::string name)const;
   void connect(CARD *x, int x0, int y0, int x1, int y1)const;
   static void read_file(string, CARD_LIST* Scope, MODEL_SUBCKT* owner=0);
   static void read_spice(string, CARD_LIST* Scope, MODEL_SUBCKT* owner=0);

   static GEDA_SYMBOL_MAP _symbol;
   static unsigned _nodenumber;
   static unsigned _netnumber;
   // put fake models here?
   // static CARD_LIST _symbols;
}lang_geda;

DISPATCHER<LANGUAGE>::INSTALL
d(&language_dispatcher, lang_geda.name(), &lang_geda);
/*----------------------------------------------------------------------*/
GEDA_SYMBOL_MAP LANG_GEDA::_symbol;
/*----------------------------------------------------------------------*/
unsigned LANG_GEDA::_netnumber, LANG_GEDA::_nodenumber;
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
// FIXME: need something like
//std::string LANGUAGE::getlines(FILE *fileptr) const
//to detect { } bodies correctly
// current workaround: read into _C and _gotline hack.
/*----------------------------------------------------------------------*/
// FIXME: _netq, _placeq might not be necessary anymore
// (since find_type_in_string finds the type)
/*----------------------------------------------------------------------*/
//Finds type from find_type_in_string
static void parse_type(CS& cmd, CARD* x)
{
   assert(x);
   std::string new_type;
   new_type=lang_geda.find_type_in_string(cmd);
   x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
std::string* LANG_GEDA::parse_pin(CS& cmd, COMPONENT* x, int index, bool ismodel)const
{
   //assert(x); can parse NULL also
   trace0("Got into parse_pin");
   assert( find_type_in_string(cmd) =="pin");
   string dump;
   cmd>>"P";
   std::string* coord = new std::string[3];
   if (!ismodel){
      std::string pinattributes[7];
      for(int i=0;i<7;i++){
         cmd>>" ">>pinattributes[i];
      }
      if (pinattributes[6]=="1"){
         coord[0]=pinattributes[2];
         coord[1]=pinattributes[3];
      }else if (pinattributes[6]=="0"){
         coord[0]=pinattributes[0];
         coord[1]=pinattributes[1];
      }
   }
   else{
      cmd>>dump;
   }
   std::string    _portvalue="_";
   static unsigned number;
   try{
      cmd.get_line("");
   }catch(Exception_End_Of_Input&){
      return NULL;
   }
   std::string temp=(cmd.fullstring()).substr(0,1);
   if(cmd.match1('{')){
      cmd>>"{";
      for(;;){
         cmd.get_line("");
         if(cmd>>"}"){
            break;
         }else{
            if (cmd>>"T"){
               cmd>>dump;
            }
            else{
               std::string _pname=cmd.ctos("=","",""),_pvalue;
               cmd>>"=">>_pvalue;
               if(_pname=="pinlabel"){
                  _portvalue=_pvalue+_portvalue;
               }else if (_pname=="pintype"){
                  _portvalue=_portvalue+_pvalue;
               }
            }
         }
      }
   }
   if(ismodel and x){
      string portname = "np_" + _portvalue+::to_string(number++);
      x->set_port_by_index(index, portname);
      return NULL;
   }else{
      return coord;
   }
}
/*--------------------------------------------------------------------------*/
// FIXME: do symbol_type?
std::vector<string*> LANG_GEDA::parse_symbol_file(CARD* x,
      string basename)const
{
   assert(!_C);
   assert(!_netq.size());
   assert(!_placeq.size());
   _gotline_sym = 0;
   COMPONENT* c = dynamic_cast<COMPONENT*>(x);
   MODEL_SUBCKT* m = dynamic_cast<MODEL_SUBCKT*>(x);
   const CLibSymbol* symbol = s_clib_get_symbol_by_name(basename.c_str());
   if(!symbol){
      throw(Exception_Cant_Find("parsing gedanetlist", basename ));
   }
   std::string filename(s_clib_symbol_get_filename(symbol));
   std::string dump;


   // name of the symbol category (collection name)
   // const char* sourcename = s_clib_source_get_name (s_clib_symbol_get_source(symbol));
   //
   // the file contents as string (wtf?!)
   // char* data = s_clib_symbol_get_data(symbol);

   // trace2("LANG_GEDA::parse_symbol_file", basename, filename);
   GEDA_SYMBOL s = _symbol[basename];
   CS sym_cmd(CS::_INC_FILE, filename);
   //Now parse the sym_cmd which will get lines
   int index=0;
   std::vector<std::string*> coord;
   while(true){
      try{
         if (!_gotline_sym) sym_cmd.get_line("");
      }catch (Exception_End_Of_Input&){
         break;
      }
      std::string linetype = find_type_in_string(sym_cmd);
      trace2("LANG_GEDA::parse_symbol_file", linetype, sym_cmd.fullstring());
      bool ismodel=false;
      if (x && x->short_label()==DUMMY_PREFIX+basename){
         ismodel=true;
      }
      if (linetype=="dev_comment"){
         // nop
      }else if (linetype=="pin" && (c || !x)){
         // trace2("parse_symbol_file parsing pin", basename, sym_cmd.fullstring());
         coord.push_back(parse_pin(sym_cmd,c,index++,ismodel));
         // trace2("parse_symbol_file pin done", basename, sym_cmd.fullstring());
      }else if(linetype=="pin"){
         // pin. this is a device, but we are in command mode
         coord.push_back(new string("foo"));
         return coord;
      }else if(linetype=="graphical"){
         sym_cmd>>"graphical=";
         std::string value;
         sym_cmd>>value;
         if(value=="1"){
            trace0("graphical");
            return coord;
         }
      }else if(linetype=="file"){
         trace2("parse_symbol_file", sym_cmd.fullstring(), linetype);
         sym_cmd>>"file=";
         sym_cmd>>dump;
         DEV_DOT* d = dynamic_cast<DEV_DOT*>(x);
         if(d && dump != "?"){
            d->set(d->s() + " " + dump);
         }else{
            untested();
         }
      }else if(m && linetype=="refdes"){
         // skip
      }else if(c && linetype=="refdes"){
         sym_cmd >> "refdes=";
         sym_cmd >> dump;
         x->set_label(dump);
      }else if(linetype=="device"){
         sym_cmd>>"device=";
         sym_cmd>>dump;
         DEV_DOT* d = dynamic_cast<DEV_DOT*>(x);
         if(d){
            d->set(dump);
         }else if( (c = dynamic_cast<COMPONENT*>(x) )){
            // c->set_label(dump);
         }else{
            incomplete();
         }
      }else if(c && linetype != "" ){
         sym_cmd>>linetype;
         unsigned here = sym_cmd.cursor();
         sym_cmd >> "=";
         if(!sym_cmd.stuck(&here))
            try {
               sym_cmd>>dump;
               untested();
               x->set_param_by_name(linetype, dump);
            } catch (Exception_No_Match&) {
               untested();
            } catch (Exception_Too_Many&) {
               untested();
            }
      } else {
         sym_cmd>>dump;
         trace3("pa dump", sym_cmd.fullstring(), linetype, dump);
      }
   }
   trace0("done symbol");
   return coord;
}
/*--------------------------------------------------------------------------*/
//place <nodename> x y
void LANG_GEDA::parse_place(CS& cmd, COMPONENT* x)
{
   trace2("parse_place", x->long_label(), cmd.fullstring());
   assert(x);
   assert(find_type_in_string(cmd)=="place");
   if( _placeq.size() ){
      portinfo p = _placeq.front();
      x->set_param_by_name("x", to_string(p.x));
      x->set_param_by_name("y", to_string(p.y));
      string portname = string(INT_PREFIX) + p.name;
      x->set_port_by_index(0, portname);
      _placeq.pop();
      x->set_label( to_string(p.x)+":"+to_string(p.y) );

      cmd.reset();
   } else if ( cmd.umatch("place") ) {
      incomplete();
      cmd>>"place";
      std::string _portname,_x,_y;
      cmd>>" ">>_portname>>" ">>_x>>" ">>_y;
      x->set_param_by_name("x",_x);
      x->set_param_by_name("y",_y);
      string portname = string(INT_PREFIX) + "np_" + _portname;
      x->set_port_by_index(0, portname);
      x->set_label( _x + ":" + _y);
   } else {
      trace1("parse_place, huh?", cmd.fullstring());
      unreachable();
   }
}
/*--------------------------------------------------------------------------*/
const place::DEV_PLACE* LANG_GEDA::find_place(const CARD* x, string xco, string yco)const
{
   return find_place(x, atoi(xco.c_str()), atoi(yco.c_str()));
}
/*--------------------------------------------------------------------------*/
const place::DEV_PLACE* LANG_GEDA::find_place(const CARD* x, int xco, int yco)const
{
   const CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();
   for (CARD_LIST::const_iterator ci = scope->begin(); ci != scope->end(); ++ci) {
      if( place::DEV_PLACE* pl=dynamic_cast<place::DEV_PLACE*>(*ci)){
         if(xco==pl->x() && yco==pl->y()){
            return pl;
         }
      }
   }
   return NULL;
}
/*--------------------------------------------------------------------------*/
static bool in_order(int a, int b, int c){
   if (a<b){
      return b<c;
   }else if (b<a){
      return c<b;
   }
   return 0;
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::connect(CARD *x, int x0, int y0, int x1, int y1)const
{
   trace4("LANG_GEDA::connect", x0, y0, x1, y1);
   assert(x0!=x1 || y0!=y1);
   CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();
   for(CARD_LIST::const_iterator ci = scope->begin(); ci != scope->end(); ++ci) {
      if(const DEV_NET* net=dynamic_cast<DEV_NET*>(*ci)){
         // exclude external ports and rails (HACK)
         if((*ci)->net_nodes()<2) continue;
         if((net->port_value(0)+"AA").substr(0, INT_PREFIX.length()) != INT_PREFIX) continue;
         if((net->port_value(1)+"AA").substr(0, INT_PREFIX.length()) != INT_PREFIX) continue;


         // connect end points to existing nets
         // (this will take ages.)
         const place::DEV_PLACE* n1 = find_place(x, net->port_value(0));
         assert(n1);
         const place::DEV_PLACE* n2 = find_place(x, net->port_value(1));
         assert(n2);
         assert(n1->x()!=n2->x() || n1->y()!=n2->y());
         if (n1->x() == n2->x() && (y0 == y1)){
            if (in_order( n2->y(), y1, n1->y())){
               if (n1->x() == x0){
                  assert( y0 !=  n1->y());
                  _netq.push( netinfo( x0, y0, n1->x(), n1->y(), 4 ));
               } else if (n2->x() == x1){
                  assert( y1 !=  n1->y());
                  _netq.push( netinfo( x1, y1, n1->x(), n1->y(), 4 ));
               }
            }
         } else if (n1->y() == n2->y() && (x0 == x1)){
            if (in_order(n1->x(), x1, n2->x())){
               if (n1->y() == y0){
                  assert( x0 !=  n1->x());
                  _netq.push( netinfo( x0, y0, n1->x(), n1->y(), 4 ));
               } else if (n2->y() == y1){
                  assert( x1 !=  n1->x());
                  _netq.push( netinfo( x1, y1, n1->x(), n1->y(), 4 ));
               }
            }
         }
      } else if(const place::DEV_PLACE* pl=dynamic_cast<place::DEV_PLACE*>(*ci)){
         // connect interior places
         int _x = pl->x();
         int _y = pl->y();
         if (y0==y1){ // horizontal
            if( in_order( x1, _x, x0) && _y==y0 ){
               unsigned col = 5;
               // connect place to 1st endpoint.
               assert(x0!=_x);
               _netq.push( netinfo( x0, y0, _x, _y, col ));
            }
            else{
            }
         }else if (x0==x1){
            if( in_order( y1, _y, y0) && _x==x0 ){
               unsigned col = 5;
               // connect place to 1st endpoint.
               assert(x0!=_x);
               _netq.push( netinfo( x0, y0, _x, _y, col ));
            }else{
            }
         }
         else{
         }
      }
   }
   trace0("connect done");
}
/*--------------------------------------------------------------------------*/
// A net is in form N x0 y0 x1 y1 c
// Need to get x0 y0 ; x1 y1 ;
// Need to go through all the nets. Anyway?
// Need to save them in other forms? How to go through all cards?
// Need to specify a name for a card?
void LANG_GEDA::parse_net(CS& cmd, COMPONENT* x)const
{
   assert(x);
   // assert(lang_geda.find_type_in_string(cmd)=="net"); // no. at end of body...
   int coord[4];
   if(_netq.size()){
      netinfo n = _netq.front();
      coord[0] = n.x0;
      coord[1] = n.y0;
      coord[2] = n.x1;
      coord[3] = n.y1;
      x->set_param_by_name("color", ::to_string(n.color));
      _netq.pop();
      x->set_label("extranet"+::to_string(_netnumber++));
   } else { // parse
      assert(cmd.fullstring().c_str()[0] == 'N');
      unsigned here=cmd.cursor();
      // x0 y0 x1 y1 color
      std::string parsedvalue[5];
      int i=0;
      while (i<5) {
         if (cmd.is_alnum()){
            cmd>>" ">>parsedvalue[i];
            if(i!=4) coord[i] = atoi(parsedvalue[i].c_str());
         }else{
            cmd.warn(bDANGER, here, x->long_label() +": Not correct format for net");
            return; // throw?
            break;
         }
         ++i;
      }
      x->set_param_by_name("color",parsedvalue[4]);
      x->set_label("net"+::to_string(_netnumber++));
   }

   const COMPONENT* port = find_place(x, coord[0], coord[1]);
   string portname;
   if(!port){
      portname = "nn_" + ::to_string(_nodenumber++);
      _placeq.push( portinfo(portname, coord[0], coord[1]) );
      portname = string(INT_PREFIX) + portname;
   } else {
      portname = port->port_value(0);
   }
   x->set_port_by_index(0, portname);

   port = find_place(x, coord[2], coord[3]);
   if(!port){
      untested();
      portname = "nn_" + ::to_string(_nodenumber++);
      _placeq.push( portinfo(portname, coord[2], coord[3]));
      portname = string(INT_PREFIX) + portname;
   } else {
      portname = port->port_value(0);
   }
   x->set_port_by_index(1, portname);

   connect(x, coord[0], coord[1], coord[2], coord[3]);

   if(_placeq.size() || _netq.size()){
      //        unneccessary?
      trace1("queuing place", cmd.fullstring());
      cmd.reset();
   }
   //To check if there are any attributes
   try {
      cmd.get_line("gnucap-geda>");
   }catch(Exception_End_Of_Input&){
      return;
   }
   std::string _paramvalue,_paramname,dump;
   if(cmd.match1('{')){
      for (;;) {
         cmd.get_line("gnucap-geda-net>");
         if (cmd >> "}") {
            break;
         }else{
            if(cmd>>"T"){
               cmd>>dump;
            }
            else {
               std::string _paramname=cmd.ctos("=","",""),_paramvalue;
               cmd>>"=">>_paramvalue;
               if (_paramname=="netname" && _paramvalue!="?"){
                  x->set_label(_paramvalue);
               }else{
                  untested();
                  try{
                     x->set_param_by_name(_paramname,_paramvalue);
                  }catch(Exception_No_Match){
                     untested();
                  }
               }
            }
         }
      }
   } else {
      cmd.reset();
      _gotline = true;
      //OPT::language->new__instance(cmd,NULL,x->scope());
      return;
   }
   _gotline = true;
   cmd.reset();
   assert(!cmd.is_end()); // there could be a queue...
   if(_placeq.size()){
      trace1("done net. queuing place", cmd.fullstring());
      cmd.reset();
   }
}
/*--------------------------------------------------------------------------*/
pair<int,int> componentposition(int* absxy, int* relxy, int angle, bool mirror);
/*--------------------------------------------------------------------------*/
void LANG_GEDA::parse_component(CS& cmd,COMPONENT* x)
{ // "component" means instance of a subckt
   trace4("LANG_GEDA::parse_component", x->long_label(), cmd.fullstring(),
         hp(x->owner()), hp(x->scope()));
   assert(x);
   assert(!_placeq.size());
   assert(_C);
   int c_x, c_y, angle;
   bool mirror;
   string dump,basename;
   std::string type=lang_geda.find_type_in_string(cmd);
   GEDA_SYMBOL* dev = _C;
   _C = 0; // to make parse_symbol_file work
   if(dev->has_key("device")){
      assert(type==(*dev)["device"] || type==DUMMY_PREFIX+((*dev)["basename"]));
   } else{
      untested();
   }
   std::string source("");

   // cannot read from cmd. too late.
   // cmd >> " " >> mirror >> " " >> basename;
   basename = (*dev)["basename"];
   c_x = dev->x;
   c_y = dev->y;
   angle = dev->angle;
   mirror = dev->mirror;
   trace1("LANG_GEDA::parse_component", basename);

   //To get port names and values from symbol?
   //Then set params below
   //Search for the file name
   // FIXME: should not need basename (dev instead)
   // FIXME: reads symbol file twice
   std::vector<std::string*> coordinates=parse_symbol_file(x,basename);
   int newx, newy;

   try{
      x->set_param_by_name("basename", basename);
   } catch(Exception_No_Match){
      untested();
   }
   // set parameters

   for(GEDA_SYMBOL::const_iterator i = dev->begin(); i!=dev->end(); ++i) {
      if (i->first == "device") {
         x->set_dev_type( i->second );
      } else if ( i->first == "refdes" && i->second != "?" ) {
         x->set_label(i->second);
         //                else if (_paramname=="source")
         //                    source = _paramvalue;
      } else {
         try{
            x->set_param_by_name(i->first, i->second);
         } catch (Exception_No_Match){
            untested();
         }
      }
   }

   static unsigned instance;
   if(x->short_label()==""){
      if(dev->has_key("net")){
         x->set_label((*dev)["net"]);
      }else{
         x->set_label(basename+"_"+to_string(instance++));
      }
   }

   // connect ports
   int index = 0;
   index = 0;
   trace1("LANG_GEDA::parse_component setting ports", x->long_label());
   for (set<GEDA_PIN>::const_iterator i = dev->pinbegin(); i!=dev->pinend(); ++i ){
      int cc[2];
      cc[0] = c_x;
      cc[1] = c_y;
      int delta[2];
      delta[0] = - i->x0();
      delta[1] = - i->y0();
      pair<int,int> new_ = componentposition( cc, delta, angle, mirror );
      newx = new_.first;
      newy = new_.second;
      //delete (*i);
      //setting new place devices for each node searching for .
      //new__instance(cmd,NULL,Scope); //cmd : can create. Scope? how to get Scope? Yes!
      const COMPONENT* port = find_place(x, newx, newy);
      string portname = "incomplete";
      if (!port){
         portname = "cn_" + ::to_string(_nodenumber++);
         _placeq.push( portinfo(portname, newx, newy) );
         portname = string(INT_PREFIX) + portname;
      } else {
         portname = port->port_value(0);
      }
      // port_by_name?!
      try{
         string p=i->label();
         trace3("LANG_GEDA::parse_component setting port", i->label(), portname, hp(x));
         x->set_port_by_name(p, portname);
         assert(p==i->label());
      }catch(Exception_No_Match){
         try{
            untested();
            trace2("LANG_GEDA::parse_component by index", i->pinseq(), portname);
            x->set_port_by_index(i->pinseq()-1, portname);
         }catch(Exception_Too_Many){
            // we have checked for pincount!
            unreachable();
         }
      }
      ++index;
   }
   if(source!=""){untested();
      trace1("parse_component", source);
   }
   delete dev;

   trace0("LANG_GEDA::parse_component done");
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_COMMENT* LANG_GEDA::parse_comment(CS& cmd, DEV_COMMENT* x)
{
   assert(x);
   x->set(cmd.fullstring());
   std::string dump,no_of_lines="";
   if (cmd >> "T "){
      _mode=mCOMMENT;
      for(int i=0; i<8; ++i){
         cmd >> dump >> " ";
      }
      cmd>>no_of_lines;
      if(no_of_lines==""){
         _no_of_lines=1;
      }else{
         _no_of_lines=atoi(no_of_lines.c_str());
      }   
   }else{
      if(_no_of_lines!=0){
         --_no_of_lines;
         if(_no_of_lines==0){
            _mode=mATTRIBUTE;
         }
      }
   }
   return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_GEDA::parse_symbol_file( DEV_DOT* x, const GEDA_SYMBOL& sym )const
{
   trace0("LANG_GEDA::parse_symbol_file");
   return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_GEDA::parse_command(CS& cmd, DEV_DOT* x)
{
   std::string component_x, component_y, mirror, angle, dump, basename;
   //too late
   //    cmd >> "C" >> component_x >> " " >> component_y >> " " >> dump
   //        >> " " >> angle >> " " >> mirror >> " " >> basename;
   //too late...
   //           x->set(cmd.fullstring());
   assert(x);
   CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;
   if(_C){
      untested();
      x->set("gC");
      CS c(CS::_STRING, "gC");
      CMD::cmdproc(c, scope );
   } else {
      CMD::cmdproc(cmd, scope );
   }
   return 0;


   assert(_C);
   basename=(*_C)["basename"];
   trace3("LANG_GEDA::parse_command", x->s(), x->owner(), basename);


   bool graphical = 0;
   if(basename.length() > 4 && basename.substr(basename.length()-4) == ".sym"){
      untested();
      //std::vector<std::string*> coord = parse_symbol_file( x, basename );
      parse_symbol_file( x, *_C );
      graphical = !_C->pincount();
   }else{
      cmd.reset();
      CMD::cmdproc(cmd, scope);
      delete x;
      return NULL;

   }
   trace3("LANG_GEDA::parse_command", basename, graphical, x->s());

   // bug: parse_symbol_file needs to tell us, if it is a command
   if( x->s() == "include"
         || x->s() == "end"
         || x->s() == "simulator"
         || x->s() == "directive"
         || x->s() == "gC"
         || x->s() == "list" ){

   }else{
      trace3("LANG_GEDA::parse_command not a command", x->s(), basename, graphical);
      cmd.reset();
      // for now, this is not a command
      untested();
      // cmd.get_line(""); // bug? may fail and abort...

      if (!graphical){
         trace2("LANG_GEDA::parse_command its a dev", x->s(), basename);
         // need command first to create devtype
         CMD::cmdproc(cmd, scope);
         _gotline = 1;
      }
      delete x;
      return NULL;

   }


   try{
      cmd.get_line("");
      trace1("parse_command body?", cmd.fullstring());
   }catch(Exception_End_Of_Input&){
   }

#if 0
   if (cmd.skip1("{")){
      for(;;){
         cmd.get_line("");
         if(cmd >> "}"){
            break;
         }else if ( cmd >> "T" ){
            cmd >> dump;
         }else{
            string pname = cmd.ctos("=","","");
            string pvalue;
            cmd >> "=" >> pvalue;
            if(       pname=="pinlabel"){
            }else if (pname=="pintype"){
            }else if (pname=="file"){
               x->set(x->s() + " " + pvalue);
            }
         }
      }
   }else{
      _gotline = 1;
   }
#endif
   trace1("LANG_GEDA::parse_command instance done", x->s());

   if(0){ // now?
      CMD::cmdproc(cmd, scope);
      delete x;
      return NULL;
   } else {
      untested();
      return x;
   }
}
/*--------------------------------------------------------------------------*/
MODEL_CARD* LANG_GEDA::parse_paramset(CS& cmd, MODEL_CARD* x)
{
   assert(x);
   return NULL;
}
/*--------------------------------------------------------------------------*/
MODEL_SUBCKT* LANG_GEDA::parse_module(CS& cmd, MODEL_SUBCKT* x)
{
   CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();

   int c_x;
   int c_y;
   bool mirror;
   angle_t angle;
   string basename;
   if (!_C) { // parse from cmd...
      incomplete(); // not possible right now
      //cmd>>"C";
      //cmd>>component_x>>" ">>component_y>>" ">>dump>>" ">>angle>>" ">>mirror>>" ">>basename;
   } else {
      if ( !_C->has_key("source") && !_C->has_key("file") ){
         untested();
         return 0;
      }
      *_C >> x;
      c_x = _C->x;
      c_y = _C->y;
      mirror = _C->mirror;
      angle = _C->angle;
      basename = (*_C)["basename"];
      USE(c_x);
      USE(c_y);
      USE(mirror);
      USE(angle);
      USE(basename);
   }

   x->set_label((*_C)["device"]);
   if ( _C->has_key("source")){
      GEDA_SYMBOL* tmp=_C; _C=0;
      read_file( (*tmp)["source"], scope, x);
      _C=tmp;
   } else if ( _C->has_key("file") ){ untested();
      trace1("spice-sdb hack", (*_C)["file"] );
      // file must be a spice deck defining device.
      // just source it and check...
      // then rewire
      read_spice((*_C)["file"], scope, x);
      CARD_LIST::const_iterator i = x->subckt()->find_((*_C)["device"]);
      if(i==scope->end()){
         error(bDANGER,"spice-sdb compat: no %s in %s\n",
               (*_C)["device"].c_str(), (*_C)["file"].c_str());
         untested();
      } else {
         COMPONENT* a=prechecked_cast<COMPONENT*>((*i)->clone_instance());
         assert(a);
         CMD::command("options lang=spice", scope); // still case problems...
         a->set_label("X"+(*_C)["device"]);
         a->set_dev_type((*_C)["device"]);
         a->set_owner(x);

         CMD::command("options lang=gschem", scope);
         (*_C) >> a;
         scope->push_back(a);
      }
   }

   trace5("LANG_GEDA::parse_module", c_x, c_y, mirror, angle, basename);
   return x;
}
/*--------------------------------------------------------------------------*/
// is this really necessary?
COMPONENT* LANG_GEDA::parse_componmod(CS& cmd, COMPONENT* x)
{
   trace1("LANG_GEDA::parse_componmod", cmd.fullstring());
   assert(x);
   assert(_C);
   cmd.reset();
   std::string type = find_type_in_string(cmd);
   trace1("LANG_GEDA::parse_componmod", type);

   // assert(type=="C"); BUG
   // too late
   // cmd>>"C";
   // cmd>>component_x>>" ">>component_y>>" ">>dump>>" ">>angle>>" ">>mirror>>" ">>basename;
   if(!_C->pincount()) return 0;
   int c_x = _C->x;
   int c_y = _C->y;
   bool mirror = _C->mirror;
   angle_t angle = _C->angle;
   string basename = (*_C)["basename"];
   trace5("LANG_GEDA::parse_componmod", c_x, c_y, mirror, angle, basename);

   //open the basename to get the ports and their placements
   //parse_ports(newcmd,x);

   //
   assert(_C->has_key("device"));
   x->set_label(DUMMY_PREFIX+basename);
   // x->set_label((*_C)["device"]);

   // std::vector<std::string*> coord=parse_symbol_file(x,basename);
   MODEL_SUBCKT* m = dynamic_cast<MODEL_SUBCKT*>(x);
   assert(m);
   *_C >> m;
   // move?
   try{
      x->set_param_by_name("x", to_string(c_x));
      x->set_param_by_name("y", to_string(c_y));
      x->set_param_by_name("mirror", to_string(mirror));
      x->set_param_by_name("angle", to_string(angle));
   }catch(Exception_No_Match){
   }

   trace0("parsed symbol file");

   cmd.reset();
   trace0("got out");
   /*type = "graphical";
     x->set_dev_type(type);
     std::cout<<x->dev_type()<<" is the dev type\n";
     */
   // delete _C; // no. keep _C, no instance yet.
   // _C = 0;
   return x;
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_GEDA::parse_instance(CS& cmd, COMPONENT* x)
{
   cmd.reset();
   parse_type(cmd, x); //parse type will parse the component type and set_dev_type
   trace2("parse_instance", cmd.fullstring(), x->dev_type());
   if (x->dev_type()=="net") {
      parse_net(cmd,x);
   }else if(x->dev_type()=="place"){
      parse_place(cmd,x);
   }else {
      parse_component(cmd,x);
   }
   //No warnings needed.
   //cmd.check(bWARNING, "what's ins this?");
   return x;

}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
// this is a hack finding the string in a C block.
// no parse, if _C is present.
GEDA_SYMBOL* LANG_GEDA::parse_C(CS& cmd)const
{
   if (_C){ untested();
      return _C;
   }

   int c_x, c_y, c_a;
   bool c_m, c_sel;
   string basename;
   c_x = cmd.ctoi();
   c_y = cmd.ctoi();
   c_sel = cmd.ctob(); USE(c_sel);
   c_a = cmd.ctoi();
   c_m = cmd.ctob();
   cmd >> " " >> basename;
   assert( ! ( c_a % 90 ) );
   assert( -1 < c_a && c_a < 271 );
   GEDA_SYMBOL sym = _symbol[basename];
   _C = new GEDA_SYMBOL(sym);
   GEDA_SYMBOL& D = *_C;
   D.x = c_x; D.y = c_y;
   D.angle = angle_t( c_a );
   D.mirror = c_m;
   string& s = (*_C)["basename"];
   s = basename; // hmmm...
   try{
      cmd.get_line("gnucap-geda-"+basename+">");
      trace1("parse_C body?", cmd.fullstring());
   }catch(Exception_End_Of_Input&){
   }
   if(cmd >> '{') {
      for (;;) {
         cmd.get_line("gnucap-geda-"+basename+">");
         if (cmd >> "}") {
            cmd.reset();
            break;
         } else if(cmd >> "T") {
         } else {
            string name = cmd.ctos("=","",""), value;
            cmd >> "=" >> value;
            (*_C)[name] = value;
         }
      }
   } else { // "C" without body
      trace1("C w/o body", cmd.fullstring());
      _gotline = 1; // dont read another time.
   }
   assert(_C);
   return _C;
}
/*----------------------------------------------------------------------*/
/* find_type_in_string : specific to each language which gives the type of the
 * cmd Eg: verilog : comment ,resistor etc
 * In Gschem the broad high level types are
 * The following don't mean anything to the electrical part of the circuit and
 * they are of type graphical
 *  version  v    * circle   V
 *  line     L    * arc      A
 *  picture  G    * path     H
 *  box      B    * text     T (not attributes)
 * The following have electrical meaning and are of type
 *  net          N
 *  bus          U
 *  pin          P
 *  component    C
 *  attributes   T (enclosed in {})  // don't include in find_type_in_string
 * Will check for type graphical (type=dev_comment) else type will be
 * net or bus or pin or component\
 */
std::string LANG_GEDA::find_type_in_string(CS& cmd)const
{
   trace2("LANG_GEDA::find_type_in_string", cmd.tail(), hp(_C));
   unsigned here = cmd.cursor(); //store cursor position to reset back later
   std::string type;   //stores type : should check device attribute..
   //graphical=["v","L","G","B","V","A","H","T"]
   if (_placeq.size()){ // hack?
      assert(!_C);
      return "place";
   } else if (_netq.size()){
      assert(!_C);
      return "net";
   } else if (_C || cmd >> "C "){
      const GEDA_SYMBOL D = *(parse_C(cmd));
      assert(_C);
      trace2("find_type_in_string C", hp(_C), D["device"]);

      if (!D.pincount()){
         if ( D.has_key("device") ){
            if ((*_C)["device"] == "directive")
               incomplete();
            // return "some command"
            // don't need non-directive symbol hold directives.
            // (for now?)
         }
         untested();
         trace1("have no pins", _C->pincount());
         delete _C;
         _C = 0;
         return "dev_comment";
      }
      trace1("have pins", _C->pincount());

      if (D.has_key("device")){
         trace1("...", D["device"]);
         assert(D["device"]!="");
         CARD_LIST::const_iterator i = CARD_LIST::card_list.find_(D["device"]);
         if (CARD* c = device_dispatcher[D["device"]]){
            COMPONENT* d = prechecked_cast<COMPONENT*>(c);
            if ( d->max_nodes() >= D.pincount()
                  && d->min_nodes() <= D.pincount()){
               untested();
               type = D["device"];
            }
         } else if (i!= CARD_LIST::card_list.end()){
            if (COMPONENT* d = prechecked_cast<COMPONENT*>(*i))
               if ( d->max_nodes() >= D.pincount()
                     && d->min_nodes() <= D.pincount()){
                  type = D["device"];
               }
            if (MODEL_SUBCKT* d = prechecked_cast<MODEL_SUBCKT*>(*i))
               if ( d->max_nodes() >= D.pincount()
                     && d->min_nodes() <= D.pincount()){
                  type = D["device"];
               }
         } else {
            string modulename = DUMMY_PREFIX + D["basename"];
            trace1("symbolthere?", modulename);
            CARD_LIST::const_iterator i = CARD_LIST::card_list.find_(modulename);
            if(i != CARD_LIST::card_list.end()) {
               trace1("instanceofsymbol", D["basename"]);
               type = modulename;
            } else {
               trace0("C -- new subckt?");
               type = "gC";
            }
         }
      } else if (D.has_key("net")) {
         trace1("...", D["net"]);
         if (CARD* c = device_dispatcher["rail"]){
            COMPONENT* d = prechecked_cast<COMPONENT*>(c);
            if ( d->max_nodes() >= D.pincount()
                  && d->min_nodes() <= D.pincount()){
               type = "rail";
               (*_C)["device"] = type;
            }
         }
      } else {
         untested();
         type = "gC"; // declare module first...
      }
      trace1("find_type_in_string, no reset", type);
      return type;
   } else if (cmd >> "v " || cmd >> "L " || cmd >> "G " || cmd >> "B " || cmd >>"V "
         || cmd >> "A " || cmd >> "H " || cmd >> "T " ){
      return "dev_comment";
   } else if (cmd >> "}"){
      unreachable();
      try {
         cmd.get_line("brace-bug>");
         return find_type_in_string(cmd);
      } catch(Exception_End_Of_Input&){
         untested();
         return "dev_comment";
      }
   } else if (cmd >> "N "){
      return "net";
   } else if (cmd >> "U "){ type="bus";}
   else if (cmd >> "P "){ type="pin";}
   else if (cmd >> "place "){
      // hmmm. ouch
      type="place"; 
   } else {
      switch(_mode){
         case mCOMMENT: return "dev_comment";
         default : cmd >> type;
      }
   }
   trace1("LANG_GEDA::find_type_in_string done", type);
   //Not matched with the type. What now?
   //trace2("find_type_in_string", cmd.fullstring(),type);
   cmd.reset(here);//Reset cursor back to the position that
   //has been started with at beginning
   return type;    // returns the type of the string
}
/*----------------------------------------------------------------------*/
/* parse_top_item :
 * The top default thing that is parsed. Here new__instances are
 * created and (TODO)post processing of nets is done
 */
void LANG_GEDA::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
   parse_item_(cmd, NULL, Scope);
}
/*----------------------------------------------------------------------*/
// check: LANGUAGE::parse_item...
void LANG_GEDA::parse_item_(CS& cmd, CARD* owner, CARD_LIST* scope)const
{

   // .... _gotline means:
   // - component needs to be instanciated after sckt declaration.
   // - parser found nonbrace when trying to parse body
   trace4("LANG_GEDA::parse_item_", _gotline, _placeq.size(), _netq.size(), hp(_C));
   if(!_gotline && !_placeq.size() && !_netq.size() && !_C ){
      cmd.get_line("gnucap-geda>");
   } else if (!_placeq.size() && !_netq.size() && !_C ){
      _gotline = 0;
   }


   //problem: if new__instance interprets as command, Scope is lost.
   trace2("LANG_GEDA::parse_item_", cmd.fullstring(), cmd.tail());
   CARD_LIST* s = (owner) ? owner->subckt() : scope;
   assert(!cmd.match1("{"));

   // nothing if cmd.is_end...
   lang_geda.new__instance(cmd, dynamic_cast<MODEL_SUBCKT*>(owner), s);
}
/*----------------------------------------------------------------------*/
// Code for Printing schematic follows
/*----------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void print_type(OMSTREAM& o, const COMPONENT* x)
{
   assert(x);
   o << x->dev_type();
}
/*--------------------------------------------------------------------------*/
static void print_label(OMSTREAM& o, const COMPONENT* x)
{
   assert(x);
   o << x->short_label();
}
/*--------------------------------------------------------------------------*/
static void print_node_xy(OMSTREAM& o,const COMPONENT* x,int _portindex)
{
   std::string _nodename=x->port_value(_portindex);
   for(CARD_LIST::const_iterator ci=x->scope()->begin(); ci!=x->scope()->end(); ++ci) {
      if((*ci)->dev_type()=="place"){
         if(static_cast<COMPONENT*>(*ci)->port_value(0)==_nodename){
            trace0("Got some place!");
            o << (*ci)->param_value(1) << " " << (*ci)->param_value(0) << " ";
         }
      }
   }
}
/*--------------------------------------------------------------------------*/
pair<int,int> componentposition(int* absxy, int* delxy, int angle, bool mirror)
{
   int newx = absxy[0];
   int newy = absxy[1];
   //if(mirror=="0")
   if(!mirror){
      switch(angle){
         case 0:
            newx -= delxy[0];
            newy -= delxy[1];
            break;
         case 90:
            newx += delxy[1];
            newy -= delxy[0];
            break;
         case 180:
            newx += delxy[0];
            newy += delxy[1];
            break;
         case 270:
            newx -= delxy[1];
            newy += delxy[0];
            break;
      }
   } else if(mirror){
      switch(angle){
         case 0:
            newx += delxy[0];
            newy -= delxy[1];
            break;
         case 90:
            newx += delxy[1];
            newy += delxy[0];
            break;
         case 180:
            newx -= delxy[0];
            newy += delxy[1];
            break;
         case 270:
            newx -= delxy[1];
            newy -= delxy[0];
            break;
      }
   }else{
      //not correct mirror!
   }
   return pair<int,int>(newx,newy);
}
/*--------------------------------------------------------------------------*/
// urghs
static std::string componentposition_string(int* absxy, int* relxy, int angle, bool mirror)
{
   return to_string(componentposition(absxy, relxy, angle, mirror).first)
      + " " +
      to_string(componentposition(absxy, relxy, angle, mirror).second);
}
/*--------------------------------------------------------------------------*/
// can this be done faster?!
const place::DEV_PLACE* LANG_GEDA::find_place(const CARD* x, string name)const
{
   const CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();
   for(CARD_LIST::const_iterator ci=scope->begin(); ci!=scope->end(); ++ci) {
      if(const place::DEV_PLACE*p=dynamic_cast<const place::DEV_PLACE*>(*ci)){
         if(p->port_value(0)==name){
            return p;
         }
      }
   }
   throw(Exception_Cant_Find("place",name));
}
/*--------------------------------------------------------------------------*/
pair<int,int> LANG_GEDA::find_place_(const CARD* x, string name)const
{
   const place::DEV_PLACE*p = find_place(x,name);
   assert(p);
   pair<int,int> a;
   a.first = p->x();
   a.second = p->y();
   trace2("LANG_GEDA::find_place_", p->x(), p->y());
   return a;
}
/*--------------------------------------------------------------------------*/
string* LANG_GEDA::find_place_string(const CARD* x, std::string name)const
{
   std::string* a = new std::string[2];
   try{
      pair<int,int> b = find_place_(x, name);
      a[0] = to_string(b.first);
      a[1] = to_string(b.second);
      return a;
   }catch(Exception_Cant_Find){
      return NULL;
   }
}
/*--------------------------------------------------------------------------*/
static void print_net(OMSTREAM& o, const COMPONENT* x)
{
   assert(x);
   assert(x->dev_type()=="net");
   o << "N ";
   //print_coordinates(node); //Will search through the CARD_LIST to find the node
   //o<< node0x << node1x
   //o<< node1x << node2x
   print_node_xy(o,x,0);
   print_node_xy(o,x,1);
   if(x->value().string()!=""){
      o  << x->value().string()<<"\n"; //The color
   }else{
      o << "4\n";
   }
}
/*--------------------------------------------------------------------------*/
/* C x y selectable angle mirror basename
 * {
 *  <params>
 * }
 */
void LANG_GEDA::print_component(OMSTREAM& o, const COMPONENT* x)
{
   assert(x);
   trace0("LANG_GEDA::print_component");
   std::string _x,_y,_angle,_mirror;
   o << "gC";
   std::string _basename=x->param_value(x->param_count()-1);
   //go through the symbol file and get the relative pin positions
   std::vector<std::string*> coordinates=parse_symbol_file(NULL, _basename);
   std::vector<std::string*> abscoord;
   for(unsigned ii=0; ii<coordinates.size(); ++ii){
      abscoord.push_back(find_place_string(x,x->port_value(ii)));
   }
   std::string angle[4]={"0","90","180","270"};
   std::string xy="";
   bool gottheanglemirror=false;
   for(int ii=0; ii<4 ; ++ii){
      if(not gottheanglemirror){
         _mirror="0";
         _angle=angle[ii];
         xy="";
         gottheanglemirror=true;
         for(unsigned pinind=0; pinind<coordinates.size(); ++pinind){
            int a[2];
            int c[2];
            a[0] = atoi(abscoord[pinind][0].c_str());
            a[1] = atoi(abscoord[pinind][1].c_str());
            c[0] = atoi(coordinates[pinind][0].c_str());
            c[1] = atoi(coordinates[pinind][1].c_str());
            if (xy==""){
               xy = componentposition_string(a, c, 90*ii, _mirror=="1");
            }else if(xy != componentposition_string(a, c, 90*ii, _mirror=="1")){
               gottheanglemirror=false;
               break;
            }
         }
         if (gottheanglemirror) {
            o << xy << " " << "1" << " " << _angle << " " << _mirror << " " << _basename<< "\n";
         } else {
            _mirror="1";
            xy="";
            gottheanglemirror=true;
            for(unsigned pinind=0; pinind<coordinates.size(); ++pinind){
               int a[2];
               int c[2];
               a[0] = atoi(abscoord[pinind][0].c_str());
               a[1] = atoi(abscoord[pinind][1].c_str());
               c[0] = atoi(coordinates[pinind][0].c_str());
               c[1] = atoi(coordinates[pinind][1].c_str());
               if (xy==""){
                  xy = componentposition_string(a, c, 90*ii, _mirror=="1");
               }else if(xy != componentposition_string(a, c, 90*ii, _mirror=="1")){
                  gottheanglemirror=false;
                  break;
               }
            }
            if (gottheanglemirror) {
               o << xy << " " << "1" << " " << _angle << " " << _mirror << " " << _basename<< "\n";
            }
         }
      }
   }
   //map those with the absolute positions of nodes and place the device
   //such that it is in between the nodes.
   //std::vector<std::string*> coord = parse_symbol_file(static_cast<COMPONENT*>(x) , _basename);

   //Got the x and y
   //To print angle mirror etc, to get from the intelligent positioning

   //*To check if there are any attributes at all first
   //    If not return;
   bool _parameters=false;
   bool _label=false;
   bool _devtype=false;
   if(x->short_label()!=""){
      _label=true;
   }
   if(x->param_count()>6){
      _parameters=true;
   }
   if(x->dev_type()!="" && x->dev_type().substr(0,DUMMY_PREFIX.length())!=DUMMY_PREFIX){
      _devtype=true;
   }
   if (_label or _parameters or _devtype){
      o << "{\n";
      if(_devtype){
         o << "T "<< xy << " 5 10 0 1 0 0 1\n";
         o << "device=" << x->dev_type() << "\n";
      }
      if(_label){
         o << "T "<< xy << " 5 10 0 1 0 0 1\n";
         o << "refdes=" << x->short_label() << "\n";
      }
      if(_parameters){
         untested();
         for(int i=x->param_count()-1; i>=0 ; --i){
            if (!(x->param_value(i)=="NA( 0.)" or x->param_value(i)=="NA( NA)" or x->param_value(i)=="NA( 27.)" or x->param_name(i)=="basename")){
               o << "T "<< xy << " 5 10 0 1 0 0 1\n";
               o << x->param_name(i) << "=" << x->param_value(i) << "\n";
            }
         }
      }
      o << "}\n";
   }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_GEDA::print_paramset(OMSTREAM& o, const MODEL_CARD* x)
{
   trace0("Got into print paramset");
   return;
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::print_module(OMSTREAM& o, const MODEL_SUBCKT* x)
{
   assert(x);
   //o<<x->short_label();
   //o<<"\n";
   assert(x->subckt());
   if(x->short_label().find(DUMMY_PREFIX)!=std::string::npos){
      trace0("Got a placeholding model");
      return;
   }
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::print_instance(OMSTREAM& o, const COMPONENT* x)
{
   trace0("Got into print_instance in geda!");
   // print_type(o, x);
   // print_label(o, x);
   if(x->dev_type()=="net"){
      print_net(o, x);
   }
   else if(x->dev_type()=="place"){
   }else{
      //Component
      print_component(o ,x);
   }
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::print_comment(OMSTREAM& o, const DEV_COMMENT* x)
{
   assert(x);
   o << x->comment() << '\n';
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::print_command(OMSTREAM& o, const DEV_DOT* x)
{
   assert(x);
   o << x->s() << '\n';
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
class CMD_GSCHEM : public CMD {
   public:
      void do_it(CS& cmd, CARD_LIST* Scope)
      {
         string oldlang = OPT::language->name();
         command("options lang=gschem", Scope);
         // BUG breaks direct "options lang=gschem", does it?
         lang_geda._mode=lang_geda.mATTRIBUTE;
         lang_geda._no_of_lines=0;
         lang_geda._gotline=false;
         //
         string filename;
         cmd >> filename;
         trace1("gschem", filename);
         if(filename==""){
            return;
         }
         bool module = 0;
         bool symbol = 0;
         string device = "";
         string source = "";
         trace1("args", cmd.tail());
         unsigned here = cmd.cursor();
         do{
            ONE_OF
               || Get(cmd, "module", &module)
               || Get(cmd, "symbol", &symbol)
               || (cmd.umatch("device {=}") && ( cmd >> device ) )
               || (cmd.umatch("source {=}") && ( cmd >> source ) ) ;
         } while (cmd.more() && !cmd.stuck(&here));

         MODEL_SUBCKT* model = 0;
         string label = (device=="")? filename : device;

         if( symbol ){
            GEDA_SYMBOL sym = GEDA_SYMBOL(lang_geda._symbol[filename]);
            trace3("symbol", sym.pincount(), filename, lang_geda._symbol[filename].pincount());
            assert(sym.pincount());
            if(source!="") sym["source"] = source;
            if(sym["source"]==""){
               command("options lang="+oldlang, Scope);
               throw Exception_CS("empty source", cmd);
            }
            model = new MODEL_SUBCKT();
            sym >> model;
            model->set_label(label);
            LANG_GEDA::read_file(source, Scope, model);
            //align(model); // might be needed for gnucap .36
            Scope->push_back(model);
         }else if( module ) {
            trace1("reading module", hp(Scope));
            model = new MODEL_SUBCKT();
            model->set_label(label);
            LANG_GEDA::read_file(filename, Scope, model);
            //align(model); // might be needed for gnucap .36
            Scope->push_back(model);
         } else if(filename!=""){
            LANG_GEDA::read_file(filename, Scope);
         } else {
            unreachable();
         }
         trace1("done", oldlang);
         command("options lang="+oldlang, Scope);
      }
   private:
      // void align(MODEL_SUBCKT* s) const;
} p8;
/*----------------------------------------------------------------------*/
// fixme.
void LANG_GEDA::read_spice(string f, CARD_LIST* Scope, MODEL_SUBCKT* owner)
{
   CS cmd(CS::_INC_FILE, f);
   CMD::command("options lang=spice", Scope);

   try{
      for(;;){
         cmd.get_line("spice-sdb>");
         OPT::language->new__instance(cmd, owner, Scope);
      }

   }catch (Exception_End_Of_Input& e){
   }
   CMD::command("options lang=gschem", Scope);
}
/*----------------------------------------------------------------------*/
void LANG_GEDA::read_file(string f, CARD_LIST* Scope, MODEL_SUBCKT* owner)
{
   CS cmd(CS::_INC_FILE, f);

   /// gnucap-uf bug: getline uses OPT::language
   LANGUAGE* oldlang = OPT::language;
   OPT::language = &lang_geda;
   ///

   try{
      for(;;){
         // new__instance. but _gotline hack
         lang_geda.parse_item_(cmd, owner, Scope);
      }

   }catch (Exception_End_Of_Input& e){
   }
   /// gnucap-uf bug
   OPT::language = oldlang;
   ///
}
/*----------------------------------------------------------------------*/
DISPATCHER<CMD>::INSTALL
d8(&command_dispatcher, "gschem|v ", &p8);
/*----------------------------------------------------------------------*/
class CMD_C : public CMD {
   void do_it(CS& cmd, CARD_LIST* Scope)
   {
      trace1("CMD_C::do_it", hp(Scope));
      CARD* c = device_dispatcher["symbol"]; // future overrides?
      if(!c) c = device_dispatcher["subckt"];
      assert(c);
      CARD* clone = c->clone();
      COMPONENT* new_compon = prechecked_cast<COMPONENT*>(clone);

      assert(new_compon);
      assert(!new_compon->owner());
      assert(new_compon->subckt());
      assert(new_compon->subckt()->is_empty());
      // BUG?: new_compon doesnt know its scope!
      // thats okay, symbols are global anyway.
      if (lang_geda.parse_module(cmd, dynamic_cast<MODEL_SUBCKT*>(new_compon))) {
         untested();

         // Scope->push_back(new_compon);
         // bug. what's the scope?!
         CARD_LIST::card_list.push_back(new_compon);
         lang_geda._gotline = true;
         cmd.reset();
      } else if (lang_geda.parse_componmod(cmd, new_compon)) {
         // this is not graphical
         lang_geda._componentname=new_compon->short_label();
         trace2("do_it, componmod", lang_geda._componentname, cmd.fullstring());
         CARD_LIST::const_iterator i = Scope->find_(new_compon->short_label());
         if (i != Scope->end()) {
            trace2(" componmod already there.", lang_geda._componentname, cmd.fullstring());
            untested();
            // i'm here, because the "C" command has type "C" first....
            // hmm maybe it should look for the symbol first, as it might be there already.
            delete clone;
         }else{
            trace2("CMD_C pushback", hp(Scope), new_compon->long_label());
            // BUG? global scope
            CARD_LIST::card_list.push_back(new_compon);
         }

         cmd.reset();
         trace1("not calling new__instance ", cmd.fullstring());
         lang_geda._gotline = true;
      } else {
         unreachable();
         untested();
         delete clone;
      }

   }
} p2;
DISPATCHER<CMD>::INSTALL
d2(&command_dispatcher, "gC", &p2);

/*----------------------------------------------------------------------*/
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
