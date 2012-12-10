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

using std::map;
using std::string;
using std::pair;

class GEDA_SYMBOL : public map<string, string>{
	typedef map<string, string> parent;
	string _filename;
	unsigned _pincount;
	// T and stuff?

	public:
//		typedef map<string, string>::const_iterator const_iterator;
//		string& operator[](const string x){return _dict[x];}

		GEDA_SYMBOL(){}
//		GEDA_SYMBOL(const GEDA_SYMBOL&p): parent(p), _filename(p._filename){}
		GEDA_SYMBOL(string basename):_pincount(0) {
			unsigned scope = 0;
			trace1( "GEDA_SYMBOL", basename);
			const CLibSymbol* symbol = s_clib_get_symbol_by_name(basename.c_str());
			if(!symbol){
				throw(Exception_Cant_Find("parsing gedanetlist", basename ));
			}
			std::string filename(s_clib_symbol_get_filename(symbol));
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
						_pincount++;
					} else {
						string pname = cmd.ctos("=","","");
						unsigned here = cmd.cursor();
						string pvalue;
						cmd >> "=";
						if(!cmd.stuck(&here)){
							cmd >> pvalue;
							trace3("GEDA_SYMBOL push", pname, pvalue, cmd.fullstring());
							parent::operator[](pname) = pvalue;
							trace1("GEDA_SYMBOL push", parent::operator[](pname) );
						}
					}
				}
			}
		}
		bool has_key(const string key){
			 const_iterator it = parent::find( key );
			 return (it != end());
		}
		unsigned pincount()const {return _pincount;}
};
/*--------------------------------------------------------------------------*/
class GEDA_SYMBOL_MAP : public std::map<string, GEDA_SYMBOL> {
	typedef std::map<string, GEDA_SYMBOL> parent;

	public:
		GEDA_SYMBOL& operator[](const string key){
			typename std::map<string, GEDA_SYMBOL>::const_iterator it = parent::find( key );
			GEDA_SYMBOL& s = parent::operator[](key);
			if ( it == parent::end() ) {
				s = GEDA_SYMBOL(key);
			} else {
			}

			return parent::operator[](key);
		}
};
/*--------------------------------------------------------------------------*/
