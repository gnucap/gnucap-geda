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
#include "c_comand.h"
#include "d_dot.h"
#include "d_coment.h"
#include "d_subckt.h"
#include "e_model.h"
#include "u_lang.h"
#include <fts.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class LANG_GSCHEM : public LANGUAGE {

public:
    //
    enum MODE {mDEFAULT, mPARAMSET} _mode;
    mutable int arg_count;
    enum {INACTIVE = -1};
    //
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
    DEV_COMMENT*  parse_comment(CS&, DEV_COMMENT*);
    DEV_DOT*	  parse_command(CS&, DEV_DOT*);
    MODEL_CARD*	  parse_paramset(CS&, MODEL_CARD*);
    MODEL_SUBCKT* parse_module(CS&, MODEL_SUBCKT*);
    COMPONENT*	  parse_instance(CS&, COMPONENT*);
    std::string	  find_type_in_string(CS&);
    std::vector<CARD*> nets;

private:
    void print_paramset(OMSTREAM&, const MODEL_CARD*);
    void print_module(OMSTREAM&, const MODEL_SUBCKT*);
    void print_instance(OMSTREAM&, const COMPONENT*);
    void print_comment(OMSTREAM&, const DEV_COMMENT*);
    void print_command(OMSTREAM& o, const DEV_DOT* c);

private:
    void print_args(OMSTREAM&, const MODEL_CARD*);
    void print_args(OMSTREAM&, const COMPONENT*);
}lang_gschem;

DISPATCHER<LANGUAGE>::INSTALL
    d(&language_dispatcher, lang_gschem.name(), &lang_gschem);
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
//functions to be defined

  //Finds type from find_type_in_string
  //If component, then assign type as device=.. ?
