// 
//  Copyright (c) 2003 Tweak Films
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License as
//  published by the Free Software Foundation; either version 2 of the
//  License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
// 
#include <Gto/Reader.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include <fnmatch.h>

using namespace std;

bool outputStrings  = false;
bool outputData     = false;
bool outputHeader   = true;
bool formatData     = false;
bool readAll        = false;
bool numericStrings = false;
bool filtered       = false;
bool outputInterp   = false;

typedef set<const Gto::Reader::PropertyInfo*> PropertySet;
typedef set<const Gto::Reader::ObjectInfo*>   ObjectSet;
PropertySet filteredProperties;
ObjectSet filteredObjects;


class Reader : public Gto::Reader
{
public:
    Reader(unsigned int mode) : 
        Gto::Reader(mode), 
        m_objectName(Gto::uint32(-1)),
        m_componentName(Gto::uint32(-1)) {}

    Gto::uint32		m_objectName;
    Gto::uint32		m_componentName;
    vector<char>        m_buffer;
    typedef Gto::Reader::Request Request;

    void		headerOutput(const Gto::Reader::PropertyInfo&);
    void		outputPropertyHeader(const Gto::Reader::PropertyInfo &);

    virtual void	header(const Gto::Header&);

    virtual Request	object(const std::string& name,
			       const std::string& protocol,
			       unsigned int protocolVersion,
			       const Gto::Reader::ObjectInfo &header);

    virtual Request	component(const std::string& name,
                                  const std::string& interp,
				  const Gto::Reader::ComponentInfo &header);

    virtual Request	property(const std::string& name,
                                 const std::string& interp,
				 const Gto::Reader::PropertyInfo &header);

    virtual void*	data(const PropertyInfo&, size_t bytes);
    virtual void        dataRead(const PropertyInfo&);

   virtual void	data(const PropertyInfo&, 
		     const float*, size_t numItems);

   virtual void	data(const PropertyInfo&, 
		     const double*, size_t numItems);

   virtual void	data(const PropertyInfo&, 
		     const int*, size_t numItems);

   virtual void	data(const PropertyInfo&, 
		     const unsigned short*, size_t numItems);

   virtual void	data(const PropertyInfo&, 
		     const unsigned char*, size_t numItems);

   virtual void	data(const PropertyInfo&, bool);

};


void
Reader::headerOutput(const Gto::Reader::PropertyInfo& pinfo)
{
    const Gto::Reader::ComponentInfo* cinfo = pinfo.component;
    const Gto::Reader::ObjectInfo*    oinfo = cinfo->object;

    if (m_objectName != oinfo->name)
    {
	//
	//  Output the object header information
	//

	m_objectName = oinfo->name;
        m_componentName = 0;

	cout << "object \""
	     << stringFromId(m_objectName)
	     << "\" protocol \""
	     << stringFromId(oinfo->protocolName)
	     << "\" v"
	     << oinfo->protocolVersion
	     << endl;
    }

    if (m_componentName != cinfo->name)
    {
	//
	//  Output the component header information
	//

	m_componentName = cinfo->name;

	cout << "  component \""
	     << stringFromId(m_componentName)
	     << "\"";

        if (outputInterp && stringFromId(cinfo->interpretation) != "")
        {
            cout << " interpret as \"" 
                 << stringFromId(cinfo->interpretation)
                 << "\" ";
        }

        cout << endl;
    }
}

void
Reader::header(const Gto::Header &header)
{
    union EndianTest
    { 
	unsigned char c[4]; 
	unsigned int i; 
    };

    EndianTest x;
    x.i = 1;
    bool big = x.c[3] == 1;

    if (outputHeader)
    {
	cout << "GTO file version " << header.version
	     << ", " << header.numStrings << " strings, ";

	if (big)
	{
	    if (isSwapped()) cout << "little endian\n";
	    else cout << "big endian\n";
	}
	else
	{
	    if (isSwapped()) cout << "big endian\n";
	    else cout << "little endian\n";
	}
    }
}

Reader::Request
Reader::object(const std::string &name,
	       const std::string &protocol,
	       unsigned int version,
	       const Gto::Reader::ObjectInfo &info)
{
    if (outputStrings)
    {
	const StringTable &strings = stringTable();
	for (int i=0; i < strings.size(); i++)
	{
	    if (i && !formatData) cout << ", ";
	    cout << i << ": \"" << strings[i] << "\"";
	    if (formatData) cout << endl;
	}

	cout << endl;
	exit(0);
    }

    return Request(true);
}

