
using System;
using System.Text;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;

class gen {
	static void Main ()
	{
		ArrayList classes = new ArrayList ();
		Hashtable bases = new Hashtable ();
		Hashtable cctors = new Hashtable ();
		Hashtable contprops = new Hashtable ();

		GetClasses (classes, bases, cctors, contprops);

		GenerateTypeCpp (classes, bases, cctors, contprops);
		GenerateValueH (classes, bases);	
		GenerateValueCpp (classes, bases);
		GenerateTypeH (classes, bases);
		GenerateKindCs (classes, bases);

		CheckGetObjectType (classes, bases);
	}

	static void CheckGetObjectType (ArrayList classes, Hashtable bases)
	{
		string [] files = Directory.GetFiles (Environment.CurrentDirectory, "*.h");
		StringBuilder all = new StringBuilder ();
		string contents;

		foreach (string file in files) {
			all.AppendLine (File.ReadAllText (file));
		}
		all.AppendLine ("class : ;");
		contents = all.ToString ();
		contents = RemoveComments (contents);

		foreach (string c in classes) {
			int a, b;
			int start = 0;
			int next;
	
			if (c == "DependencyObject")
				continue;

			do {
				start = contents.IndexOf (Environment.NewLine + "class " + c + " ", start);
				next = contents.IndexOf (Environment.NewLine + "class ", start + 10);
				
				if (start == -1) {
					Console.WriteLine ("Could not find the class " + c);
					break;
				}

				a = contents.IndexOf (":", start);
				b = contents.IndexOf (";", start);
				if (a > b)
					start++;
			} while (start >= 0 && a > b);

			if (start == -1)
				continue;

			if (contents.IndexOf ("Type::Kind GetObjectType", start, next - start) == -1) {
				Console.WriteLine ("Warning: The class '{0}' does not seem to have an implementation of GetObjectType.", c);
				//Console.WriteLine (">Code Checked:");
				//Console.WriteLine (contents.Substring (start, next - start));	
			} else if (contents.IndexOf ("Type::" + getU (c), start, next - start) == -1) {
				Console.WriteLine ("Warning: The method '{0}::GetObjectType' does not seem to return the correct type (didn't find 'Type::{1}' anywhere within the headers).", c, getU (c));
			} else {
				//Console.WriteLine ("OK: " + c);
			}
		}
	}

	static void GetClasses (ArrayList classes, Hashtable bases, Hashtable cctors, Hashtable contprops)
	{
		string [] files = Directory.GetFiles (Environment.CurrentDirectory, "*.h");
		ArrayList tmp = new ArrayList ();
		string next_content_property = null;

		foreach (string file in files) {
			string [] lines = File.ReadAllLines (file);
			foreach (string line in lines) {
				string l = line;

				// This could probably be replaced by a regexp
				if (!l.Trim ().StartsWith ("class ")) {


					Match m = Regex.Match (l, "@ContentProperty\\s*=\\s*\"(.*)\"");
					if (m.Success) {
						next_content_property = m.Groups [1].Value;
					}


                                        if (Regex.IsMatch (l, "_new\\s*\\(\\s*(void)?\\s*\\);")) {
                                                MapClassNameToCCtor (l.Trim (), cctors);
                                        }
					continue;
				}

				if (!l.Contains (":"))
					continue;

				while (l.Contains ("/*"))
					l = l.Replace (l.Substring (l.IndexOf ("/*"), l.IndexOf ("*/") - l.IndexOf ("/*") + 2), "");

				l = l.Replace ("{", "");
				l = l.Replace ("class ", "");
				l = l.Replace (":", "");
				l = l.Replace (";", "");
				l = l.Replace ("public", "");
				while (l.Contains ("  "))
					l = l.Replace ("  ", "");

				string c, p;
				if (l.Contains (" ")) {
					c = l.Substring (0, l.IndexOf (" "));
					p = l.Substring (l.IndexOf (" ") + 1);
				} else {
					c = l;
					p = "";
				}
				if (tmp.Contains (c)) {
					//Console.WriteLine ("Already added " + c);
				} else {
					tmp.Add (c.Trim ());
					if (next_content_property != null) {
						contprops [c.Trim ()] = next_content_property;
					}
				}
				if (p != null && p != string.Empty)  {
					if (bases.ContainsKey (c.Trim())) {
						Console.WriteLine ("Adding {0} -> {1} more than once to `bases' collection",
								   c.Trim (), p.Trim ());
					}
					else {
						bases.Add (c.Trim (), p.Trim ());
					}
				}

				next_content_property = null;
			}
		}
		tmp.Sort ();
		foreach (string c in tmp) {
			if (!bases.ContainsKey (c))
				continue;

			// Check that the class inherits from DO
			string pp = c;	
			while (pp != "DependencyObject") {
				if (!bases.ContainsKey (pp))
					break;
				pp = (string) bases [pp];
			}
			if (pp != "DependencyObject")
				continue;

			classes.Add (c);
		}
		// DO has to be the first class in the list, the rest are sorted alphabetically
		classes.Remove ("DependencyObject");
		classes.Insert (0, "DependencyObject"); 
	}

