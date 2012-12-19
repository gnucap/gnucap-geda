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
#include "lang_geda.h"
#undef COMPLEX
/*--------------------------------------------------------------------------*/
// using namespace std;
using std::string;
using std::vector;
using std::pair;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class LANG_GEDA : public LANGUAGE {
    friend class CMD_GSCHEM;
    TOPLEVEL* pr_current;

    typedef struct {
        string name;
        int x,y;
    } portinfo;
    mutable std::queue<portinfo> _placeq;
    typedef struct {
        int x0, y0, x1, y1;
        unsigned color;
    } netinfo;
    mutable std::queue<netinfo> _netq;
    mutable GEDA_SYMBOL* _dev;
public:
    LANG_GEDA() : LANGUAGE(), _dev(NULL){
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

    void create_place(string name, string x, string y, COMPONENT* c)const;
    void parse_component(CS& cmd,COMPONENT* x);
    std::vector<std::string*> parse_symbol_file(CARD* x, std::string basename) const;
    COMPONENT* findplace(COMPONENT* x, int xco, int yco)const;
    COMPONENT* findplace(COMPONENT* x, std::string xco, std::string yco)const;
    pair<int,int>* findnode(CARD *x, int x0, int y0, int x1, int y1)const;

    static GEDA_SYMBOL_MAP _symbol;
    static unsigned _nodenumber;
    static unsigned _netnumber;

}lang_geda;

DISPATCHER<LANGUAGE>::INSTALL
    d(&language_dispatcher, lang_geda.name(), &lang_geda);
/*----------------------------------------------------------------------*/
GEDA_SYMBOL_MAP LANG_GEDA::_symbol;
/*----------------------------------------------------------------------*/
unsigned LANG_GEDA::_netnumber, LANG_GEDA::_nodenumber;
/*----------------------------------------------------------------------*/
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
    assert(!_dev);
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

    trace2("LANG_GEDA::parse_symbol_file", basename, filename);
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
        if (x && x->short_label()=="!_"+basename){
            ismodel=true;
        }
        if (linetype=="dev_comment"){
            // nop
        }else if (linetype=="pin" && (c || !x)){
            trace2("parse_symbol_file parsing pin", basename, sym_cmd.fullstring());
            coord.push_back(parse_pin(sym_cmd,c,index++,ismodel));
            trace2("parse_symbol_file pin done", basename, sym_cmd.fullstring());
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
            }else if(c = dynamic_cast<COMPONENT*>(x)){
                // c->set_label(dump);
            }else{
                incomplete();
            }
        }else if(c && linetype != "" ){
            sym_cmd>>(linetype + "=").c_str();
            sym_cmd>>dump;
            try {
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
    if (cmd.umatch("place")){
        incomplete();
        cmd>>"place";
        std::string _portname,_x,_y;
        cmd>>" ">>_portname>>" ">>_x>>" ">>_y;
        x->set_param_by_name("x",_x);
        x->set_param_by_name("y",_y);
        string portname = string(INT_PREFIX) + "np_" + _portname;
        x->set_port_by_index(0, portname);
    } else if(cmd.umatch("N ") || cmd.umatch("C ") || cmd.match1('}')){
        assert(_placeq.size());
        portinfo p = _placeq.front();
        x->set_param_by_name("x", to_string(p.x));
        x->set_param_by_name("y", to_string(p.y));
        string portname = string(INT_PREFIX) + p.name;
        x->set_port_by_index(0, portname);
        _placeq.pop();
        if (_placeq.size()){
            cmd.reset();
        }
        cmd.reset();
    } else {
        trace1("parse_place, huh?", cmd.fullstring());
        unreachable();
    }
}
/*--------------------------------------------------------------------------*/
void LANG_GEDA::create_place(string n, string x, string y, COMPONENT* c)const
{
    unreachable();
    trace1("create_place", n);
    MODEL_SUBCKT* owner = dynamic_cast<MODEL_SUBCKT*>(c->owner());
    CARD_LIST* scope = owner?owner->scope():c->scope();
    assert(c);
    string cmdstr = "place "+n+" "+x+" "+y;
    CS place_cmd(CS::_STRING,cmdstr);
    // BUG? new__instance is not const...
    // problem: c->scope()==NULL
    // why?
    // create_place is called from parse_net.
    // from parse_instance.
    // from new__instance( net ... )
    //
    // but: new__instance doesnt pass the scope to parse_instance.
    lang_geda.new__instance(place_cmd, owner, scope);
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_GEDA::findplace(COMPONENT* x, int xco, int yco)const
{
    return findplace(x, to_string(xco), to_string(yco));
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_GEDA::findplace(COMPONENT* x, string xco, string yco)const
{
    CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();
    for (CARD_LIST::const_iterator ci = scope->begin(); ci != scope->end(); ++ci) {
        if((*ci)->dev_type()=="place"){
            if(xco==(*ci)->param_value(1) && yco==(*ci)->param_value(0)){
                return static_cast<COMPONENT*>(*ci); // ->port_value(0);
            }
        }
    }
    return NULL;
}
/*--------------------------------------------------------------------------*/
pair<int,int>* LANG_GEDA::findnode(CARD *x, int x0, int y0, int x1, int y1)const
{
    CARD_LIST* scope = x->owner()?x->owner()->scope():x->scope();
    for(CARD_LIST::const_iterator ci = scope->begin(); ci != scope->end(); ++ci) {
        if((*ci)->dev_type()=="place"){
            int _x = atoi((*ci)->param_value(1).c_str());
            int _y = atoi((*ci)->param_value(0).c_str());
            if (y0==y1){
                if((((x1 < _x) && (_x<x0) ) || ((x0<_x) && (_x<x1))) && _y==y0 && _x!=x0 && _x!=x1){
                    trace0("true0");
                    pair<int,int>* coord = new pair<int,int>;
                    coord->first = _x;
                    coord->second = _y;
                    untested();
                    return coord;
                }
                else{
                    return NULL;
                }
            }else if (x0==x1){
                if((((y1 < _y) && (_y<y0)) || ((y0 < _y) && (_y<y1))) && _x==x0 && _y!=y0 && _y!=y1){
                    trace0("true1");
                    pair<int,int>* coord = new pair<int,int>;
                    coord->first = _x;
                    coord->second = _y;
                    untested();
                    return coord;
                }else{
                    return NULL;
                }
            }
            else{
                return NULL;
            }
        }
    }
    return 0;
}
/*--------------------------------------------------------------------------*/
// A net is in form N x0 y0 x1 y1 c
// Need to get x0 y0 ; x1 y1 ;
// Need to go through all the nets. Anyway?
// Need to save them in other forms? How to go through all cards?
// Need to specify a name for a card?
void LANG_GEDA::parse_net(CS& cmd, COMPONENT* x)const
{
    trace1("LANG_GEDA::parse_net", hp(x->scope()));
    MODEL_SUBCKT* owner = dynamic_cast<MODEL_SUBCKT*>(x->owner());
    assert(x);
    // assert(lang_geda.find_type_in_string(cmd)=="net"); // no. at end of body...
    int coord[4];
    if(_netq.size()){
        netinfo n = _netq.front();
        coord[0] = n.x0;
        coord[1] = n.y0;
        coord[2] = n.x1;
        coord[3] = n.y1;
        x->set_param_by_name("color", to_string(n.color));
        _netq.pop();
        x->set_label("extranet"+::to_string(_netnumber++));
    } else {
        cmd>>"N";     //Got N
        unsigned here=cmd.cursor();
        // x0 y0 x1 y1 color
        std::string parsedvalue[5];
        int i=0;
        bool gotthenet=true;
        while (i<5) {
            if (cmd.is_alnum()){
                cmd>>" ">>parsedvalue[i];
                if(i!=4) coord[i] = atoi(parsedvalue[i].c_str());
            }else{
                gotthenet=false;
                cmd.warn(bDANGER, here, x->long_label() +": Not correct format for net");
                return; // throw?
                break;
            }
            ++i;
        }
        x->set_param_by_name("color",parsedvalue[4]);
        x->set_label("net"+::to_string(_netnumber++));
    }

    COMPONENT* port = findplace(x, coord[0], coord[1]);
    string portname;
    if(!port){
        portname = "nn_" + ::to_string(_nodenumber++);
        _placeq.push( portinfo{portname, coord[0], coord[1]} );
        portname = string(INT_PREFIX) + portname;
    } else {
        portname = port->port_value(0);
    }
    x->set_port_by_index(0, portname);

    port = findplace(x, coord[2], coord[3]);
    if(!port){
        untested();
        portname = "nn_" + ::to_string(_nodenumber++);
        _placeq.push( portinfo{portname, coord[2], coord[3]});
        portname = string(INT_PREFIX) + portname;
    } else {
        portname = port->port_value(0);
    }
    x->set_port_by_index(1, portname);

    pair<int,int>* nodeonthisnet = findnode(x,coord[0],coord[1],coord[2],coord[3]);
    if (nodeonthisnet) {
        trace2("nodeonthisnet", nodeonthisnet->first, nodeonthisnet->second);
        //create new net from nodeonthisnet to one of edges of net.
//        string netcmdstr="N "+parsedvalue[0]+" "+parsedvalue[1]+" "
  //                           +nodeonthisnet->first+" "+nodeonthisnet.second+" 5";
        unsigned col = 5;
        _netq.push( netinfo{ coord[0], coord[1], nodeonthisnet->first, nodeonthisnet->second, col });
        delete nodeonthisnet;
        incomplete();
        cmd.reset();
    }
    if(_placeq.size()){
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
                        x->set_param_by_name(_paramname,_paramvalue);
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
    assert(_dev);
    int c_x, c_y, angle;
    bool mirror;
    string dump,basename;
    std::string type=lang_geda.find_type_in_string(cmd);
    GEDA_SYMBOL* dev = _dev;
    _dev = 0; // to make parse_symbol_file work
    assert(type==(*dev)["device"]);
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
    int index = 0;

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

    // connect ports
    for (std::vector<std::string*>::const_iterator i=coordinates.begin();i<coordinates.end();++i){
        int cc[2];
        cc[0] = c_x;
        cc[1] = c_y;
        int delta[2];
        delta[0] = - atoi((*i)[0].c_str());
        delta[1] = - atoi((*i)[1].c_str());
        pair<int,int> new_ = componentposition( cc, delta, angle, mirror );
        newx = new_.first;
        newy = new_.second;
        //delete (*i);
        //setting new place devices for each node searching for .
        //new__instance(cmd,NULL,Scope); //cmd : can create. Scope? how to get Scope? Yes!
        COMPONENT* port = findplace(x, newx, newy);
        string portname = "incomplete";
        if (!port){
            portname = "cn_" + ::to_string(_nodenumber++);
            _placeq.push( portinfo{portname, newx, newy} );
            portname = string(INT_PREFIX) + portname;
        } else {
            portname = port->port_value(0);
        }
        x->set_port_by_index(index, portname);
        trace4("LANG_GEDA::parse_component setting port", x->long_label(), index, portname, x->n_(index).e_());
        ++index;
    }
    if(_placeq.size()){
        cmd.reset();
    }
    static unsigned instance;
    if(x->short_label()==""){
        x->set_label(basename+"_"+to_string(instance++));
    }
    if(source!=""){untested();
        trace1("parse_component", source);
    }
    if(_placeq.size()){
        cmd.reset();
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
DEV_DOT* LANG_GEDA::parse_command(CS& cmd, DEV_DOT* x)
{
    trace2("LANG_GEDA::parse_command", cmd.fullstring(), x->owner());
    std::string component_x, component_y, mirror, angle, dump, basename;
    cmd >> "C" >> component_x >> " " >> component_y >> " " >> dump
        >> " " >> angle >> " " >> mirror >> " " >> basename;

    assert(x);
    CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;

    bool graphical = 0;
    if(basename.length() > 4 && basename.substr(basename.length()-4) == ".sym"){
        untested();
        std::vector<std::string*> coord = parse_symbol_file( x, basename );
        graphical = !coord.size();
    }else{
        cmd.reset();
        CMD::cmdproc(cmd, scope);
        delete x;
        return NULL;

    }
    trace3("LANG_GEDA::parse_command", cmd.fullstring(), basename, graphical);

    // bug: parse_symbol_file needs to tell us, if it is a command
    if( x->s() == "include"
            || x->s() == "end"
            || x->s() == "simulator"
            || x->s() == "directive"
            || x->s() == "C"
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
        trace1("cmd body?", cmd.fullstring());
    }catch(Exception_End_Of_Input&){
    }

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
  //To parse heirarchical schematics
  return NULL;
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_GEDA::parse_componmod(CS& cmd, COMPONENT* x)
{
    trace1("LANG_GEDA::parse_componmod", cmd.fullstring());
    assert(x);
    cmd.reset();
    std::string type = find_type_in_string(cmd);
    trace1("LANG_GEDA::parse_componmod", type);
    assert(type=="C");
    std::string component_x, component_y, mirror, angle, dump,basename;
    cmd>>"C";
    cmd>>component_x>>" ">>component_y>>" ">>dump>>" ">>angle>>" ">>mirror>>" ">>basename;
    trace0("got params");
    //open the basename to get the ports and their placements
    //parse_ports(newcmd,x);
    x->set_label("!_"+basename);
    trace0("set labels");
    std::vector<std::string*> coord=parse_symbol_file(x,basename);
    trace0("parsed symbol file");
    if(coord.size()==0){
        //graphical
        return NULL;
    }

    cmd.reset();
    trace0("got out");
    /*type = "graphical";
    x->set_dev_type(type);
    std::cout<<x->dev_type()<<" is the dev type\n";
    */
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
    trace2("LANG_GEDA::find_type_in_string", cmd.fullstring(), hp(_dev));
    unsigned here = cmd.cursor(); //store cursor position to reset back later
    std::string type;   //stores type : should check device attribute..
    //graphical=["v","L","G","B","V","A","H","T"]
    if(_dev) { untested();
        assert( (*_dev)["basename"] != "" );
        assert( (*_dev)["device"] != "" );
        return (*_dev)["device"];
    }
    if (_placeq.size()){ // hack?
        return "place";
    }
    if (_netq.size()){
        return "net";
    }
    if (cmd >> "v " || cmd >> "L " || cmd >> "G " || cmd >> "B " || cmd >>"V "
        || cmd >> "A " || cmd >> "H " || cmd >> "T " ){
        return "dev_comment";
    }
    if (cmd >> "}"){
        unreachable();
        try {
            cmd.get_line("brace-bug>");
            return find_type_in_string(cmd);
        } catch(Exception_End_Of_Input&){
            untested();
            return "dev_comment";
        }
    }
    if (cmd >> "N "){
        return "net";
    }
    trace1("LANG_GEDA::find_type_in_string II", cmd.fullstring());

    if (cmd >> "U "){ type="bus";}
    else if (cmd >> "P "){ type="pin";}
    else if (cmd >> "C "){
        int c_x, c_y, c_a;
        bool c_m, c_sel;
        string basename;
        c_x = cmd.ctoi();
        c_y = cmd.ctoi();
        c_sel = cmd.ctob();
        c_a = cmd.ctoi();
        c_m = cmd.ctob();
        cmd >> " " >> basename;
        assert( ! ( c_a % 90 ) );
        assert( -1 < c_a && c_a < 271 );
        GEDA_SYMBOL sym = _symbol[basename];
        if (!sym.pincount()){
            if ( sym.has_key("device") ){
                if (sym["device"] == "directive")
                    incomplete();
                // return "some command"
                // don't need non-directive symbol hold directives.
                // (for now?)
            }
            untested();
            return "dev_comment";
        }
        GEDA_SYMBOL* sd = new GEDA_SYMBOL(sym);
        GEDA_SYMBOL& D = *sd;
        D.x = c_x; D.y = c_y;
        D.angle = angle_t( c_a );
        D.mirror = c_m;
        D["basename"] = basename; // hmmm...
        try{
            cmd.get_line("gnucap-geda-"+basename+">");
            trace1("body?", cmd.fullstring());
        }catch(Exception_End_Of_Input&){
        }
        if(cmd >> '{') {
            for (;;) {
                cmd.get_line("gnucap-geda-"+basename+">");
                if (cmd >> "}") {
                    break;
                } else if(cmd >> "T") {
                } else {
                    string name = cmd.ctos("=","",""), value;
                    cmd >> "=" >> value;
                    D[name] = value;
                }
            }
        } else { // "C" without body
            trace1("C w/o body", cmd.fullstring());
            _gotline = 1; // dont read another time.
        }
        if (D.has_key("device")){
            CARD_LIST::const_iterator i = CARD_LIST::card_list.find_(D["device"]);
            if (CARD* c = device_dispatcher[D["device"]]){
                COMPONENT* d = prechecked_cast<COMPONENT*>(c);
                if ( d->max_nodes() >= D.pincount()
                        && d->min_nodes() <= D.pincount()){
                    untested();
                    type = D["device"];
                    _dev = sd;
                }
            } else if (i!= CARD_LIST::card_list.end()){
                if (COMPONENT* d = prechecked_cast<COMPONENT*>(*i))
                    if ( d->max_nodes() >= D.pincount()
                            && d->min_nodes() <= D.pincount()){
                        untested();
                        type = D["device"];
                        _dev = sd;
                    }
                if (MODEL_SUBCKT* d = prechecked_cast<MODEL_SUBCKT*>(*i))
                    if ( d->max_nodes() >= D.pincount()
                            && d->min_nodes() <= D.pincount()){
                        untested();
                        type = D["device"];
                        _dev = sd;
                    }
            }
        } else if (D.has_key("net")) {
            if (CARD* c = device_dispatcher["rail"]){
                COMPONENT* d = prechecked_cast<COMPONENT*>(c);
                if ( d->max_nodes() >= D.pincount()
                        && d->min_nodes() <= D.pincount()){
                    type = "rail";
                    _dev = sd;
                    D["device"] = type;
                }
            }
        } else {
            untested();
            type = "C"; // declare module first...
            _dev = sd;
        }
    } else if (cmd >> "place "){ 
    // hmmm. ouch
        type="place"; 
    } else {
        switch(_mode){
            case mCOMMENT: return "dev_comment";
            default : cmd >> type;
        } 
    } //Not matched with the type. What now?
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
    trace4("LANG_GEDA::parse_item_", _gotline, _placeq.size(), _netq.size(), hp(_dev));
    if(!_gotline && !_placeq.size() && !_netq.size() ){
        cmd.get_line("gnucap-geda>");
    } else if (!_placeq.size() && !_netq.size() ){
        _gotline = 0;
    }


    //problem: if new__instance interprets as command, Scope is lost.
    trace1("LANG_GEDA::parse_item_", cmd.fullstring());
    CARD_LIST* s = (owner) ? owner->subckt() : scope;
    assert(!cmd.match1("{"));

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
static std::string* find_place(const COMPONENT* x, std::string _portvalue)
{
    for(CARD_LIST::const_iterator ci=x->scope()->begin(); ci!=x->scope()->end(); ++ci) {
        if((*ci)->dev_type()=="place"){
            if(static_cast<COMPONENT*>(*ci)->port_value(0)==_portvalue){
                trace0("Got the coord");
                std::string*  a= new std::string[2];
                a[0]=(*ci)->param_value(1);     
                a[1]=(*ci)->param_value(0);
                trace0("Got out of find_place");
                return a;
            }
        }
    }
    return NULL;
    //To return place coordinates after searching for it.
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
    o <<  "C ";
    std::string _basename=x->param_value(x->param_count()-1);
    //go through the symbol file and get the relative pin positions
    std::vector<std::string*> coordinates=parse_symbol_file(NULL, _basename);
    std::vector<std::string*> abscoord;
    for(int ii=0; ii<coordinates.size(); ++ii){
        abscoord.push_back(find_place(x,x->port_value(ii)));
    }
    std::string angle[4]={"0","90","180","270"};
    int index=0; 
    std::string xy="";
    bool gottheanglemirror=false;
    for(int ii=0; ii<4 ; ++ii){
        if(not gottheanglemirror){
            _mirror="0";
            _angle=angle[ii];
            xy="";
            gottheanglemirror=true;
            for(int pinind=0; pinind<coordinates.size(); ++pinind){
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
                for(int pinind=0; pinind<coordinates.size(); ++pinind){
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
    if(x->dev_type()!="" and (x->dev_type()).substr(0,2)!="!_"){
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
  if(x->short_label().find("!_")!=std::string::npos){
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
      
      // BUG breaks direct "options lang=gschem", does it?
      lang_geda._mode=lang_geda.mATTRIBUTE;
      lang_geda._no_of_lines=0;
      lang_geda._gotline=false;
      //
      
      cmd.reset();

      string arg="";
      
      cmd >> arg;
      if(arg=="gschem"){
          cmd >> arg;
      } else {
          arg="";
      }
      MODEL_SUBCKT* model = 0;

      if(arg!=""){
          trace1("reading file", hp(Scope));
          string mod;
          cmd >> mod;
          if( mod == "module"){
              model = new MODEL_SUBCKT();
              COMPONENT* x = model;
              model->set_label(arg);
              read_file(arg, Scope, model);
              //align(model); // might be needed for gnucap .36
              Scope->push_back(model);
          } else {
              read_file(arg, Scope);
          }

      }else{
          command("options lang=gschem", Scope);
      }
    }
private:
    void read_file(string, CARD_LIST* Scope, MODEL_SUBCKT* owner=0);
    // void align(MODEL_SUBCKT* s) const;
} p8;
/*----------------------------------------------------------------------*/
void CMD_GSCHEM::read_file(string f, CARD_LIST* Scope, MODEL_SUBCKT* owner)
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
    d8(&command_dispatcher, "gschem|v", &p8);
/*----------------------------------------------------------------------*/
class CMD_C : public CMD {
    void do_it(CS& cmd, CARD_LIST* Scope)
    {
      trace1("CMD_C::do_it", hp(Scope));
      CARD* c = device_dispatcher["symbol"];
      if(!c) c = device_dispatcher["subckt"]; // fallback to dummy
      assert(c);
      CARD* clone = c->clone();
      COMPONENT* new_compon = prechecked_cast<COMPONENT*>(clone);
      untested();

      assert(new_compon);
      assert(!new_compon->owner());
      assert(new_compon->subckt());
      assert(new_compon->subckt()->is_empty());
      // BUG?: new_compon doesnt know its scope!
      // thats okay, symbols are global anyway.
      if (lang_geda.parse_componmod(cmd, new_compon)) {
        // this is not graphical
        lang_geda._componentname=new_compon->short_label();
        trace2("do_it", lang_geda._componentname, cmd.fullstring());
        CARD_LIST::const_iterator i = Scope->find_(new_compon->short_label());
        if (i != Scope->end()) {
          untested();
          // i'm here, because the "C" command has type "C" first....
          // hmm maybe it should look for the symbol first, as it might be there already.
          delete clone;
        }else{
            trace1("CMD_C pushback", hp(Scope));
            Scope->push_back(new_compon);
        }

        cmd.reset();
        trace1("not calling new__instance ", cmd.fullstring());
        lang_geda._gotline = true;
      } else {
        untested();
        delete clone;
      }

    }
} p2;
DISPATCHER<CMD>::INSTALL
    d2(&command_dispatcher, "C", &p2);

/*----------------------------------------------------------------------*/
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
// vim:ts=8:sw=4:et
