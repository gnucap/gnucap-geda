/*$Id: io_trace.h,v 1.7 2010-09-07 07:46:23 felix Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 * trace macros for model debugging
 */
//testing=trivial 2006.07.17
/* allow multiple inclusions with different DO_TRACE */
#include <iostream>
#include <stdio.h>
using std::cerr;
using std::cout;
using std::endl;


#undef trace_line
#undef trace
#undef trace0
#undef trace1
#undef trace2
#undef trace3
#undef trace4
#undef trace5
#undef trace6
#undef trace7
#undef trace8
#undef itested
#undef untested
#undef untested0
#undef unreachable
#undef incomplete
/*--------------------------------------------------------------------------*/
#define USE(x) (1)?(void)(0):(void)(x)

#ifndef hashpointer_
# define hashpointer_
#include <stdint.h> // intptr_t

class hp{
	intptr_t p;
	public:
	hp(const void* x){
		p = (intptr_t)x %30011;
	}
	operator int(){
		return static_cast<int>(p);
	}
};

#endif 

#define deprecated() (fprintf(stderr, "@!@#$\n@@@deprecated:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))

#ifdef DO_TRACE

#define trace_line() (fprintf(stderr, "@@#\n@#@:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))
#define trace0(s) ( cerr << "@#@" << (s) << "\n")
#define trace1(s,x) ( \
		cerr <<  "@#@" << (s) << "  " << #x << "=" << (x)  \
		     << endl )
#define trace2(s,x,y) ( \
		cerr <<  "@#@" << (s) << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << endl )
#define trace3(s,x,y,z) ( \
		cerr <<  "@#@" << (s) << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#define trace4(s,w,x,y,z) ( \
		cerr <<  "@#@" << (s) << "  " << #w << "=" << (w)  \
		     << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#define trace5(s,v,w,x,y,z) ( \
		cerr <<  "@#@" << (s) << "  " << #v << "=" << (v)  \
		     << "  " << #w << "=" << (w)  \
		     << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#define trace6(s,u,v,w,x,y,z) ( \
		cerr <<  "@#@" << (s) \
		     << "  " << #u << "=" << (u)  \
		     << "  " << #v << "=" << (v)  \
		     << "  " << #w << "=" << (w)  \
		     << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#define trace7(s,t,u,v,w,x,y,z) ( \
		cerr <<  "@#@" << (s)  \
		     << "  " << #t << "=" << (t)  \
		     << "  " << #u << "=" << (u)  \
		     << "  " << #v << "=" << (v)  \
		     << "  " << #w << "=" << (w)  \
		     << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#define trace8(s,r,t,u,v,w,x,y,z) ( \
		cerr <<  "@#@" << (s)  \
		     << "  " << #r << "=" << (r)  \
		     << "  " << #t << "=" << (t)  \
		     << "  " << #u << "=" << (u)  \
		     << "  " << #v << "=" << (v)  \
		     << "  " << #w << "=" << (w)  \
		     << "  " << #x << "=" << (x)  \
		     << "  " << #y << "=" << (y)  \
		     << "  " << #z << "=" << (z)  \
		     << endl )
#else
#define trace_line()
#define trace0(s) USE(s)
#define trace1(s,x) (USE(s),USE(x))
#define trace2(s,x,y) USE(s);USE(x);USE(y)
#define trace3(s,x,y,z) USE(s);USE(x);USE(y);USE(z)
#define trace4(s,w,x,y,z) USE(s);USE(w);USE(x);USE(y);USE(z)
#define trace5(s,v,w,x,y,z) USE(s);USE(v);USE(w);USE(x);USE(y);USE(z)
#define trace6(s,u,v,w,x,y,z) USE(s);USE(u);USE(v);USE(w);USE(x);USE(y);USE(z)
#define trace7(s,t,u,v,w,x,y,z) USE(s);USE(t);USE(u);USE(v);USE(w);USE(x);USE(y);USE(z)
#define trace8(r,s,t,u,v,w,x,y,z) USE(r);USE(s);USE(t);USE(u);USE(v);USE(w);USE(x);USE(y);USE(z)
#endif

#define unreachable() (fprintf(stderr, "@@#\n@@@unreachable:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))

#define incomplete() (fprintf(stderr, "@@#\n@@@incomplete:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))

#ifdef DO_TRACE
#define untout stderr
#else
#define untout stderr
#endif

#ifdef TRACE_UNTESTED
#define untested() (fprintf(untout, "@@# untested \n@@@:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))
#define untested0(s) (fprintf(stderr, "@@#\n@@@:%s:%u:%s: %s\n", \
			   __FILE__, __LINE__, __func__, s))
#define untested1(s,x) ( cerr <<  "@@#\n@@@:"<< __FILE__ << ":"<< __LINE__ <<":" << __func__ << ": "  << s << "  " << #x << "=" << (x) << endl )
#define untested2(s,x,y) (fprintf(stderr, "@@#\n@@@:%s:%u:%s: %s  %s=%g, %s=%g\n", \
			   __FILE__, __LINE__, __func__, s, #x, (double)(x), #y, (double)(y) ))
#define untested3(s,x,y,z) ( cerr <<  "@@#\n@@@" << s << "  " << #x << "=" << (x)  \
		                                      << "  " << #y << "=" << (y)  \
		                                      << "  " << #z << "=" << z  \
		                                      << endl )
#else
#define untested()
#define untested0(s)
#define untested1(s,x)
#define untested2(s,x,y)
#define untested3(s,x,y,z)
#endif

#ifdef TRACE_ITESTED
#define itested() (fprintf(stderr, "@i#\n@i@:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))
#else
#define itested()
#endif


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
