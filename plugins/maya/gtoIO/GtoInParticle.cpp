//
//  Copyright (c) 2009, Tweak Software
//  All rights reserved.
// 
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the following
//       disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
//     * Neither the name of the Tweak Software nor the names of its
//       contributors may be used to endorse or promote products
//       derived from this software without specific prior written
//       permission.
// 
//  THIS SOFTWARE IS PROVIDED BY Tweak Software ''AS IS'' AND ANY EXPRESS
//  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL Tweak Software BE LIABLE FOR
//  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
//  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
#include "version.h"

#include "GtoInParticle.h"
#include <maya/MGlobal.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <float.h>
#include <algorithm>

namespace GtoIOPlugin {
using namespace std;

//******************************************************************************
Particle::Particle( const std::string &name, 
                    const std::string &protocol, 
                    const unsigned int protocolVersion )
    : Object( name, protocol, protocolVersion ),
      m_position(0),
      m_numParticles(0)
{
    // Nothing
}

Particle::~Particle()
{
    for (size_t i=0; i < m_attributes.size(); i++)
    {
        delete m_attributes[i];
    }
}

Request Particle::component( const std::string &name ) const
{
    if( name == "points" )
    {
        return Request( true, (void*)POINTS_C );
    }

    return Object::component( name );
}

Request Particle::property( const std::string &name,
                            void *componentData ) const
{
    if ( (( int )componentData) == POINTS_C )
    {
        Attribute *a = new Attribute( name );
        return Request( true, (void *)a );
    }

    return Object::property( name, componentData );
}

void *Particle::data( const PropertyInfo &pinfo, 
                      size_t bytes,
                      void *componentData,
                      void *propertyData )
{
    if (int(componentData) != POINTS_C) return NULL;
    if (m_numParticles == 0) m_numParticles = pinfo.size;
    Attribute *a = reinterpret_cast<Attribute*>(propertyData);

    if (m_numParticles != pinfo.size) 
    {
        delete a;
        return NULL;
    }

    a->width = pinfo.width;
    a->size  = pinfo.size;
    a->floatData = new float[pinfo.size * pinfo.width];

    if( a->name == GTO_PROPERTY_POSITION )
    {
        m_position = a;
    }

    m_attributes.push_back(a);
    
    return a->floatData;
}

void Particle::dataRead( const PropertyInfo &pinfo,
                       void *componentData,
                       void *propertyData,
                       const StringTable &strings )
{
    // Nothing
}


void
Particle::declareMaya()
{
    MFnDagNode particle;
    MStatus status;

    //
    //        If you use the create functions that takes a typeid it will
    //        fail for some reason.
    //

    m_mayaParentObject = particle.create( "particle",
                                          MObject::kNullObj, 
                                          &status );
    if ( status != MS::kSuccess )
    {
        status.perror( "Particle object creation failed" );
        MGlobal::displayError(status.errorString());
        m_mayaObject = MObject::kNullObj;
        m_mayaParentObject = MObject::kNullObj;
        return;
    }

    //
    // Now use that transform node we got earlier to store a handle to
    // the real geometry node.
    //

    MFnDagNode parentDN( m_mayaParentObject );
    m_mayaObject = parentDN.child( 0 );
    MFnDependencyNode shape(m_mayaObject);
    setName();
    addToDefaultSG();

    //
    //        Jesus H. Christ, why can there not be an MFnParticle class?
    //
    // addAttr -ln foobar0 -dt vectorArray  |particle1|particleShape1;
    // addAttr -ln foobar -dt vectorArray  |particle1|particleShape1;

    for (size_t i=0; i < m_attributes.size(); i++)
    {
        Attribute *a = m_attributes[i];

        //
        //  Add the attributes
        //

        for (int q=0; q < 2; q++)
        {
            MString command = "addAttr -ln ";
            command += a->name.c_str();
            if (!q) command += "0";
            command += " -dt ";

            if (a->width == 3) command += "vectorArray";
            else if (a->width == 1) command += "doubleArray";
            else continue;

            command += " ";
            command += parentDN.fullPathName();
            command += "|";
            command += shape.name();
            command += ";";

            if (q)
            {
                command += "setAttr -e -keyable true ";
                command += parentDN.fullPathName();
                command += "|";
                command += shape.name();
                command += ".";
                command += a->name.c_str();
                command += ";";
            }

            MGlobal::executeCommand(command, false, false);
        }
    }

    MString command;

    for (size_t i=0; i < m_numParticles; i++)
    {
        if ( !(i % 512) )
        {
            if (i)
            {
                command += ";";
                MGlobal::executeCommand(command, false, false);
            }

            command = "emit -o ";
            command += parentDN.fullPathName();
            command += "|";
            command += shape.name();
        }

        command += " -pos ";
        command += double(m_position->floatData[i*3 + 0]); 
        command += " ";
        command += double(m_position->floatData[i*3 + 1]);
        command += " ";
        command += double(m_position->floatData[i*3 + 2]);
        command += " ";

        for (size_t q=0; q < m_attributes.size(); q++)
        {
            Attribute *a = m_attributes[q];

            if (a->width == 1)
            {
                command += " -at ";
                command += a->name.c_str();
                command += " -fv ";
                command += double(a->floatData[i]);
            }
            else if (a->width == 3)
            {
                command += " -at ";
                command += a->name.c_str();
                command += " -vv ";
                command += double(a->floatData[i*3 + 0]);
                command += " ";
                command += double(a->floatData[i*3 + 1]);
                command += " ";
                command += double(a->floatData[i*3 + 2]);
                command += " ";
            }
        }
    }

    command += ";";
    MGlobal::executeCommand(command, false, false);

    command = "saveInitialState ";
    command += parentDN.fullPathName();
    command += "|";
    command += shape.name();
    command += ";";
    command += "connectAttr time1.outTime ";
    command += parentDN.fullPathName();
    command += "|";
    command += shape.name();
    command += ".currentTime;";
    MGlobal::executeCommand(command, false, false);
}

} // End namespace GtoIOPlugin