Reader::Request
Reader::component(const std::string &n,
                  const std::string &i,
		  const Gto::Reader::ComponentInfo& c)
{
    return Request(true);
}

Reader::Request
Reader::property(const std::string&, 
                 const std::string&,
                 const Gto::Reader::PropertyInfo &info)
{
    if (outputHeader)
    {
	outputPropertyHeader(info);
    }

    if (filtered)
    {
        return Request(filteredProperties.find(&info) 
                       != filteredProperties.end());
    }
    else
    {
        return Request(true);
    }
}

void
Reader::outputPropertyHeader(const Gto::Reader::PropertyInfo &info)
{
    headerOutput(info);

    const char* type;
    
    switch (info.type)
    {
      case Gto::Int:	 type = "int"; break;
      case Gto::Float:	 type = "float"; break;
      case Gto::Double:  type = "double"; break;
      case Gto::Half:	 type = "half"; break;
      case Gto::String:  type = "string"; break;
      case Gto::Short:	 type = "short"; break;
      case Gto::Byte:	 type = "byte"; break;
      case Gto::Boolean: type = "bool"; break;
      default:		 type = "unknown"; break;
    }

    cout << "    property " << type
	 << "[" << info.width << "]"
	 << "[" << info.size << "]"
	 << " \"" 
	 << stringFromId(info.name)
	 << "\"";

    if (outputInterp && stringFromId(info.interpretation) != "")
    {
        cout << " interpret as \"" 
             << stringFromId(info.interpretation)
             << "\" ";
    }
    
    cout << endl;
}

void*
Reader::data(const PropertyInfo& info, size_t bytes)
{
    if (bytes)
    {
        m_buffer.resize(bytes);
        return &m_buffer.front();
    }
    else
    {
        switch (info.type)
        {
          case Gto::Float:
              data(info, (float*)0, info.size);
              break;
          case Gto::Double:
              data(info, (double*)0, info.size);
              break;
          case Gto::Int:
          case Gto::String:
              data(info, (int*)0, info.size);
              break;
          case Gto::Short:
              data(info, (unsigned short*)0, info.size);
              break;
          case Gto::Byte:
              data(info, (unsigned char*)0, info.size);
              break;
          case Gto::Boolean:
              break;
        }
    }
}

void
Reader::dataRead(const PropertyInfo& info)
{
    void* buffer = (void*)&m_buffer.front();

    switch (info.type)
    {
      case Gto::Float:
          data(info, (float*)buffer, info.size);
          break;
      case Gto::Double:
          data(info, (double*)buffer, info.size);
          break;
      case Gto::Int:
      case Gto::String:
          data(info, (int*)buffer, info.size);
          break;
      case Gto::Short:
          data(info, (unsigned short*)buffer, info.size);
          break;
      case Gto::Byte:
          data(info, (unsigned char*)buffer, info.size);
          break;
      case Gto::Boolean:
          break;
    }
}

void Reader::data(const PropertyInfo& info, const float* data, size_t numItems)
{
    if (!outputData) return;

    cout << "float[" << info.width << "] "
	 << stringFromId(info.component->object->name)
	 << "."  << stringFromId(info.component->name)
	 << "." << stringFromId(info.name)
	 << " = ";

    if (formatData) 
    {
	cout << endl << "[" << endl;
    }
    else
    {
	cout << "[";
    }

    for (int i=0; i < numItems; i++)
    {
	if (formatData) cout << "    ";

        if (info.width > 1 )
        {
            cout << " [";
            for( int j = 0; j < info.width; ++j )
            {
            	cout << " " << data[(i*info.width)+j];
            }
            cout << " ]";
        }
        else
        {
	    cout << " " << data[i];
        }
	
	if (formatData) cout << endl;
    }

    cout << (formatData ? "" : " ") << "]" << endl;
}

void Reader::data(const PropertyInfo& info, const double* data, size_t numItems)
{
    if (!outputData) return;

    cout << "double[" << info.width << "] "
	 << stringFromId(info.component->object->name)
	 << "."  << stringFromId(info.component->name)
	 << "." << stringFromId(info.name)
	 << " = ";

    if (formatData) 
    {
	cout << endl << "[" << endl;
    }
    else
    {
	cout << "[";
    }

    for (int i=0; i < numItems; i++)
    {
	if (formatData) cout << "    ";

        if (info.width > 1 )
        {
            cout << " [";
            for( int j = 0; j < info.width; ++j )
            {
            	cout << " " << data[(i*info.width)+j];
            }
            cout << " ]";
        }
        else
        {
	    cout << " " << data[i];
        }
	
	if (formatData) cout << endl;
    }

    cout << (formatData ? "" : " ") << "]" << endl;
}

