// -*- c++ -*-
#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>
#include <ycp/YStatement.h>

/**
 * YaST interface to a Perl module
 */
class YPerlNamespace : public Y2Namespace
{
private:
    string m_name;		//! this namespace's name, eg. XML::Writer
    string m_timestamp;		//! ts of the file we were loaded from
    SymbolTable * m_table;	//! public symbols
    unsigned m_count;		//! number of public symbols
    //! maps integers to pointers
    // pointer -> integer is slow, ok when compiling (?)
    // integer -> pointer fast, ok when loading
    // does not own the pointers, m_table does
    vector<SymbolEntry *> m_positions;
public:
    /**
     * Construct an interface. The module must be already loaded
     * @param name eg "XML::Writer"
     * @param timestamp remember it so that it can be saved to bytecode
     */
    YPerlNamespace (string name, string timestamp);

    virtual ~YPerlNamespace ();

    //! what namespace do we implement
    virtual const string name () const { return m_name; }
    //! used for error reporting
    virtual const string filename () const;

    //! somehow needed for function declarations ?!
    virtual unsigned int symbolCount ();

    //! function parameters ??
    // bytecode uses unsigneds
    virtual SymbolEntry* symbolEntry (unsigned int position);
    // bytecode uses unsigneds
    virtual int findSymbol (const SymbolEntry *entry);

    //! unparse. useful  only for YCP namespaces??
    virtual string toString () const;
    //! called when evaluating the import statement
    // constructor is handled separately
    virtual YCPValue evaluate (bool cse = false);

    //! get our whole symbol table?
    virtual SymbolTable* table ();

    /**
     * A method to get a unique timestamp for the file (for example MD5),
     * from which
     * was this namespace loaded/created. The interpreter will
     * request exactly the same timestamp when reloading the file
     * next time
     * @return a string value containing the unique timestamp
     */
    virtual const string timestamp () const;

    virtual Y2Function* createFunctionCall (const string name);
};
