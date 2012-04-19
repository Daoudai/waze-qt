/*
 * LICENSE:
 *
 *   Copyright 2008 PazO
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef  __WEBSVC_ADDRESS_H__
#define  __WEBSVC_ADDRESS_H__
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#include "../websvc_trans/websvc_address_defs.h"
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"   {
#endif   //   __cplusplue
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
BOOL  WSA_IsValid(   const char* szWebServiceAddress);

BOOL  WSA_ExtractParams(   
            const char* szWebServiceAddress, //   IN        -   Web service full address (http://...)
            char*       pServerURL,          //   OUT,OPT   -   Server URL[:Port]
            int*        pServerPort,         //   OUT,OPT   -   Server Port
            char*       pServiceName);       //   OUT,OPT   -   Web service name

void  WSA_RemovePortNumberFromURL(
            char*       szWebServiceAddress);//   IN,OUT    -   Web service full address (http://...)
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef   __cplusplus
};
#endif   // __cplusplue
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
#endif   // __WEBSVC_ADDRESS_H__