void Reader::data(const PropertyInfo& info, const int* data, size_t numItems)
{
    if (!outputData) return;

    if (info.type == Gto::Int)
    {
	cout << "int[" 
	     << info.width << "] "
	     << stringFromId(info.component->object->name)
	     << "."  << stringFromId(info.component->name)
	     << "." << stringFromId(info.name)
	     << " = ";

	if (formatData)
	{
	    cout << endl << "[" << endl;
	}
	else
	{
	    cout << "[";
	}

	for (int i=0; i < numItems; i++)
	{
	    if (formatData) cout << "    ";

	    if (info.width > 1)
	    {
		cout << " [";
		for( int j = 0; j < info.width; ++j )
		{
		    cout << " " << data[(i*info.width)+j];
		}
		cout << " ]";
	    }
	    else
	    {
        	cout << " " << data[i];
	    }
	    
	    if (formatData) cout << endl;
	}
    }
    else
    {
	cout << "string[" 
	     << info.width << "] "
	     << stringFromId(info.component->object->name)
	     << "."  << stringFromId(info.component->name)
	     << "." << stringFromId(info.name)
	     << " = ";

	if (formatData)
	{
	    cout << endl << "[" << endl;
	}
	else
	{
	    cout << "[";
	}

	for (int i=0; i < numItems; i++)
	{
	    if (formatData) cout << "    ";
	    
	    if( info.width > 1 )
	    {
		cout << " [";
		for( int j = 0; j < info.width; ++j )
		{
                    if (numericStrings)
                    {
                        cout << " " << data[(i*info.width)+j];
                    }
                    else
                    {
                        cout << " \"" << stringFromId(data[(i*info.width)+j]);
                        cout << "\"";
                    }
		}
		cout << " ]";
	    }
	    else
	    {
                if (numericStrings)
                {
                    cout << " " << data[i];
                }
                else
                {
                    cout << " \"" << stringFromId(data[i]) << "\"";
                }
	    }
	    
	    if (formatData) cout << endl;
	}
    }

    cout << (formatData ? "" : " ") << "]" << endl;
}

void Reader::data(const PropertyInfo& info, 
		  const unsigned short* data, 
		  size_t numItems)
{
    if (!outputData) return;

    if (info.type == Gto::Short)
    {
	cout << "short[" 
	     << info.width 
	     << "] "
	     << stringFromId(info.component->object->name)
	     << "."  << stringFromId(info.component->name)
	     << "." << stringFromId(info.name)
	     << " = ";

	if (formatData)
	{
	    cout << endl << "[" << endl;
	}
	else
	{
	    cout << "[";
	}


	for (int i=0; i < numItems; i++)
	{
	    if (formatData) cout << "    ";
	    
	    if( info.width > 1 )
	    {
		cout << " [";
		for( int j = 0; j < info.width; ++j )
		{
		    cout << " " << data[(i*info.width)+j];
		}
		cout << " ]";
	    }
	    else
	    {
        	cout << " " << data[i];
	    }

	    if (formatData) cout << endl;
	}
    }

    cout << (formatData ? "" : " ") << "]" << endl;
}

void Reader::data(const PropertyInfo& info, 
		  const unsigned char* data, 
		  size_t numItems)
{
    if (!outputData) return;

    if (info.type == Gto::Byte)
    {
	cout << "byte[" 
	     << info.width 
	     << "] "
	     << stringFromId(info.component->object->name)
	     << "."  << stringFromId(info.component->name)
	     << "." << stringFromId(info.name)
	     << " = ";

	if (formatData)
	{
	    cout << endl << "[" << endl;
	}
	else
	{
	    cout << "[";
	}

	for (int i=0; i < numItems; i++)
	{
	    if (formatData) cout << "    ";

	    if( info.width > 1 )
	    {
		cout << " [";
		for( int j = 0; j < info.width; ++j )
		{
		    cout << " " << int(data[(i*info.width)+j]);
		}
		cout << " ]";
	    }
	    else
	    {
        	cout << " " << int(data[i]);
	    }

	    if (formatData) cout << endl;
	}
    }

    cout << (formatData ? "" : " ") << "]" << endl;
}

