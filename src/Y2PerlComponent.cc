/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (C) SuSE Linux AG |
\----------------------------------------------------------------------/

  File:	      Y2PerlComponent.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#define y2log_component "Y2Perl"
#include <ycp/y2log.h>
#include <ycp/YCPError.h>

#include "Y2PerlComponent.h"
#include "YPerl.h"

using std::string;


Y2PerlComponent::Y2PerlComponent()
{
    // Actual creation of a Perl interpreter is postponed until one of the
    // YPerl static methods is used. They handle that.

    y2milestone( "Creating Y2PerlComponent" );
}


Y2PerlComponent::~Y2PerlComponent()
{
    YPerl::destroy();
}


void Y2PerlComponent::result( const YCPValue & )
{
}



YCPValue
Y2PerlComponent::evaluate( string function, YCPList args )
{
    y2milestone ("YPerl::evaluate (%s, %s)", function.c_str(), args->toString().c_str());

    if ( function == "Call" )	return YPerl::call( args );
    if ( function == "Eval" )	return YPerl::eval( args );
	
    return YCPError( string ( "Undefined Perl::" ) + function );
}
