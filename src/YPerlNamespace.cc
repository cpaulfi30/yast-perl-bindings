/**
 * $Id$
 *
 * This is the path from YCP to Perl.
 */

#include "YPerlNamespace.h"

#include <EXTERN.h>	// Perl stuff
#include <perl.h>

#define y2log_component "Y2Perl"
#include <ycp/y2log.h>

#include <ycp/YCPElement.h>
#include <ycp/YBlock.h>
#include <ycp/YExpression.h>
#include <ycp/Type.h>
#include <ycp/YCPVoid.h>
//#include <YCP.h>
#include <YPerl.h>
#include <stdio.h>

#define AV_FETCH_NON_NULL(svp)				\
    do {						\
    if ((svp) == NULL) {				\
	y2error ("av_fetch returned NULL (%s)", #svp);	\
	return Type::Error;				\
    }							\
    } while (false)

/**
 * using this instead of plain strcmp
 * enables embedding argument names into the typeinfo
 */
static bool firstWordIs (const char *where, const char *what)
{
    size_t n = strlen (what);
    return !strncmp (where, what, n) &&
	(where[n] == '\0' || isspace (where[n]));
}

/**
 * if the typeinfo is wrong, returns Type::Error
 */
static constTypePtr parseTypeinfo (SV *ti)
{
    EMBEDDED_PERL_DEFS;
    if (SvPOK (ti)) // is it a string
    {
	const char *s = SvPV_nolen (ti);
	if      (firstWordIs (s, "any"))
	    return Type::Any;
	else if (firstWordIs (s, "void"))
	    return Type::Void;
	else if (firstWordIs (s, "boolean"))
	    return Type::Boolean;
	else if (firstWordIs (s, "byteblock"))
	    return Type::Byteblock;
	else if (firstWordIs (s, "integer"))
	    return Type::Integer;
	else if (firstWordIs (s, "float"))
	    return Type::Float;
	else if (firstWordIs (s, "string"))
	    return Type::String;
	else if (firstWordIs (s, "locale"))
	    return Type::Locale;
	else if (firstWordIs (s, "path"))
	    return Type::Path;
	else if (firstWordIs (s, "symbol"))
	    return Type::Symbol;
	else if (firstWordIs (s, "term"))
	    return Type::Term;
	else
	{
	    y2error ("Unknown typeinfo %s", s);
	    return Type::Error;
	}
    }
    else if (SvROK (ti)) // is it a reference?
    {
	SV *rti = SvRV (ti);	// dereference it
	if (SvTYPE (rti) == SVt_PVAV) // is it an array?
	{
	    AV *av = (AV *) rti;
	    I32 maxidx = av_len (av);
	    if (maxidx < 1)
	    {
		y2error ("Typeinfo list too short");
		return Type::Error;
	    }
	    I32 i = 0;
	    SV **kind = av_fetch (av, i, 0 /* not an l-value */);
	    AV_FETCH_NON_NULL (kind);

	    if (!SvPOK (*kind))
	    {
		y2error ("Typeinfo list of bad kind");
		return Type::Error;
	    }

	    const char *s = SvPV_nolen (*kind);
	    if (firstWordIs (s, "list"))
	    {
		// maxidx already checked
		SV **val = av_fetch (av, 1, 0);
		AV_FETCH_NON_NULL (val);

		constTypePtr tp_val = parseTypeinfo (*val);
		if (tp_val->isError ())
		{
		    return Type::Error;
		}
		else
		{
		    return new ListType (tp_val);
		}
	    }
	    else if (firstWordIs (s, "map"))
	    {
		if (maxidx != 2)
		{
		    y2error ("Typeinfo for map must have 2 arguments");
		    return Type::Error;
		}
		SV **key = av_fetch (av, 1, 0);
		AV_FETCH_NON_NULL (key);
		SV **val = av_fetch (av, 2, 0);
		AV_FETCH_NON_NULL (val);

		constTypePtr tp_key = parseTypeinfo (*key);
		if (tp_key->isError ())
		{
		    return Type::Error;
		}
		constTypePtr tp_val = parseTypeinfo (*val);
		if (tp_val->isError ())
		{
		    return Type::Error;
		}
		else
		{
		    return new MapType (tp_key, tp_val);
		}
	    }
	    else if (firstWordIs (s, "variable"))
	    {
		// like list
		// maxidx already checked
		SV **val = av_fetch (av, 1, 0);
		AV_FETCH_NON_NULL (val);

		constTypePtr tp_val = parseTypeinfo (*val);
		if (tp_val->isError ())
		{
		    return Type::Error;
		}
		else
		{
		    return new VariableType (tp_val);
		}
	    }
	    else if (firstWordIs (s, "function"))
	    {
		///
		// maxidx already checked
		SV **val = av_fetch (av, 1, 0);
		AV_FETCH_NON_NULL (val);

		constTypePtr tp_val = parseTypeinfo (*val);
		if (tp_val->isError ())
		{
		    return Type::Error;
		}
		FunctionTypePtr func = new FunctionType (tp_val);

		for (i = 2; i <= maxidx; ++i)
		{
		    SV **arg = av_fetch (av, i, 0);
		    AV_FETCH_NON_NULL (arg);

		    constTypePtr tp_arg = parseTypeinfo (*arg);
		    if (tp_arg->isError ())
		    {
			return Type::Error;
		    }
		    func->concat (tp_arg);
		}
		return func;
	    }
	    else
	    {
		y2error ("Unknown list typeinfo %s", s);
		return Type::Error;
	    }
	}
	else
	{
	    y2error ("Typeinfo reference is not an array");
	    return Type::Error;
	}
    }
    else
    {
	y2error ("Typeinfo is neither a scalar nor a reference");
	return Type::Error;
    }
}
#undef AV_FETCH_NON_NULL


/**
 * The definition of a function that is implemented in Perl
 */
class YPerlFunctionDefinition : public YBlock
{
    //! module name
    string m_module_name;
    //! function name, excluding module name
    string m_local_name;
    //! get at everything we want :-|
    SymbolEntryPtr m_symbolentry;

public:
    YPerlFunctionDefinition (const string &module_name,
			     const string &local_name,
			     SymbolEntryPtr se
	) :
	YBlock ("TODOfile", YBlock::b_unknown/*?*/),
	m_module_name (module_name),
	m_local_name (local_name),
	m_symbolentry (se)
	{}

    //! called by YEFunction::evaluate
    virtual YCPValue evaluate (bool cse=false)
    {
	// this should be optimized so that it is fast at runtime. TODO
	if (cse) return YCPNull ();

	YCPList call;
	// placeholder, formerly function name
	call->add (YCPVoid ());
	// parameters
	// the static cast actually calls an operator involving a dynamic cast
	YFunctionPtr m_function = static_cast<YFunctionPtr> (m_symbolentry->code ());
	for (unsigned i = 0; i < m_function->parameterCount (); ++i)
	{
	    call->add (m_function->parameter (i)->value ());
	}
	// wanted return type
	constFunctionTypePtr sig = (constFunctionTypePtr) m_symbolentry->type ();
	return YPerl::yPerl()->callInner (
	    m_module_name, m_local_name, false /* everything as methods? */,
	    call, sig->returnType ());
    }
};


YPerlNamespace::YPerlNamespace (string name)
    : m_name (name)
{
    EMBEDDED_PERL_DEFS;
    const char * c_name = m_name.c_str ();

    const I32 create = 0;

    // stash == Symbol TAble haSH
    HV *stash = gv_stashpv (c_name, create);

    if (stash == NULL)
    {
	y2error ("The Perl package %s is not provided by its pm file", c_name);
	return;
    }

    // iterate thru the stash

    // see man perlfunc/each: Since Perl 5.8.1 the ordering is
    // different even between different runs of Perl for security
    // reasons :-( so we must sort the symbols so that the
    // symbolentries get the same positions

    I32 numsymbols = hv_iterinit (stash);
    // IVdf = Integer Value decimal format
    y2debug ("numsymbols (%s) = %" IVdf, c_name, numsymbols);

    // get sorted list of symbols
    AV *symbols_av = newAV ();
    av_extend (symbols_av, numsymbols);	// prepare room
    while (numsymbols--)
    {
	char *symbol;
	I32 symlen;

	(void) hv_iternextsv (stash, &symbol, &symlen);
	// we could optimize here by putting only interesting symbols
	// into the array and thus reducing the sort time. later.
	av_push (symbols_av, newSVpv (symbol, symlen));
    }
    // strange, sv_cmp (without the Perl_ prefix) should work but doesn't
    sortsv (AvARRAY (symbols_av), av_len (symbols_av) + 1, Perl_sv_cmp);

    // try to get the typeinfo, if provided
    HV *typeinfo = get_hv ((m_name + "::TYPEINFO").c_str (), create);
    //sv_dump ((SV *) typeinfo);

    char *symbol;
    int count = 0;
    STRLEN symlen;
    SV *glob;
    // iterate again, use the array this time.
    // since it will be deleted, we must take copies of the symbols!
    for (I32 i = 0; i <= av_len (symbols_av); ++i)
    {
	symbol = SvPV (* av_fetch (symbols_av, i, create), symlen);
	glob = * hv_fetch (stash, symbol, symlen, create);
	y2debug ("Processing glob %s", symbol);
//	fprintf (stderr, "========================\n");
//	sv_dump (glob);

//	fprintf (stderr, "GvSV: %p, GvAV: %p, GvHV: %p, GvCV: %p\n",
//		 GvSV (glob), GvAV (glob), GvHV (glob), GvCV (glob));

	// is it a code value, a sub?
	if (GvCV (glob))
	{
	    y2debug ("Processing sub %s", symbol);
	    // get the type information or make it up
	    // TODO factorize
	    constTypePtr sym_tp = Type::Unspec;
	    if ((SV *)typeinfo != &PL_sv_undef)
	    {
		SV **sym_ti = hv_fetch (typeinfo, symbol, symlen, create);
		if (sym_ti != NULL)
		{
		    sym_tp = parseTypeinfo (*sym_ti);
		    if (sym_tp->isError ())
		    {
			y2error ("Cannot parse $TYPEINFO{%s}", symbol);
			continue;
		    }
		}
	    }
	    if (sym_tp->isUnspec ())
	    {
		// not a y2warning, because there may be functions
		// pushed from 3rd party Perl modules
		y2debug ("No $TYPEINFO{%s}", symbol);
		sym_tp = new FunctionType (Type::Any);
	    }


	    if (!sym_tp->isFunction ())
	    {
		y2error ("$TYPEINFO{%s} does not specify a function", symbol);
		continue;
	    }

	    //constFunctionTypePtr fun_tp = dynamic_cast<const FunctionType *> (sym_tp);
	    constFunctionTypePtr fun_tp = (constFunctionTypePtr) sym_tp;

//	    constFunctionTypePtr ret_tp = fun_tp->returnType ();

	    // build a function definition

	    // parameter block
	    YBlock *parblock = new YBlock ((Point *)0);
	    unsigned lineno = GvLINE (glob); //perl gives us the location, good

	    static const int namesize = 7; // "arg000\0" should be enough
	    static char argname[namesize];
	    for (int i = 0; i < fun_tp->parameterCount (); ++i)
	    {
		// the name used in debug and error messages
		snprintf (argname, namesize, "arg%d", i);
		parblock->newEntry (
		    argname,
		    SymbolEntry::c_variable,
		    fun_tp->parameterType (i),
		    lineno
		    );
	    }

	    // the function itself
	    YFunction *fun_f = new YFunction (parblock);

	    // symbol entry for the function
	    SymbolEntry *fun_se = new SymbolEntry (
		this,
		count++,	// position. arbitrary numbering. must stay consistent when?
		symbol,		// passed to Ustring, no need to strdup
		SymbolEntry::c_global, // the only way to get it global
		fun_tp,
		fun_f);
	    fun_se->setCategory (SymbolEntry::c_function);

	    // the function definition
	    YBlock *fun_def = new YPerlFunctionDefinition (m_name, symbol, fun_se);
	    fun_f->setDefinition (fun_def);

	    // enter it to the symbol table
	    enterSymbol (fun_se, 0);
	}

    }

    av_undef (symbols_av);
}

YPerlNamespace::~YPerlNamespace ()
{
}

const string YPerlNamespace::filename () const
{
    // TODO improve
    return ".../" + m_name;
}

// this is for error reporting only?
string YPerlNamespace::toString () const
{
    y2error ("TODO");
    return "{\n"
	"/* this namespace is provided in Perl */\n"
	"}\n";
}

// called when running and the import statement is encountered
// does initialization of variables
// constructor is handled separately after this
YCPValue YPerlNamespace::evaluate (bool cse)
{
    // so we don't need to do anything
    y2debug ("Doing nothing");
    return YCPNull ();
}

// It seems that this is the standard implementation. why would we
// ever want it to be different?
Y2Function* YPerlNamespace::createFunctionCall (const string name)
{
    TableEntry *func_te = table ()->find (name.c_str (), SymbolEntry::c_function);
    if (func_te)
    {
	return new YEFunction (func_te->sentry ());
    }
    y2error ("No such function %s", name.c_str ());
    return NULL;
}