void Reader::data(const PropertyInfo& info, bool)
{
    if (!outputData) return;
}

//----------------------------------------------------------------------

void filterData(const std::string& filterExpr, Reader& reader)
{
    Gto::Reader::Properties& properties = reader.properties();
    Gto::Reader::Objects& objects = reader.objects();

    for (int i=0; i < properties.size(); i++)
    {
        Gto::Reader::PropertyInfo&  p = properties[i];
        const Gto::Reader::ComponentInfo* c = p.component;
        const Gto::Reader::ObjectInfo*    o = c->object;

        string name;
        name = reader.stringFromId(o->name);
        name += ".";
        name += reader.stringFromId(c->name);
        name += ".";
        name += reader.stringFromId(p.name);

        if (!fnmatch(filterExpr.c_str(), name.c_str(), 0))
        {
            filteredProperties.insert(&p);
            filteredObjects.insert(o);
        }
    }
    
    for (int i=0; i < objects.size(); i++)
    {
        Gto::Reader::ObjectInfo* p = &objects[i];

        if (filteredObjects.find(p) != filteredObjects.end())
        {
            reader.accessObject(*p);
        }
    }
}

//----------------------------------------------------------------------

void usage()
{
    cout << "gtoinfo [options] file.gto\n"
         << endl
         << "-a/--all                       dump data and header\n"
         << "-d/--dump                      dump data (no header)\n"
         << "-l/--line                      dump data/strings one per line\n"
         << "-h/--header                    header only (default)\n"
         << "-s/--strings                   output strings table\n"
         << "-n/--numeric-strings           output string properties as string numbers\n"
         << "-i/--interpretation-strings    output interpretation strings\n"
         << "-f/--filter expr               filter shell-like expression\n"
         << "-r/--readall                   force data read\n"
         << "--help                         usage\n"
         << endl;
    
    exit(-1);
}


int main(int argc, char *argv[])
{
    const char *inFile = 0;
    string filterExpr;

    for (int i=1; i < argc; i++)
    {
	const char *arg = argv[i];

	if (*arg == '-')
	{
	    if (!strcmp(arg, "-d") ||
		!strcmp(arg, "--dump"))
	    {
		outputData   = true;
		outputHeader = false;
	    }
	    else if (!strcmp(arg, "-a") ||
		     !strcmp(arg, "--all"))
	    {
		outputData   = true;
		outputHeader = true;
                outputInterp = true;
	    }
	    else if (!strcmp(arg, "-h") ||
		     !strcmp(arg, "--header"))
	    {
		outputData   = false;
		outputHeader = true;
	    }
	    else if (!strcmp(arg, "-s") ||
		     !strcmp(arg, "--strings"))
	    {
		outputData   = false;
		outputHeader = false;
		outputStrings = true;
	    }
	    else if (!strcmp(arg, "-r") ||
		     !strcmp(arg, "--readall"))
	    {
		readAll      = true;
	    }
	    else if (!strcmp(arg, "-l") ||
		     !strcmp(arg, "--line"))
	    {
		formatData   = true;
	    }
            else if (!strcmp(arg, "-n") ||
                     !strcmp(arg, "--numeric-strings"))
            {
                numericStrings = true;
            }
            else if (!strcmp(arg, "-i") ||
                     !strcmp(arg, "--interpretation-strings"))
            {
                outputInterp = true;
            }
            else if (!strcmp(arg, "-f") ||
                     !strcmp(arg, "--filter-expression"))
            {
                i++;
                filterExpr = argv[i];
                filtered = true;
            }
	    else
	    {
                usage();
	    }
	}
	else
	{
	    inFile = arg;
	}
    }

    if (!inFile)
    {
	cout << "no input .gto file specified.\n" << flush;
        cout << endl;
        usage();
    }

    unsigned int mode = 0;
    if (!outputData && !readAll) mode |= Gto::Reader::HeaderOnly;

    if (filterExpr != "") mode = Gto::Reader::RandomAccess;

    Reader reader(mode);

    if ( !reader.open(inFile) )
    {
	cerr << "Error reading file " << inFile << endl;
    }

    if (filterExpr != "")
    {
        filterData(filterExpr, reader);
    }

    return 0;
}
