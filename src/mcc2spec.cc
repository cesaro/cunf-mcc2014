
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "mcc-properties.hh"
#include "util/config.h"
#include "util/misc.h"

void cannot ()
{
		printf ("CANNOT_COMPUTE\n");
		exit (EXIT_SUCCESS);
}

void do_not ()
{
		printf ("DO_NOT_COMPETE\n");
		exit (EXIT_SUCCESS);
}

void translate_predicate (const xml_schema::type & f, std::string & out)
{
	SHOW (typeid (f) == typeid (mcc::conjunction), "d");
	SHOW (typeid (f) == typeid (mcc::disjunction), "d");
	SHOW (typeid (f) == typeid (mcc::is_fireable), "d");

	if (typeid (f) == typeid (mcc::is_fireable))
	{
		const mcc::is_fireable & f1 = (mcc::is_fireable &) f;
		auto tseq = f1.transition ();
		out += "( \"";
		for (auto it = tseq.begin (); it != tseq.end (); ++it)
		{
			// *it is xml_schema::idref, which inherits from std::basic_string
			if (it != tseq.begin ()) out += "|| \"";
			out += *it + "\" ";
		}
		out += ")";
	}
	else if (typeid (f) == typeid (mcc::disjunction))
	{
		const mcc::disjunction & f1 = (mcc::disjunction &) f;
		auto fseq = f1.boolean_formula ();
		out += "( ";
		for (auto it = fseq.begin (); it != fseq.end (); ++it)
		{
			// *it is xml_schema::type
			if (it != fseq.begin ()) out += "|| \"";
			translate_predicate (*it, out);
		}
		out += ")";
	}
	else if (typeid (f) == typeid (mcc::conjunction))
	{
		const mcc::conjunction & f1 = (mcc::conjunction &) f;
		auto fseq = f1.boolean_formula ();
		out += "( ";
		for (auto it = fseq.begin (); it != fseq.end (); ++it)
		{
			// *it is xml_schema::type
			if (it != fseq.begin ()) out += "&& \"";
			translate_predicate (*it, out);
		}
		out += ")";
	}
	else
	{
		do_not ();
	}
}

void translate_formula (const xml_schema::type & f, std::string & out, bool & negate)
{
	SHOW (typeid (f) == typeid (mcc::invariant), "d");
	SHOW (typeid (f) == typeid (mcc::impossibility), "d");
	SHOW (typeid (f) == typeid (mcc::possibility), "d");

	negate = false;

	if (typeid (f) == typeid (mcc::invariant))
	{
		const mcc::invariant & f1 = (mcc::invariant &) f;
		if (typeid (f1.boolean_formula ()) != typeid (mcc::deadlock)) do_not ();
		out = "! deadlock";
		negate = true;
	}
	else if (typeid (f) == typeid (mcc::impossibility))
	{
		const mcc::impossibility & f1 = (mcc::impossibility &) f;
		if (typeid (f1.boolean_formula ()) == typeid (mcc::deadlock))
		{
			out = "deadlock";
		}
		else
		{
			translate_predicate (f1.boolean_formula (), out);
		}
		negate = true;
	}
	else if (typeid (f) == typeid (mcc::possibility))
	{
		const mcc::possibility & f1 = (mcc::possibility &) f;
		if (typeid (f1.boolean_formula ()) == typeid (mcc::deadlock))
			out = "deadlock";
		else
			translate_predicate (f1.boolean_formula (), out);
	}
	else
	{
		translate_predicate (f, out);
	}
}

int main (int argc, char ** argv)
{
	if (argc != 1) {
		PRINT ("Usage: mcc2spec < XMLFILE > SPECFILE");
		return EXIT_FAILURE;
	}

	// parse the XML file, pset is the memory representation of it
	std::auto_ptr<mcc::property_set> pset;
	try {
		pset = mcc::property_set_ (std::cin, "(stdin)", xml_schema::flags::dont_validate);
	}
	catch (std::ifstream::failure & e) {
		PRINT ("mcc2spec: cannot read standard input");
		return EXIT_FAILURE;
	}
	catch (xml_schema::exception & e) {
		std::cerr << e << std::endl;
		return EXIT_FAILURE;
	}

	// for each formula in the file ...
	for (auto p = pset->property().begin(); p != pset->property().end(); p++)
	{
		// reject the entire file on the first non-boolean formula
		auto & f = p->formula ();
		auto & bf = f.boolean_formula();
		if (! bf.present()) do_not ();

		// translate the formula to Cunf's spec format
		std::string s;
		bool negate;
		translate_formula (bf.get (), s, negate);

		printf ("# %s %s\n%s;\n",
				negate ? "Y" : "N",
				p->id().c_str(), 
				s.c_str ());
	}
	exit (EXIT_SUCCESS);
}

