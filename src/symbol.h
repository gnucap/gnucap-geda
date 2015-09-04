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
#undef COMPLEX
#include <io_trace.h>
#include <ap.h>
#include <set>

enum angle_t { a_invalid = -1,
               a_0 = 0,
               a_90 = 90,
               a_180 = 180,
               a_270 = 270 };

// should be std::map<std::string, morethanstring>?
class GEDA_PIN : public std::map<std::string, std::string>{
	public:
		GEDA_PIN(){
			operator[]("pinlabel") = "unknown";
			operator[]("pinseq") = "0";
		}
		GEDA_PIN(CS& cmd);
		typedef std::map<std::string, std::string> parent;
		typedef parent::const_iterator const_iterator;
	private:
		int _xy[2];
		unsigned _color;
		bool _bus; //"type of pin"
	public:
		int& x0(){return _xy[0];}
		int& y0(){return _xy[1];}
		int x0()const{return _xy[0];}
		int y0()const{return _xy[1];}
		//int& x1(){return _xy[2];}
		//int& y1(){return _xy[3];}
		unsigned& color(){return _color;}
		bool& bus(){return _bus;}
	public:
		bool has_key(const std::string key)const{
			const_iterator it = parent::find( key );
			return (it != end());
		}
	public:
		unsigned pinseq()const{
			assert(find("pinseq") != end());
			return atoi(find("pinseq")->second.c_str());
		}
		std::string label()const{
			if(find("pinlabel") == end()){
				incomplete();
				assert(find("pinseq") != end());
				return( "unknown_pin_" + find("pinseq")->second );
			}
			return find("pinlabel")->second.c_str();
		}
};
/*--------------------------------------------------------------------------*/
class GEDA_SYMBOL : public std::map<std::string, std::string> {
	typedef std::map<std::string, std::string> parent;
	std::string _filename;
	std::set<GEDA_PIN> _pins;
	// T and stuff?

	public:
		GEDA_SYMBOL(){}
//		GEDA_SYMBOL(const GEDA_SYMBOL& x) :
//			std::map<std::string, std::string>(x),
//			_filename(x._filename),
//			_pincount(x._pincount)
//  		{
//			trace1("copying pins", _pins.size());
//			_pins = x._pins;
//		}
		GEDA_SYMBOL(std::string basename) :
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
			std::string filename(s_clib_symbol_get_filename(symbol));
			trace1("...", filename);
			CS cmd(CS::_INC_FILE, filename);
			cmd.get_line("");
			while(true){
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
						if(!p.has_key("pinseq")) {
							untested();
							p["pinseq"] = to_string(unsigned(1+_pins.size()));
						} else {
						}
						// _pins.resize(max(p.pinseq(), _pins.size()+1));
						assert(p.pinseq());
						_pins.insert(p);
						continue; // line has been read
					}
					{
						std::string pname = cmd.ctos("=","","");
						unsigned here = cmd.cursor();
						std::string pvalue;
						cmd >> "=";
						if(!cmd.stuck(&here)){
							cmd >> pvalue;
							if(pvalue!="")
								parent::operator[](pname) = pvalue;
						}
					}
				}
				try{
					cmd.get_line("");
				}catch (Exception_End_Of_Input&){
					break;
				}
			}
			trace2("done parse", basename, _pins.size());
		}
		bool has_key(const std::string key) const{
			const_iterator it = parent::find( key );
			return (it != end());
		}
		unsigned pincount()const {return _pins.size();}
		const std::string operator[](const std::string& x)const{
			const_iterator it = parent::find( x );
			if (it != end()) return it->second;
			return "";
		}
		std::string& operator[](const std::string& x){
			return parent::operator[](x);
		}
	public: // abuse for symbol instances
		int x;
		int y;
		bool mirror;
		angle_t angle;
		void push_back(const GEDA_PIN& x) {_pins.insert(x);}
		std::set<GEDA_PIN>::const_iterator pinbegin()const {return _pins.begin();}
		std::set<GEDA_PIN>::const_iterator pinend()const {return _pins.end();}
		COMPONENT* operator>>(COMPONENT*) const;
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
	std::string    _portvalue="_";
	try{
		cmd.get_line("");
	}catch(Exception_End_Of_Input&){
		return;
	}
	if (cmd >> "{")
		for(;;){
			cmd.get_line("");
			if(cmd>>"}"){
				cmd.get_line(""); // better just try?
				break;
			}else{
				if (cmd>>"T"){
				} else {
					std::string pname = cmd.ctos("=","","");
					std::string value;
					unsigned here = cmd.cursor();
					cmd >> "=";
					if(!cmd.stuck(&here)){
						cmd >> value;
						operator[](pname) = value;
					} else {
						untested();
					}
				}
			}
		}
	if(find("pinlabel") == end()){
	}
}
/*--------------------------------------------------------------------------*/
COMPONENT* GEDA_SYMBOL::operator>>(COMPONENT* m) const{
	for(std::set<GEDA_PIN>::const_iterator i=pinbegin(); i!=pinend(); ++i){
		trace0("GEDA_SYMBOL::operator>>");
		std::string l = i->label(); // why std::string&? hmmm
		trace2("GEDA_SYMBOL::operator>>", i->pinseq(), i->label());
		assert(i->pinseq());
		m->set_port_by_index(i->pinseq()-1, l);
	}
	return m;
}
/*--------------------------------------------------------------------------*/
class GEDA_SYMBOL_MAP : public std::map<std::string, GEDA_SYMBOL> {
	typedef std::map<std::string, GEDA_SYMBOL> parent;

	public:
		GEDA_SYMBOL& operator[](const std::string key){
			parent::const_iterator it = parent::find( key );
			GEDA_SYMBOL& s = parent::operator[](key);
			if ( it == parent::end() ) {
				s = GEDA_SYMBOL(key);
			} else {
			}

			return parent::operator[](key);
		}
};
/*--------------------------------------------------------------------------*/
