// calculator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

/*
	Simple calculator

	Revision history:
	
	Revised by jckcoenf 2020/8/7
	Revised by jckcoenf 2020/8/4
	Originally written by jckcoenf 2020/8/2

	This program implements a basic expression calculator.
	Input from cin, output to cout
	
	The Grammar for input is:
	
	Calculation:
		Statment
		Print
		Quit
		Calculation Statment

	Print:
		; or \n


	Quit:
		q or quit


	Statment:
		Declaration
		Expression
		Statment Statment
		
	Declaration:
		"let" name " = " Expression
		Assignment
	Assignment
		name = Expression

	
	Expression:
		Term
		Expression + Term
		Expression - Term
		

	Term:
		Primary
		Term * Primary
		Term / Primary
		Term % Primary


	Primary: 
		Number
		( Expression )
		- Primary
		+ Primary
		Variable
		pow
		sqrt

	Variable
		name
		value

	Number:
		floating-point-literal

	
	Input comes from cin through the Token_stream called ts
	Variable comes from cin through the Symbol_stream called bs
*/

#include "std_lib_facilities.h"
//Token 
struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { }
	Token(char ch, string n) : kind(ch), name(n) { }
};
//Token stream
class Token_stream {
public:
	bool full;  //to indicate the buffer's state
	Token buffer; 
	
	Token_stream() :full(false), buffer(0) { }

	Token get();  //to get a Token
	void unget(Token t) { buffer = t; full = true; } //put a Token aside

	void ignore(char);  //ignore the character until the specified character is been read
};



const char print = ';';
const char number_type = '8';
const char name_type = 'a';

const string declkey = "let";
const char let = 'L';
const string quitkey = "quit";
const char quit = 'q';
const string cstkey = "const";
const char cst = 'c';
const string sqrtkey = "sqrt";
const char sqrt_func = 's';
const string powkey = "pow";
const char pow_func = 'p';

Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	
	cin.get(ch); //not skip whitespace

	while (isspace(ch))
	{
		if (ch == '\n')
			return Token(print);
		cin.get(ch);
	}
	
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':   //print
	case '=':
	case 'q':  //for quit
	case ',':  //for pow(x, i) 
	case 'h':
		return Token(ch);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		cin.putback(ch);
		double val;
		cin >> val;
		return Token(number_type, val);  //number_type: '8'
	}
	default:
		if (isalpha(ch)) {
			string s;
			s += ch;
			while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) s += ch;  //the name of variable only consist of characters and numbers, underscores, and start with a character 
			cin.putback(ch);
			if (s == declkey) return Token(let);  //declkey == "let"
			else if (s == quitkey) return Token(quit);  //quitkey == "quit"
			else if (s == sqrtkey) return Token(sqrt_func);
			else if (s == powkey) return Token(pow_func);
			else if (s == cstkey) return Token(cst);
			return Token(name_type, s);  //name_type:'a' is a variable's kind, just like the number_type:'8'
		}

		error("Bad token");
	}
}

void Token_stream::ignore(char c)
{
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	while (cin >> ch)
		if (ch == c) return;
}
//struct: Variable is introduced for storing variable's name and its value
struct Variable {
	string name;
	double value;
	bool is_constant;
	Variable(string n, double v, bool state = 0) :name(n), value(v), is_constant(state) { }
};

//store a sequence of Variable
vector<Variable> var_table;


//operate on the vector<Variable>
class Symbol_table {
public:
	vector<Variable> var_table;

	double get(string s);  //get variable's(by given string) value
	void set(string s, double d);  //set variable's value
	bool is_declared(string s);   //check if the given string(variable's name) have been declared
	double declare(string s, double val, bool state = false);  //add variable to var_table
};

double Symbol_table::get(string s)
{
	for (const Variable& v : var_table)
		if (v.name == s) return v.value;
	error("get: undefined name ", s);
}

void Symbol_table::set(string s, double d)
{
	for (Variable& v : var_table)
	{
		if (v.name == s && !(v.is_constant)) //not a constant
		{
			v.value = d;
			return;
		}
		else if (v.name == s && v.is_constant) //is a constant
			error(s, " is a constant value");
	}
	error("set: undefined name ", s);
}

bool Symbol_table::is_declared(string s)
{
	for (const Variable& v : var_table)
		if (v.name == s) return true;
	return false;
}

