
#include "std_lib_facilities.h"

const char number = '8';
const char quit = 'q';
const char print = ';';
const string prompt = "> ";
const string result = "= "; //used to indicate what follows is a result
const char name = 'a'; //name token
const char let = 'L'; //declaration token
const string declkey = "let"; // declaration keyword

class Token {
public:
	char kind;
	double value;
	string name;
	//Constructor overloads
	Token() : kind(0) {}
	Token(char ch) : kind{ch} {}
	Token(char ch, double val) : kind(ch), value{val} {} //initializing values in constructor
	Token(char ch, string n) : kind{ch}, name{n} {}


};

class Token_stream {
public:
	Token get();
	void putback(Token t);
	void ignore(char c);
	//ui
private:
	bool full {false};
	Token buffer;
	//implmentation
};


class Variable {
public:
	string name;
	double value;

};

vector<Variable> var_table;
Token_stream ts;

double get_value(string s)
{
	for (const Variable&v : var_table)
		if (v.name == s) return v.value;
	error("get: undefined variable ",s);
}

void set_value(string s, double d)
{
	for(Variable& v : var_table)
	{
		if(v.name == s) {
			v.value = d;
			return;
		}
	}
	error("set: undefined variable ", s);
}

bool is_declared(string var)
{ //is var already in var_table
	for (const Variable& v : var_table)
		if(v.name == var) return true;
	return false;
}

double define_name(string var, double val) //add {var, val} to var_table
{
	if (is_declared(var)) error(var, " declared twice");
	var_table.push_back(Variable{var, val});
	return val;
}


void Token_stream::ignore(char c)
{
	if (full && c==buffer.kind) { //first look in buffer
		full = false;
		return;
	}
	full = false;

	//now search input
	char ch = 0;
	while (cin>>ch)
		if(ch==c) return;
}

void Token_stream::putback(Token t)
{
	if (full) error("putback() into a full buffer");
	buffer = t;
	full = true;
}



Token Token_stream::get()
{ //read characters from cin and compose a Token
	if (full) { //do we have a token ready?
		full = false; //remove Token from buffer
		return buffer;
	}
	char ch;
	cin >> ch; // note >> skips white space

	switch(ch){
	case ';': //for print
	case 'q': //for quit
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case '=':
		return Token{ch}; // let each character represent itself
	case '.':
	case '0': case '1': case '2': case '3': case'4':
	case '5': case '6': case'7': case'8': case'9':
	{
		cin.putback(ch); //put digit back into the input stream
		double val;
		cin >> val;
		return Token{number, val};
	}
	default:
        if (isalpha(ch)) {	// start with a letter
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;	// letters digits and underscores
            cin.putback(ch);
            if (s == declkey) return Token(let); // keyword "let"
            //if (s == constkey) return Token(con); // keyword "const"
            return Token(name,s);
        }
		error("Bad token");

	}
}



double expression();


double primary()
{

	Token t = ts.get();
	switch(t.kind){
	case '(': { // handle '(' expression
		double d = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		return d;
	}
	case number: // we use '8' to represent a number
		return t.value; //return the number's value
	case '-':
		return - primary();
	case '+':
		return primary();
    	case name: // the reason there's a whole case for name is because without it you can't actually use the variables you define.
	{ //if you defined x and y and then tried to multiply them for example (x*y) it would just spit out an error
		Token next = ts.get();
		if (next.kind == '=') {	// handle name = expression
			double d = expression();
			set_value(t.name,d);
			return d;
		}
		else {
			ts.putback(next);		// not an assignment: return the value
			return get_value(t.name); // return the variable's value
		}
	}
	default:
		error("primary expected");

	}
}



double term()
{

	double left = primary();
	Token t = ts.get();
	while (true) {
		switch(t.kind) {
		case '*':
			left *= primary();
			t = ts.get();
			break;
		case '/':
		{
			double d = primary();
			if(d == 0) error ("divide by zero");
			left /= d;
			t = ts.get();
			break;
		}
		case '%':
		{
			double d = primary();
			if (d==0) error("%: divide by zero");
			left = fmod(left,d);
			t = ts.get();
			break;
		}
		default:
			ts.putback(t);
			return left;
		}
	}
}


double expression()
{

	double left = term(); // read and evaluate a Term
	Token t = ts.get(); //get the next token
	while (true) {
		switch(t.kind)
		{
		case '+':
			left += term(); // evulate term and add
			t = ts.get();
			break;
		case '-':
			left -= term(); // evaluate term and subtract
			t = ts.get();
			break;
		default:
			ts.putback(t);
			return left; //finally: no more + or -; return answer
		}
	}
}
//possibly comment
void clean_up_mess()
{
	ts.ignore(print);
}

double declaration()
{
	Token t = ts.get();
	if(t.kind != name) error ("name expected in declaration");
	string var_name = t.name;

	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", var_name);

	double d = expression();
	define_name(var_name, d);
	return d;
}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration();
	default:
		ts.putback(t);
		return expression();
	}
}


void calculate()
{
	while(cin)
	try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t= ts.get(); //eat ';'
		if (t.kind == quit) return;
		ts.putback(t);
		cout << result << statement() << '\n';
	}
	catch(exception& e) {
		cerr << e.what() << '\n';
		clean_up_mess();
	}
}


int main() {
	try {
		define_name("pi", 3.1415926535);
		define_name("e", 2.7182818284);

		calculate();
		keep_window_open();
		return 0;
	}
	catch (runtime_error& e)
	{
		cerr << e.what() << '\n';
		keep_window_open("~~");
		return 1;
	}
	catch(exception& e)
	{
		cerr << e.what() << '\n';
		keep_window_open("~~");
		return 1;
	}
	catch(...) {
		cerr << "exception \n";
		keep_window_open("~~");
		return 2;
	}
}