static void parse_type(CS& cmd, CARD* x)
{
  assert(x);
  std::string new_type;
  new_type=lang_gschem.find_type_in_string(cmd);
  x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_COMMENT* LANG_GSCHEM::parse_comment(CS& cmd, DEV_COMMENT* x)
{
  assert(x);
  x->set(cmd.fullstring());
  return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_GSCHEM::parse_command(CS& cmd, DEV_DOT* x)
{
  std::cout<<"Command parsed\n";
  assert(x);
  x->set(cmd.fullstring());
  CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;

  cmd.reset();
  CMD::cmdproc(cmd, scope);
  delete x;
  return NULL;
}
/*--------------------------------------------------------------------------*/
MODEL_CARD* LANG_GSCHEM::parse_paramset(CS& cmd, MODEL_CARD* x)
{
  assert(x);
  return NULL;
}
/*--------------------------------------------------------------------------*/
static void parse_pin(CS& cmd, COMPONENT* x, int index)
{
    assert(x);
    std::string type=OPT::language->find_type_in_string(cmd),dump;
    assert(type=="pin");
    cmd>>"P";
    std::string xpin,ypin;
    std::string pinattributes[7];
    for(int i=0;i<7;i++){
        cmd>>" ">>pinattributes[i];
    }
    if (pinattributes[6]=="1"){
        xpin=pinattributes[2];
        ypin=pinattributes[3];
    }else if (pinattributes[6]=="0"){
        xpin=pinattributes[0];
        ypin=pinattributes[1];
    }
    std::string    _portvalue="_";
    cmd.get_line("");
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
    x->set_port_by_index(index,_portvalue);
}
/*--------------------------------------------------------------------------*/
static std::string find_file_given_name(std::string basename)
{
    char *p[]={"../geda-sym/symbols",NULL};
    FTS* dir=fts_open(p,FTS_NOCHDIR, NULL);
    if(!dir){
        std::cout<<"Not opened\n";
    }
    assert(dir);
    FTSENT* node;
    std::string dirname="";
    while(node=fts_read(dir)){
        if(node->fts_info & FTS_F && basename==node->fts_name){
            std::cout<<node->fts_name<<std::endl;
            std::cout<<"Got it"<<" "<<node->fts_path<<std::endl;
            dirname=node->fts_path;
        }
    }
    if(dirname==""){
        std::cout<<"No symbol file for"+basename<<std::endl;
    }
    return dirname;
}
/*--------------------------------------------------------------------------*/
static void parse_symbol_file(COMPONENT* x, std::string basename)
{
    std::string filename=find_file_given_name(basename),dump;
    CS sym_cmd(CS::_INC_FILE, filename);
    //Now parse the sym_cmd which will get lines
    int index=0;
    while(true){
        try{
            sym_cmd.get_line("");
        }catch (Exception_End_Of_Input&){
            std::cout<<"Breaking";
            break;
        }
        std::cout<<"Got the line";
        std::cout<<sym_cmd.fullstring();
        if(sym_cmd.fullstring()!=""){
            std::cout<<sym_cmd.fullstring()<<"Came here"<<std::endl;
        }
        std::string linetype=OPT::language->find_type_in_string(sym_cmd);
        if (linetype=="pin"){
            parse_pin(sym_cmd,x,index++);
            std::cout<<"Pin number "+to_string(index)<<std::endl;
        }else{
            sym_cmd>>dump;
        }
    }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
MODEL_SUBCKT* LANG_GSCHEM::parse_module(CS& cmd, MODEL_SUBCKT* x)
{
  std::cout<<"Got into parse_module\n";
  assert(x);
  cmd.reset();
  std::string type = find_type_in_string(cmd);
  assert(type=="C");
  std::string component_x, component_y, mirror, angle, dump,basename;
  bool isgraphical=false;
  cmd>>"C";
  unsigned here=cmd.cursor();
  cmd>>component_x>>" ">>component_y>>" ">>dump>>" ">>angle>>" ">>mirror>>" ">>basename;
  //open the basename to get the ports and their placements
  //parse_ports(newcmd,x);
      parse_symbol_file(x,basename);
  x->set_label(basename);
  if (isgraphical==true) {
    return NULL;
  }
  else{
    cmd.reset(here);
  }
  std::cout<<"Going out of parse_module\n";
  return x;
}
/*--------------------------------------------------------------------------*/
// A net is in form N x0 y0 x1 y1 c
// Need to get x0 y0 ; x1 y1 ;
// Need to go through all the nets. Anyway?
// Need to save them in other forms? How to go through all cards?
// Need to specify a name for a card?
static void parse_net(CS& cmd, COMPONENT* x)
{
    assert(x);
    assert(lang_gschem.find_type_in_string(cmd)=="net");
    cmd>>"N";     //Got N
    std::string x0,y0,x1,y1,color;
    unsigned here=cmd.cursor();
    std::string paramnames[5]={"x0","x1","y0","y1","color"};
    std::string paramvalue[5];
    int i=0;
    bool gotthenet=true;
    while (i<5) {
        if (cmd.is_alnum()){
            cmd>>" ">>paramvalue[i];
            try{
                x->set_param_by_name(paramnames[i],paramvalue[i]);
            }catch (Exception_No_Match&) {untested();
                cmd.warn(bDANGER, here, x->long_label() + ": bad parameter "+ paramvalue[i] + "ignored");
            }
        }else{
            gotthenet=false;
            cmd.warn(bDANGER, here, x->long_label() +": Not correct format for net");
            break;
        }
        ++i;
    }
    if (gotthenet){
        //lang_gschem.nets.push_back(x);
        //To check if any of the previous nodes have same placement.
    }
    x->set_label("net"+to_string(rand()%10000)); //BUG : names may coincide!. Doesn't matter? Or try some initialisation method. (latch like digital)
    std::string _node1="node"+to_string(rand()%10000);
    std::string _node2="node"+to_string(rand()%10000);
    std::cout<<_node1<<" "<<_node2<<std::endl;
    std::string nodename1="p";
    std::string nodename2="n";
    std::string node1="whaer1\0";
    std::string node2="whaer2\0";
    int indexp=0;
    x->set_port_by_name(nodename1,node1);
    std::cout<<"got out here too";
    x->set_port_by_name(nodename2,node2);
    std::cout<<"got out";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void parse_component(CS& cmd,COMPONENT* x){
    std::cout<<"Got into parse_componet-1\n";
    assert(x);
    std::string component_x, component_y, mirror, angle, dump,basename;
    std::string type=lang_gschem.find_type_in_string(cmd);
    std::cout<<type;
    cmd>>type;
    std::vector<std::string> paramname;
    std::vector<std::string> paramvalue;
    std::cout<<"Got into parse_componet0\n";
    cmd>>component_x>>" ">>component_y>>" ">>dump>>" ">>angle>>" ">>mirror>>" ">>basename;
    std::cout<<component_x<<" "<<component_y<<" "<<dump<<" "<<angle<<" "<<mirror<<" "<<basename;
    //To get port names and values from symbol?
    //Then set params below
    //Search for the file name
    parse_symbol_file(x,basename);

    std::cout<<"Got into parse_componet1\n";
    x->set_param_by_name("x",component_x);
    x->set_param_by_name("y",component_y);
    x->set_param_by_name("angle",angle);
    x->set_param_by_name("mirror",mirror);
    x->set_param_by_name("basename",basename);
    std::cout<<"Got into parse_componet2\n";
    std::cout<<x->param_count()<<std::endl;
    CS new_cmd(CS::_STDIN);
    new_cmd.get_line("gnucap-gschem- "+basename+">");
    new_cmd>>"{";
    for (;;) {
        new_cmd.get_line("gnucap-gschem- "+basename+">");
        std::cout<<"Got into parse_componet3\n";
        if (new_cmd >> "}") {
            break;
        }else{
            if(new_cmd>>"T"){
                new_cmd>>dump;
            }
            else {
                std::cout<<new_cmd.fullstring()<<std::endl;
                std::string _paramname=new_cmd.ctos("=","",""),_paramvalue;
                new_cmd>>"=">>_paramvalue;
                paramname.push_back (_paramname);
                std::cout<<"fine0\n";
                paramvalue.push_back(_paramvalue);
                std::cout<<"fine1\n";
                std::cout<<_paramname<<" "<<_paramvalue<<std::endl;
                if(_paramname=="device"){
                    x->set_dev_type(_paramvalue);
                }else if (_paramname=="refdes" && _paramvalue!="?"){
                    x->set_label(_paramvalue);
                }else{
                    x->set_param_by_name(_paramname,_paramvalue);
                }
            }
        }
    }
    if(x->short_label()!=""){
        x->set_label(type+to_string(rand()));
    }
    std::cout<<"Going out of parse_component"<<std::endl;
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_GSCHEM::parse_instance(CS& cmd, COMPONENT* x)
{
  cmd.reset();
  parse_type(cmd, x); //parse type will parse the component type and set_dev_type
  if (x->dev_type()=="net") {
    parse_net(cmd,x);
  }else {
    parse_component(cmd,x);
  }
  cmd.check(bWARNING, "what's ins this?");
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
std::string LANG_GSCHEM::find_type_in_string(CS& cmd)
{
    unsigned here = cmd.cursor(); //store cursor position to reset back later
    std::string type;   //stores type : should check device attribute..
    //graphical=["v","L","G","B","V","A","H","T"]
    if (cmd >> "v " || cmd >> "L " || cmd >> "G " || cmd >> "B " || cmd >>"V " ||
        cmd >> "A " || cmd >> "H " || cmd >> "T "){ type="dev_comment";}
    else if (cmd >> "N "){ type="net";}
    else if (cmd >> "U "){ type="bus";}
    else if (cmd >> "P "){ type="pin";}
    else if (cmd >> "C "){ type="C";}
    else {  cmd >> type; } //Not matched with the type. What now?
    cmd.reset(here);//Reset cursor back to the position that
                    //has been started with at beginning
    return type;    // returns the type of the string
}
/*----------------------------------------------------------------------*/
/* parse_top_item :
 * The top default thing that is parsed. Here new__instances are
 * created and (TODO)post processing of nets is done
 */
void LANG_GSCHEM::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
  cmd.get_line("gnucap-gschem>");
  new__instance(cmd, NULL, Scope);
  //Can iterate over all the CARD_LIST
  /*for(CARD_LIST::const_iterator ci=Scope->begin();ci!= Scope->end();++ci) {
    std::cout<<(*ci)->dev_type()<<" "<<(*ci)->param_value(4)<<std::endl;
  }*/
}
/*----------------------------------------------------------------------*/

//Printing stuff : TO ADD

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
class CMD_GSCHEM : public CMD {
public:
    void do_it(CS& cmd, CARD_LIST* Scope)
    {
      command("options lang=gschem", Scope);
    }
} p8;
DISPATCHER<CMD>::INSTALL
    d8(&command_dispatcher, "gschem", &p8);
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
class CMD_C : public CMD {
    void do_it(CS& cmd, CARD_LIST* Scope)
    {
      MODEL_SUBCKT* new_compon = new MODEL_SUBCKT;
      assert(new_compon);
      assert(!new_compon->owner());
      assert(new_compon->subckt());
      assert(new_compon->subckt()->is_empty());
      lang_gschem.parse_module(cmd, new_compon);
      if(new_compon){
        Scope->push_back(new_compon);
        std::string s=new_compon->short_label()+" "+cmd.tail();
        CS cmd(CS::_STRING,s);
        lang_gschem.new__instance(cmd,NULL,Scope);
      }
    }
} p2;
DISPATCHER<CMD>::INSTALL
    d2(&command_dispatcher, "C", &p2);

/*----------------------------------------------------------------------*/
}
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
