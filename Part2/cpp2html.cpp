/*
 * CSc103 Project 5: Syntax highlighting, part two.
 * See readme.html for details.
 * Please list all references you made use of in order to complete the
 * assignment: your classmates, websites, etc.  Aside from the lecture notes
 * and the book, please list everything.  And remember- citing a source does
 * NOT mean it is okay to COPY THAT SOURCE.  What you submit here **MUST BE
 * YOUR OWN WORK**.
 * References:
 *
 *
 * Finally, please indicate approximately how many hours you spent on this:
 * #hours: 
 */

#include "fsm.h"
using namespace cppfsm;
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <set>
using std::set;
#include <map>
using std::map;
#include <initializer_list> // for setting up maps without constructors.

// enumeration for our highlighting tags:
enum {
	hlstatement,  // used for "if,else,for,while" etc...
	hlcomment,    // for comments
	hlstrlit,     // for string literals
	hlpreproc,    // for preprocessor directives (e.g., #include)
	hltype,       // for datatypes and similar (e.g. int, char, double)
	hlnumeric,    // for numeric literals (e.g. 1234)
	hlescseq,     // for escape sequences
	hlerror,      // for parse errors, like a bad numeric or invalid escape
	hlident       // for other identifiers.  Probably won't use this.
};

// usually global variables are a bad thing, but for simplicity,
// we'll make an exception here.
// initialize our map with the keywords from our list:
map<string, short> hlmap = {
#include "res/keywords.txt"
};

map<string, short>::iterator iter;
map<string, short>::iterator iter2;

// note: the above is not a very standard use of #include...

// map of highlighting spans:
map<int, string> hlspans = {
	{hlstatement, "<span class='statement'>"},
	{hlcomment, "<span class='comment'>"},
	{hlstrlit, "<span class='strlit'>"},
	{hlpreproc, "<span class='preproc'>"},
	{hltype, "<span class='type'>"},
	{hlnumeric, "<span class='numeric'>"},
	{hlescseq, "<span class='escseq'>"},
	{hlerror, "<span class='error'>"}
};
// note: initializing maps as above requires the -std=c++0x compiler flag,
// as well as #include<initializer_list>.  Very convenient though.
// to save some typing, store a variable for the end of these tags:
string spanend = "</span>";

string translateHTMLReserved(char c) {
	switch (c) {
		case '"':
			return "&quot;";
		case '\'':
			return "&apos;";
		case '&':
			return "&amp;";
		case '<':
			return "&lt;";
		case '>':
			return "&gt;";
		case '\t': // make tabs 4 spaces instead.
			return "&nbsp;&nbsp;&nbsp;&nbsp;";
		default:
			char s[2] = {c,0};
			return s;
	}
}
string htmlDestroyer(string s)
{
   unsigned int i;
   int cstate     = start;
   int previous;
   string output  = "";
   string tagline = "";
   
   for(i = 0; i<s.length(); i++)
   {
      previous = updateState(cstate,s[i]);
	  
	  //FOR START///
	  
	  while( cstate == start )
	  {
	     if( s[i] == '"' )
		 {
		    tagline = tagline + hlspans[hlstrlit] + translateHTMLReserved(s[i]) + spanend;
		 }
		 else
		 {
	        tagline = tagline + translateHTMLReserved(s[i]);
		 }
		 
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate!= start || i == s.length() )
		 {
		    i--;
			cstate = start;
			break;
		 }
	  } 
	  
	  ///FOR SCANID///
	  
	  while( cstate == scanid )
	  {
	     output = output + s[i];
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != scanid || i == s.length() )
		 {
		    iter  = hlmap.find(output);
			iter2 = hlmap.find(output);
			i--;
			cstate = scanid;
			
			if( iter != hlmap.end() )
			{
			   for( iter = hlmap.begin(); iter != hlmap.end(); iter++ )
			   {
			      if( hlmap[iter2 -> first] == hlmap[iter -> first] && hlmap[iter -> first] == hltype )
				  {
				     tagline = tagline + hlspans[hltype] + output + spanend;
					 break;
				  }
				  else if( hlmap[iter2 -> first] == hlmap[iter -> first] && hlmap[iter -> first] == hlstatement )
				  {
				     tagline = tagline + hlspans[hlstatement] + output + spanend;
					 break;
				  }
				  else if( hlmap[iter2 -> first] == hlmap[iter -> first] && hlmap[iter -> first] == hlpreproc )
				  {
				     tagline = tagline + hlspans[hlpreproc] + output + spanend;
					 break;
				  }
			   }
			}
			else
			{
			   tagline = tagline + output;
			}
			output = "";
			break;
		 }
	  }
	  
	  //FOR FORWARD SLASH
	  
	  while( cstate == readfs )
	  {
	     if(s[i+1] == '/')
		 {
		    output = "";
		 }
		 else
		 {
		    output = output + translateHTMLReserved(s[i]);
		 }
		 
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != readfs || i == s.length() )
		 {
		    tagline = tagline + output;
			output ="";
			i--;
			cstate = readfs;
			break;
		 }
	  }
	  //FOR COMMENT
	  while( cstate == comment )
	  {
	     output = output + translateHTMLReserved(s[i]);
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != comment || i == s.length() )
		 {
		    tagline = tagline + hlspans[hlcomment] + '/'+ output + spanend;
			output = "";
			i--;
			cstate = comment;
			break;
		 }
	  }
	  
	  //FOR STRLIT//
	  
	  while( cstate == strlit )
	  {
		 if( previous == readesc )
		 {
		    previous = 0;
			break;
		 }
		 
	     output = output + translateHTMLReserved(s[i]);
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != strlit || i == s.length() )
		 {
		    tagline = tagline + hlspans[hlstrlit] + output + spanend;
			output = "";
			i--;
			cstate = strlit;
			break;
		 }
	  }
	  
	  //FOR READESQ
	  
	  while( cstate == readesc )
	  {
	     if( INSET(s[i + 1],escseq))
		 {
		    tagline = tagline + hlspans[hlescseq] +  s[i] + s[i + 1] + spanend;
		 }
		 else
		 {
		    tagline = tagline + "";
		 }
		 
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != readesc || i == s.length() )
		 {
		    i--;
			cstate = readesc;
			break;
		 }
	  }
	  
	  //FOR SCANNUM///
	  
	  while( cstate == scannum )
	  {
	     output = output + s[i];
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != scannum || i == s.length() )
		 {
		    tagline = tagline + hlspans[hlnumeric] + output + spanend;
			output = "";
			i--;
			cstate = scannum;
			break;
		 }
	  }
	  
	  //FOR ERROR
	  
	  while( cstate == error )
	  {
		 
		 if( previous == readesc )
		 {
		    output = output + '\\';
			previous = 0;
		 }
		 
		 output = output + translateHTMLReserved(s[i]);
		 i++;
		 
		 if( i < s.length() )
		 {
		    updateState(cstate,s[i]);
		 }
		 
		 if( cstate != error || i == s.length() )
		 {
		    tagline = tagline + hlspans[hlerror] + output + spanend;
			i--;
			cstate = error;
			break;
		 }
	  }
   }
   //cout<<tagline;
   return tagline;
}
   
int main() {
	// TODO: write the main program.
	// It may be helpful to break this down and write
	// a function that processes a single line, which
	// you repeatedly call from main().
	
	string my_line = "";
	string my_newline = "";
	
	while(getline(cin,my_line))
	{
	   my_line = my_line + '\n';
	   my_newline = htmlDestroyer(my_line);
	   cout<<my_newline;
	   my_newline = "";
	}
	return 0;
}