	static void GenerateTypeH (ArrayList classes, Hashtable bases)
	{
		const string file = "type.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");
		
		text = new StringBuilder ();
		foreach (string c in classes) {
			text.AppendLine ("\t\t" + getU (c) + ",");
		}
		contents = contents.Replace ("/*DO_KINDS*/", text.ToString ());

		text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from type.h.in, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine ("*/");
		contents = text.ToString () + contents;

		File.WriteAllText (file, contents);
	}

	static void GenerateKindCs (ArrayList classes, Hashtable hash)
	{
		const string file = "type.h";
		StringBuilder text = new StringBuilder ();
		string contents = File.ReadAllText (file);
		int a = contents.IndexOf ("// START_MANAGED_MAPPING") + "// START_MANAGED_MAPPING".Length;
		int b = contents.IndexOf ("// END_MANAGED_MAPPING");
		string values = contents.Substring (a, b - a);		

		text.AppendLine ("/* this file was autogenerated from moon/src/value.h.  do not edit this file \n */");
		text.AppendLine ("namespace Mono {");
		text.AppendLine ("\tpublic enum Kind {");
		text.AppendLine (values);
		text.AppendLine ("\t}");
		text.AppendLine ("}");

		File.WriteAllText ("Kind.cs", text.ToString ());

		string realfile = "../../olive/class/agmono/Mono/Kind.cs";
		realfile = realfile.Replace ('/', Path.DirectorySeparatorChar);
		realfile = Path.GetFullPath (realfile);
		if (File.Exists (realfile)) {
			File.Copy ("Kind.cs", realfile, true);
			File.Delete ("Kind.cs");

			string svn;
			svn =  Path.Combine (Path.GetDirectoryName (realfile), ".svn/text-base/Kind.cs.svn-base".Replace ('/', Path.DirectorySeparatorChar));
			if (string.CompareOrdinal (File.ReadAllText (realfile), File.ReadAllText (svn)) != 0) {
				Console.WriteLine ("The file '{0}' has been updated, don't forget to commit the changes.", realfile);
			}			
		} else {
			Console.WriteLine ("You need to update the file 'Kind.cs' in the 'olive/class/agmono/Mono/' directory with the Kind.cs file generated here");
		}
	}
	
	static void GenerateValueH (ArrayList classes, Hashtable hash)
	{
		const string file = "value.h";
		StringBuilder text;
		string contents = File.ReadAllText (file + ".in");

		text = new StringBuilder ();
		foreach (string c in classes) {
			text.AppendLine ("class " + c + ";");
		}
		contents = contents.Replace ("/*DO_FWD_DECLS*/", text.ToString ());
		
		text = new StringBuilder ();
		foreach (string c in classes) {
			text.AppendLine ("\t\t" + getU (c) + ",");
		}
		contents = contents.Replace ("/*DO_KINDS*/", text.ToString ());
		

		text = new StringBuilder ();
		foreach (string c in classes) {
			text.AppendLine (string.Format ("	{1,-30} As{0} () {{ checked_get_subclass (Type::{2}, {0}) }}", c, c + "*", getU (c)));
		}
		contents = contents.Replace ("/*DO_AS*/", text.ToString ());
		
		text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from value.h.in, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine ("*/");
		contents = text.ToString () + contents;

		File.WriteAllText (file, contents);
	}

	static void GenerateValueCpp (ArrayList classes, Hashtable hash)
	{
		return; // Nothing to do here anymore.
		
//		const string file = "value.cpp";
//		StringBuilder text;
//		string contents = File.ReadAllText (file + ".in");
//
//		text = new StringBuilder ();
//		
//		text.AppendLine ("/*");
//		text.AppendLine (" * Automatically generated from value.cpp.in, do not edit this file directly");
//		text.AppendLine (" * To regenerate execute typegen.sh");
//		text.AppendLine ("*/");
//		
//		text.AppendLine (contents);
//		
//		File.WriteAllText (file, text.ToString ());
	}

	static void GenerateTypeCpp (ArrayList classes, Hashtable hash, Hashtable cctors, Hashtable contprops)
	{
		StringBuilder text = new StringBuilder ();
		text.AppendLine ("/*");
		text.AppendLine (" * Automatically generated from type.cpp.in, do not edit this file directly");
		text.AppendLine (" * To regenerate execute typegen.sh");
		text.AppendLine ("*/");
		text.AppendLine ("");
		text.AppendLine (File.ReadAllText ("type.cpp.in"));
		text.AppendLine ("void\ntypes_init (void)");
		text.AppendLine ("{");
		text.AppendLine ("\tif (types_initialized)");
		text.AppendLine ("\t\treturn;");
		text.AppendLine ("\ttypes_initialized = true;\n");
		text.AppendLine ("");
		foreach (string c in classes) {
			string p = null;
			if (hash.ContainsKey (c))
				p = (string) hash [c];

			if (p != null && p != string.Empty) {
                                string cctor = cctors [c] as string;
				string ctprp = contprops [c] as string;
				text.AppendLine (String.Format ("\tType::RegisterType (\"{0}\", Type::{1}, Type::{2}, {3}, {4});", c, getU(c), getU (p),
                                                cctor != null ? String.Concat ("(create_inst_func *) ", cctor) : "NULL",
						ctprp != null ? String.Concat ("\"", ctprp, "\"") : "NULL"));
			}
		}
		text.AppendLine ("\ttypes_init_manually ();");
		text.AppendLine ("\ttypes_init_register_events ();");
		text.AppendLine ("}");
		File.WriteAllText ("type.cpp", text.ToString ());
	}

	static string RemoveComments (string v)
	{
		int a, b;
		a = v.IndexOf ("/*");
		while (a >= 0) {
			b = v.IndexOf ("*/", a + 2);
			v = v.Remove (a, b - a + 2);
			a = v.IndexOf ("/*", a + 2);
		}
		return v;
	}

	static string getU (string v)
	{
		v = v.ToUpper ();
		v = v.Replace ("DEPENDENCYOBJECT", "DEPENDENCY_OBJECT");
		if (v.Length > "COLLECTION".Length)
			v = v.Replace ("COLLECTION", "_COLLECTION");
		if (v.Length > "DICTIONARY".Length)
			v = v.Replace ("DICTIONARY", "_DICTIONARY");
		return v;
	}

	static void MapClassNameToCCtor (string line, Hashtable cctors)
	{
		string cn;
		StringBuilder cctor = new StringBuilder ();
		int i = line.IndexOf ("*");
	
		if (Char.IsWhiteSpace (line [--i]))
                        --i;

		cn = line.Substring (0, i + 1);

		while (line [++i] == '*' || Char.IsWhiteSpace (line [i])) { }

		while (!Char.IsWhiteSpace (line [i]) && line [i] != '(')
                        cctor.Append (line [i++]);

		cctors [cn] = cctor.ToString ();
	}

}