double Symbol_table::declare(string s, double val, bool state)
{
	if (is_declared(s)) error(s, " is already declared");

	var_table.push_back(Variable(s, val, state));
	return val;
}

Token_stream ts;
Symbol_table sl;

double expression();  //primary() will invoke it before expression() has a specfic define
double assignment(string);
double primary()
//deal with '('expression')' and '-'(negative notation) and value(constants' and variables')
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{	
		double d = expression();
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		return d;
	}
	case '-':
		return -primary();
	case number_type:  
	{
		return t.value;
	//	int i = narrow_cast<int>(t.value);
		//return i;
	}
	case name_type:
	{
		string variable_name = t.name;
		t = ts.get();
		if (t.kind == '=')
		{
			return assignment(variable_name);
		}
		ts.unget(t);
		return sl.get(variable_name);
	}
	case sqrt_func:   //sqrt(x), x must be positive
	{
		double d = primary();
		if (d < 0) error("the parameter cannot be negative");
		return sqrtf(d);
	}
	case pow_func:  //pow(x, i), i must be integer
	{
		t = ts.get();
		if (t.kind != '(') error("'(' expected");
		
		double d = expression();
		t = ts.get();
		if (t.kind != ',') error("',' expected");
		
		double i = expression();
		
		t = ts.get();
		if (t.kind != ')') error("')' expected");
		i = narrow_cast<int>(i);
		
		return pow(d, i);	
	}
	default:
		error("primary expected");
	}
}

double term()
//deal with '*' and '/' and '%'
{
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{	
			double d = primary();
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		case '%':
		{
			double d = primary();
			if (d == 0) error("divede by zero");
			left = fmod(left, d);
			break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

double expression()
//deal with '+' and '-'
{
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;
		default:
			ts.unget(t);
			return left;
		}
	}
}

double declaration()
//declaration format: let variable_name = expression
//have already deal with "let"(by the function: statment()) before invoke this function
//now deal with "variable_name = expression"
{
	Token t = ts.get();
	if (t.kind != name_type) error("name expected in declaration");  
	string name = t.name;
	if (sl.is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression();

	sl.declare(name, d);
	return d;
}
double declaration_const()
//declaration format: const variable_name = expression
{
	Token t = ts.get();
	if (t.kind != name_type) error("name expected in const declaration");
	string name = t.name;
	if (sl.is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in const declaration of ", name);
	double d = expression();

	sl.declare(name, d, true);
	return d;
}
double assignment(string name)
//assignment format : exist_variable_name = expression
{
	double value = expression();
	sl.set(name, value);
	return value;
}

double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration();
	case cst: //cst -- const
		return declaration_const();
	default:
		ts.unget(t);
		return expression();
	}
}

void clean_up_mess()
{
	ts.ignore(print);
}

const string prompt = "> ";
const string result = "= ";
const char help = 'h';

//print out instructions
void instructions()
{

		cout << "this is an simple calculator with some simple function.\n";
		cout << "It can calculate some simple expression. for example:\n";
		cout << "you can typing '2 + 5 * 8;' and then the calculator will do calculate for you.\n\n";
		cout << "Also, you can declare an variable by typing:\n";
		cout << "let variable = expression;such as:\n'let x = 3.3;'(DO NOT forget the semicolon)\n";
		cout << "if you want define constants, just need typing 'const' before variable\n such as 'const y = 3.3;'\n\n";
		cout << "you can also use some function(we now just support pow() and sqrt() function)\n";
		cout << "Typing 'pow(2, 3);' can calculate x to the power of y.\n or typing sqrt can calculate the root of x\n\n";

		cout << "Now HAVE A TRY. typing when the prompt notation '>' appear.\n\n";
}
void calculate()
{
	while (true) try {
		cout << "typing 'h' or 'H' for help.\n";
		cout << prompt;
		Token t = ts.get();
		while (t.kind == print) t = ts.get();
		if (t.kind == quit) return;
		if (tolower(t.kind) == help) instructions();
		
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}


int main()
try {
	
	sl.declare("k", 1000, true);
	calculate();
	return 0;
}
catch (exception& e) {
	cerr << "exception: " << e.what() << endl;
	char c;
	while (cin >> c && c != ';');
	return 1;
}
catch (...) {
	cerr << "exception\n";
	char c;
	while (cin >> c && c != ';');
	return 2;
}