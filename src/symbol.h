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

enum mirror_t { m_invalid = -1,
                m_0 = 0,
                m_r = 1 };

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
		std::pair<int,int> _xy;
		unsigned _color;
		bool _bus; //"type of pin"
	public:
		int& x0(){return _xy.first;}
		int& y0(){return _xy.second;}
		int x0()const{return _xy.first;}
		int y0()const{return _xy.second;}
		const std::pair<int,int>& X()const { return _xy; }
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
			// the manual says pinseq is mandatory.
			assert(find("pinseq") != end()); // for now.
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
class GEDA_SYMBOL {
	std::string _filename;
	std::set<GEDA_PIN> _pins;
	std::map<std::string, const GEDA_PIN*> _pins_by_name;
	std::vector<const GEDA_PIN*> _pins_by_seq;
  	std::map<std::string, std::string> _attribs;
	// T and stuff?

	public:
		typedef std::map<std::string, std::string>::iterator iterator;
		typedef std::map<std::string, std::string>::const_iterator const_iterator;
		GEDA_SYMBOL() {untested();}
		~GEDA_SYMBOL(){ untested(); }
	private:
		GEDA_SYMBOL(const GEDA_SYMBOL& p) :
			_pins(p._pins),
			_pins_by_name(p._pins_by_name), // hmmm.
			_pins_by_seq(p._pins_by_seq), // hmmm.
			_attribs(p._attribs),
			x(p.x),
			y(p.y),
			_mirror(p._mirror),
			_angle(p._angle)
			{untested();}
//		GEDA_SYMBOL(const GEDA_SYMBOL& x) :
//			std::map<std::string, std::string>(x),
//			_filename(x._filename),
//			_pincount(x._pincount)
//  		{
//			trace1("copying pins", _pins.size());
//			_pins = x._pins;
//		}
	public:
		GEDA_SYMBOL* clone() const{return new GEDA_SYMBOL(*this);}
		GEDA_SYMBOL(std::string basename) :
		    _pins(),
		    x(0),
		    y(0),
		    _mirror(0),
		    _angle(a_0)
		{
			unsigned scope = 0;
			trace1( "GEDA_SYMBOL::GEDA_SYMBOL", basename);
			const CLibSymbol* symbol = s_clib_get_symbol_by_name(basename.c_str());
			if(!symbol){ untested();
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
						if(!p.has_key("pinseq")) { untested();
							error(bWARNING, "pin without pinseq in %s\n", basename.c_str());
							p["pinseq"] = to_string(unsigned(1+_pins.size()));
						} else {
						}
						// _pins.resize(max(p.pinseq(), _pins.size()+1));
						assert(p.pinseq()); // starts at 1?
						std::pair<std::set<GEDA_PIN>::const_iterator,bool> f = _pins.insert(p);
						if(f.second){
						}else{incomplete();
							error(bDANGER,"pin collision %d %s\n", p.pinseq(), p.label().c_str());
							// collision
						}
						const GEDA_PIN* P = &(*(f.first));
						_pins_by_name[p.label()] = P;

						if(p.pinseq()<=_pins_by_seq.size()){
							// ok
						}else{
							_pins_by_seq.resize(p.pinseq());
						}
						_pins_by_seq[p.pinseq()-1] = P;
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
								_attribs[pname] = pvalue;
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
			const_iterator it = _attribs.find( key );
			return (it != _attribs.end());
		}
		unsigned pincount()const {return _pins.size();}
		const std::string operator[](const std::string& x)const{
			const_iterator it = _attribs.find( x );
			if (it != _attribs.end()) return it->second;
			return "";
		}
		std::string& operator[](const std::string& x){
			return _attribs[x];
		}
		iterator begin(){ return _attribs.begin(); }
		iterator end(){ return _attribs.end(); }
	public: // abuse for symbol instances
		int x;
		int y;
		void push_back(const GEDA_PIN& x) {_pins.insert(x);}
		std::set<GEDA_PIN>::const_iterator pinbegin()const {return _pins.begin();}
		std::set<GEDA_PIN>::const_iterator pinend()const {return _pins.end();}
		COMPONENT* operator>>(COMPONENT*) const;
		GEDA_PIN const* pin(std::string x){return _pins_by_name[x];}
		GEDA_PIN const* pin(unsigned x){assert(x); return _pins_by_seq[x-1];}
		const angle_t& angle()const {return _angle;}
		bool mirror()const {return _mirror;}
		// BUG?
		void set_angle(angle_t x){_angle=x;}
		void set_mirror(bool x) {_mirror=x;}
	private:
		bool _mirror;
		angle_t _angle;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
GEDA_PIN::GEDA_PIN( CS& cmd )
{
	x0() = cmd.ctoi();
	y0() = cmd.ctoi();
	int x = cmd.ctoi();
	int y = cmd.ctoi();
	_bus = cmd.ctob();
	_color = cmd.ctou();
	bool swap = cmd.ctob();
	if (swap){
		x0() = x;
		y0() = y;
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
// sort of dynamic dispatcher ...
class GEDA_SYMBOL_MAP{
	public:
		typedef std::map<std::string, GEDA_SYMBOL*> parent;
		~GEDA_SYMBOL_MAP(){ untested();
			for(parent::iterator i = _m.begin(); i!=_m.end(); ++i){ untested();
				// delete i->second;
			}
		}

	public:
		GEDA_SYMBOL* operator[](const std::string key){
			parent::const_iterator it = _m.find( key );
			GEDA_SYMBOL*& s = _m[key];
			if ( it == _m.end() ) { untested();
				trace1("GEDA_SYMBOL_MAP", key);
				s = new GEDA_SYMBOL(key);
			} else {
			}

			return _m[key];
		}
	private:
	  	std::map<std::string, GEDA_SYMBOL*> _m;
};
/*--------------------------------------------------------------------------*/
