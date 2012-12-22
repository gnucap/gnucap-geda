/* (C) 2012 Felix Salfelder
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

#include <memory>
#include <gmpxx.h> // to workaround bug in gmp header about __cplusplus
#define COMPLEX NOCOMPLEX // COMPLEX already came from md.h
extern "C"{
# include <libgeda/libgeda.h>
}
#include <io_trace.h>
#include <ap.h>

using std::map;
using std::string;
using std::pair;
using std::vector;

enum angle_t { a_0 = 0,
               a_90 = 90,
               a_180 = 180,
               a_270 = 270 };

// should be map<string, morethanstring>?
class GEDA_PIN : public map<string, string>{
	public:
		GEDA_PIN(){
			operator[]("label") = "unknown";
			operator[]("pinseq") = "0";
		}
		GEDA_PIN(CS& cmd);
		typedef map<string, string> parent;
		typedef parent::const_iterator const_iterator;
	private:
		int _xy[2];
		unsigned _color;
		bool _bus; //"type of pin"
	public:
		int& x0(){return _xy[0];}
		int& y0(){return _xy[1];}
		//int& x1(){return _xy[2];}
		//int& y1(){return _xy[3];}
		unsigned& color(){return _color;}
		bool& bus(){return _bus;}
	public:
		bool has_key(const string key){
			const_iterator it = parent::find( key );
			return (it != end());
		}
	public:
		unsigned pinseq()const{
			assert(find("pinseq") != end());
			return atoi(find("pinseq")->second.c_str());
		}
		string label()const{
			assert(find("label") != end());
			return find("label")->second.c_str();
		}
};
/*--------------------------------------------------------------------------*/
class GEDA_SYMBOL : public map<string, string> {
	typedef map<string, string> parent;
	string _filename;
	unsigned _pincount;
	vector<GEDA_PIN> _pins;
	// T and stuff?

	public:
		GEDA_SYMBOL(){}
//		GEDA_SYMBOL(const GEDA_SYMBOL& x) :
//			map<string, string>(x),
//			_filename(x._filename),
//			_pincount(x._pincount)
//  		{
//			trace1("copying pins", _pins.size());
//			_pins = x._pins;
//		}
		GEDA_SYMBOL(string basename) :
		    _pins(),
		    x(0),
		    y(0),
		    mirror(0),
		    angle(a_0)
		{
			unsigned scope = 0;
			trace1( "GEDA_SYMBOL::GEDA_SYMBOL", basename);
			const CLibSymbol* symbol = s_clib_get_symbol_by_name(basename.c_str());
			if(!symbol){
				throw(Exception_Cant_Find("parsing gedanetlist", basename ));
			}
			string filename(s_clib_symbol_get_filename(symbol));
			trace1("...", filename);
			CS cmd(CS::_INC_FILE, filename);
			while(true){
				try{
					cmd.get_line("");
				}catch (Exception_End_Of_Input&){
					break;
				}
				if (cmd.match1('{')) {
					scope++;
				} else if(cmd.match1('}')) {
					assert(scope);
					scope--;
					continue;
				}

				if (!scope){
					if (cmd.umatch("P ")){
						// waah unclever...
						GEDA_PIN p(cmd);
						trace1("pin", cmd.fullstring());
						if(!p.has_key("pinseq")) {
							p["pinseq"] = to_string(1+_pins.size());
						}
						trace2("pin", p.pinseq(), max(1+p.pinseq(), _pins.size()));
						_pins.resize(1+max(p.pinseq(), _pins.size()));
						trace3("P", _pincount, _pins.size(), p.pinseq());
						trace1("pin", p.pinseq());
						_pins[p.pinseq()] = p;
					}
					{
						string pname = cmd.ctos("=","","");
						unsigned here = cmd.cursor();
						string pvalue;
						cmd >> "=";
						if(!cmd.stuck(&here)){
							cmd >> pvalue;
							parent::operator[](pname) = pvalue;
						}
					}
				}
			}
		}
		bool has_key(const string key){
			const_iterator it = parent::find( key );
			return (it != end());
		}
		unsigned pincount()const {return _pins.size();}
	public: // abuse for symbol instances
		int x;
		int y;
		bool mirror;
		angle_t angle;
		void push_back(const GEDA_PIN& x) {_pins.push_back(x);}
		vector<GEDA_PIN>::const_iterator pinbegin()const {return _pins.begin();}
		vector<GEDA_PIN>::const_iterator pinend()const {return _pins.end();}
		MODEL_SUBCKT* operator>>(MODEL_SUBCKT*) const;
		MODEL_SUBCKT* operator>>(COMPONENT*x) const{
			if(MODEL_SUBCKT* x=dynamic_cast<MODEL_SUBCKT*>(x)){
				return operator>>(x);
			}
			return 0;
		}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
GEDA_PIN::GEDA_PIN( CS& cmd )
{
	_xy[0] = cmd.ctoi();
	_xy[1] = cmd.ctoi();
	int x = cmd.ctoi();
	int y = cmd.ctoi();
	_bus = cmd.ctob();
	_color = cmd.ctou();
	bool swap = cmd.ctob();
	if (swap){
		_xy[0] = x;
		_xy[1] = y;
	}
	operator[]("label") = "unknown";
	string    _portvalue="_";
	try{
		cmd.get_line("");
	}catch(Exception_End_Of_Input&){
		return;
	}
	if (cmd >> "{")
		for(;;){
			cmd.get_line("");
			if(cmd>>"}"){
				cmd.get_line("");
				break;
			}else{
				if (cmd>>"T"){
				} else {
					string pname = cmd.ctos("=","","");
					string value;
					cmd >> "=" >> value;
					operator[](pname) = value;
				}
			}
		}
}
/*--------------------------------------------------------------------------*/
MODEL_SUBCKT* GEDA_SYMBOL::operator>>(MODEL_SUBCKT* m) const{
	for(vector<GEDA_PIN>::const_iterator i=pinbegin(); i!=pinend(); ++i){
		string l=i->label(); // why string&? hmmm
		trace1(__func__, l);
		m->set_port_by_index(i->pinseq(), l);
	}
	return m;
}
/*--------------------------------------------------------------------------*/
class GEDA_SYMBOL_MAP : public map<string, GEDA_SYMBOL> {
	typedef map<string, GEDA_SYMBOL> parent;

	public:
		GEDA_SYMBOL& operator[](const string key){
			typename map<string, GEDA_SYMBOL>::const_iterator it = parent::find( key );
			GEDA_SYMBOL& s = parent::operator[](key);
			if ( it == parent::end() ) {
				s = GEDA_SYMBOL(key);
			} else {
			}

			return parent::operator[](key);
		}
};
/*--------------------------------------------------------------------------*/
