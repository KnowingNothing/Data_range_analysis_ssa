/*
 * author: zhengsz@pku.edu.cn
 * date:   2018-06-04
 * discription: parse ssa file
 * 
 */
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <cmath>
#include <stdlib.h>
using namespace std;


enum ERROR_TYPE {NOTSSA, NONE, NAME_ERROR, SYNTEXT_ERROR};

class myError
{
	ERROR_TYPE code;
	string extra;
	int line;
public:
	myError() :code(NONE), extra(""), line(1) {}
	myError(ERROR_TYPE _code, string _extra, int _line) : code(_code), extra(_extra), line(_line) {}
	void give_msg()
	{
		switch (code)
		{
		case NAME_ERROR: cout << "at line " << line << " "; cout << "[ERROR] Too long name! Most 100 characters.\n" << extra << endl;
			break;
		case NOTSSA: cout << "at line " << line << " "; cout << "[ERROR] This is not a ssa file!\n" << extra << endl;
			break;
		case SYNTEXT_ERROR: cout << "at line " << line << " "; cout << "[ERROR] syntax error!\n" << extra << endl;
			break;
		case NONE: cout << "[NONE] no error currently!\n" << extra << endl;
			break;
		default:
			break;
		}
	}
	void set(ERROR_TYPE _code, string _extra, int _line)
	{
		code = _code;
		extra = _extra;
		line = _line;
	}
};

myError global_error;

enum DATA_TYPE {INT, FLOAT};
string data_type[] = { "int", "float" };

enum OP_TYPE {ADD, SUB, MUL, DIV, FLOAT_CAST, INT_CAST, CALL, GOTO_EQ, 
	GOTO_NE, GOTO_LE, GOTO_GE, GOTO_L, GOTO_G, GOTO, ASSIGN, RETURN};

class Statement
{
public:
	OP_TYPE op;
	string result = "0";
};

class Exp_stat : public Statement
{
public:
	string arg1;
	string arg2;
};

class Call_stat : public Statement
{
public:
	string func_name;
	vector<string> args;
};

class VarTable
{
public:
	string name;
	DATA_TYPE type;
	double low, up;
	char bound[2];
	VarTable(string _name, DATA_TYPE _type, double _low= -INFINITY, double _up = INFINITY, char low_bound = '[', char up_bound = ']')
	{
		name = _name;
		type = _type;
		low = _low;
		up = _up;
		bound[0] = low_bound;
		bound[1] = up_bound;
	}
};

class BlockTable
{
public:
	string block_name;
	map<string, VarTable*> IN;
	map<string, VarTable*> OUT;
	vector<Statement*> statements;
	vector<string> next;
	vector<string> pre;
	BlockTable(string _name)
	{
		block_name = _name;
	}
	void draw(fstream &drawer, int indent = 4);
};

void BlockTable::draw(fstream &drawer, int indent)
{
	string white;
	for (int i = 0; i < indent; ++i)
		white.append(" ");
	drawer << white << "<" << block_name << ">" << endl;
	drawer << white << "PRE:(";
	for (auto i = pre.begin(); i != pre.end(); ++i)
		drawer << *i << ",";
	drawer << ")" << endl;
	drawer << white << "IN:( ";
	for (auto i = IN.begin(); i != IN.end(); ++i)
	{
		drawer << i->second->name << "(" << data_type[i->second->type] << ")" << ":" << i->second->bound[0] << i->second->low << "," << i->second->up << i->second->bound[1] << "  ";
	}
	drawer << ")" << endl;
	drawer << white << "statements:" << endl;
	int counter = 1;
	for (auto i = 0; i < statements.size(); ++i)
	{
		if (statements[i]->op == ADD)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "ADD " << p->result << "," << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == SUB)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "SUB " << p->result << "," << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == MUL)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "MUL " << p->result << "," << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == DIV)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "DIV " << p->result << "," << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == CALL)
		{
			auto p = (Call_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "CALL " << p->result << "," << p->func_name << "(";
			for (auto j = p->args.begin(); j != p->args.end(); ++j)
				drawer << *j << ",";
			drawer << ")" << endl;
		}
		else if (statements[i]->op == ASSIGN)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "ASSIGN " << p->result << "," << p->arg1 << endl;
		}
		else if (statements[i]->op == INT_CAST)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "CAST_TO_INT " << p->result << "," << p->arg1 << endl;
		}
		else if (statements[i]->op == FLOAT_CAST)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "CAST_TO_FLOAT " << p->result << "," << p->arg1 << endl;
		}
		else if (statements[i]->op == GOTO)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO " << p->result << endl;
		}
		else if (statements[i]->op == GOTO_EQ)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_EQ " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == GOTO_NE)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_NE " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == GOTO_G)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_G " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == GOTO_GE)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_GE " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == GOTO_L)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_L " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == GOTO_LE)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "GOTO_LE " << p->arg1 << "," << p->arg2 << endl;
		}
		else if (statements[i]->op == RETURN)
		{
			auto p = (Exp_stat*)statements[i];
			drawer << white << "    " << "(" << counter++ << ")" << "RETURN " << p->result << endl;
		}
	}
	drawer << white << "OUT:( ";
	for (auto i = OUT.begin(); i != OUT.end(); ++i)
	{
		drawer << i->second->name << "(" << data_type[i->second->type] << ")" << ":" << i->second->bound[0] << i->second->low << "," << i->second->up << i->second->bound[1] << "  ";
	}
	drawer << ")" << endl;
	drawer << white << "NEXT:(";
	for (auto i = next.begin(); i != next.end(); ++i)
		drawer << *i << ",";
	drawer << ")" << endl;
	drawer << endl;
}

class FuncTable
{
public:
	string func_name;
	vector<VarTable*> args;
	map<string, VarTable*> vars;
	map<string, BlockTable*> blocks;
	BlockTable* add_block(string bname);
	void draw(fstream &drawer);
};

BlockTable* FuncTable::add_block(string bname)
{
	BlockTable *new_blk = new BlockTable(bname);
	map<string, VarTable*>::iterator i = this->vars.begin();
	for (; i != this->vars.end(); i++)
	{
		new_blk->IN[i->first] = new VarTable(*(i->second));
		new_blk->OUT[i->first] = new VarTable(*(i->second));
	}
	blocks[bname] = new_blk;
	return new_blk;
}

void FuncTable::draw(fstream &drawer)
{
	drawer << "**********************************" << endl;
	drawer << "function: " << func_name << endl;
	drawer << "parameters: ( ";
	for (int i = 0; i < args.size(); ++i)
		drawer << args[i]->name << "(" << data_type[args[i]->type] << ")" << ":" << args[i]->bound[0] << args[i]->low << "," << args[i]->up << args[i]->bound[1] << "  ";
	drawer << ")" << endl;
	drawer << "variables: ( ";
	for (auto i = vars.begin(); i != vars.end(); ++i)
		drawer << i->first << "(" << data_type[i->second->type] << ")" << ":" << i->second->bound[0] << i->second->low << "," << i->second->up << i->second->bound[1] << "  ";
	drawer << ")" << endl;
	drawer << "----------------" << endl;
	drawer << "blocks:" << endl;
	for (auto i = blocks.begin(); i != blocks.end(); ++i)
	{
		i->second->draw(drawer);
	}
	drawer << endl;
	drawer << endl;
	
}

class GlobalTable
{
public:
	map<string, FuncTable*> funcs;
	void put(string fname)
	{
		FuncTable *tmp = new FuncTable();
		tmp->func_name = fname;
		funcs[fname] = tmp;
	}
	void draw();
};

void GlobalTable::draw()
{
	fstream drawer;
	drawer.open("draw.txt", ios::out);
	for (auto i = funcs.begin(); i != funcs.end(); i++)
		i->second->draw(drawer);
}

class SSAHandler
{
	fstream logger;
	fstream &file;
	int p;
	char ch;
	char* buffer;
	string cur_func_name;
	string cur_block_name;
	enum STATES {END, GLOBAL, FUNCTION_HEAD, PARAMETER, FUNCTION_BODY};
	GlobalTable table;
	stack<STATES> states;
	int line;

	int nextline();
	void strip();
	void passto(char end);
	void cutid(string &a);
	void cutnumber(string &a);
	void cut(string &a);
	void pruneid(string &a);
	void parse_parameter();
	void parse_function_head();
	void parse_statement();
	void parse_goto(string &a);

public:
	SSAHandler(fstream &_file) : file(_file) 
	{
		p = 0;
		buffer = new char[1024];
		buffer[0] = '\0';
		states.push(END);
		cur_func_name = "0";
		cur_block_name = "0";
		line = 0;
		logger.open("log.txt", ios::out);
	}
	~SSAHandler()
	{
		delete[]buffer;
	}
	void parse(bool draw = false);
	
};

void SSAHandler::cutid(string &a)
{
	char name[100];		// FIXME: name may be longer than 100
	int pname = 0;
	int count = 0;
	while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_') || (ch == '.') || (ch == '(')
		|| (ch == ')') || (ch >= '0' && ch <= '9'))
	{
		if (ch == '(')
			count++;
		if (ch == ')')
		{
			if (count > 0)
				count--;
			else
				break;
		}
		name[pname++] = ch;
		ch = buffer[p++];
	}
	name[pname] = '\0';
	a = string(name);
}

void SSAHandler::cutnumber(string &a)
{
	char number[100];
	int pnumber = 0;
	number[pnumber++] = ch;
	ch = buffer[p++];
	while ((ch >= '0' && ch <= '9') || (ch == '.'))
	{
		number[pnumber++] = ch;
		ch = buffer[p++];
	}
	if (ch == 'e' || ch == 'E')
	{
		number[pnumber++] = ch;
		ch = buffer[p++];
		if (ch == '-' || ch == '+')
		{
			number[pnumber++] = ch;
			ch = buffer[p++];
		}
		while (ch >= '0' && ch <= '9')
		{
			number[pnumber++] = ch;
			ch = buffer[p++];
		}
	}
	number[pnumber] = '\0';
	a = string(number);
}

void SSAHandler::cut(string &a)
{
	if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9'))
	{
		/* again a number */
		cutnumber(a);
		return;
	}
	else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_')
	{
		/* this should be a variable */
		cutid(a);
		pruneid(a);
		return;
	}
}

void SSAHandler::pruneid(string &a)
{
	if (table.funcs[cur_func_name]->vars.find(a) != table.funcs[cur_func_name]->vars.end())
		return;
	size_t p_ = string::npos;
	/* TODO: what if the subscript useful? */
	if ((p_ = a.find_last_of('_')) != string::npos && p_ != 0)
	{
		a = a.substr(0, p_);
	}
}

int SSAHandler::nextline()
{
	if (file.eof())
		return -1;
	p = 0;
	file.getline(buffer, 1024);
	line++;
	ch = buffer[p++];
	return 0;
}

void SSAHandler::strip()
{
	if (ch == '\0')
	{
		if (nextline() < 0)
		{
			global_error.set(SYNTEXT_ERROR, string("unexpected end of file"), line);
			global_error.give_msg();
			return;
		}
	}
	while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	{
		ch = buffer[p++];
		if (ch == '\0')
		{
			if (nextline() < 0)
			{
				global_error.set(SYNTEXT_ERROR, string("unexpected end of file"), line);
				global_error.give_msg();
				return;
			}
		}
	}
}

void SSAHandler::passto(char end)
{
	while (ch != end)
	{
		ch = buffer[p++];
		if (ch == '\0')
		{
			if (nextline() < 0)
			{
				string msg("miss ");
				msg.push_back(end);
				global_error.set(SYNTEXT_ERROR, msg, line);
				global_error.give_msg();
				return;
			}
		}
	}
}

void SSAHandler::parse_parameter()
{
	while (ch != ')')
	{
		string sname;
		string stype;
		cut(stype);
		strip();
		cut(sname);
		strip();
		if (ch == ',')
			ch = buffer[p++];
		strip();

		DATA_TYPE dtype = INT;
		if (stype == "float")
			dtype = FLOAT;

		VarTable *tmp = new VarTable(sname, dtype);
		table.funcs[cur_func_name]->args.push_back(tmp);
		table.funcs[cur_func_name]->vars[sname] = new VarTable(*tmp);
	}
	return;
}

void SSAHandler::parse_function_head()
{
	char name[100];		//name may be longer than 100
	int pname = 0;
	while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_') || (ch >= '0' && ch <= '9'))
	{
		name[pname++] = ch;
		ch = buffer[p++];
	}
	name[pname] = '\0';
	string fname = string(name);
	table.put(fname);
	/* record function name in process */
	cur_func_name = fname;
	while (ch == ' ' || ch == '\t')
		ch = buffer[p++];
	if (ch != '(')
	{
		global_error.set(SYNTEXT_ERROR, string("[ERROR] no '(' at function head ") + fname, line);
		global_error.give_msg();
		return;
	}
	/* in state of handle parameters */
	states.push(PARAMETER);
	ch = buffer[p++];
	while (ch == ' ' || ch == '\t')
		ch = buffer[p++];
	parse_parameter();
	while (ch == ' ' || ch == '\t')
		ch = buffer[p++];
	if (ch != ')')
	{
		global_error.set(SYNTEXT_ERROR, string("[ERROR] no ')' at function head ").append(name), line);
		global_error.give_msg();
		return;
	}
	/* out state of handle parameter */
	states.pop();
	ch = buffer[p++];
	return;
}

void SSAHandler::parse_goto(string &a)
{
	if (ch == '<')
	{
		char to_where[100];
		int pt = 0;
		ch = buffer[p++];	// this is '<'
		while (ch != '>')
		{
			to_where[pt++] = ch;
			ch = buffer[p++];
		}
		to_where[pt] = '\0';
		a = to_where;
		ch = buffer[p++];	// this is '>'
		strip();
		if (ch == '(')
		{
			ch = buffer[p++];
			strip();
			if (ch == '<')
			{
				ch = buffer[p++];	// this is '<'
				pt = 0;
				while (ch != '>')
				{
					to_where[pt++] = ch;
					ch = buffer[p++];
				}
				to_where[pt] = '\0';
				a = to_where;
				ch = buffer[p++];	// this is '>'
				strip();
			}
			ch = buffer[p++];	//	this is ')'
		}
		strip();
		ch = buffer[p++];	// this is ';'
		strip();
	}
	return;
}

void SSAHandler::parse_statement()
{
	if (ch == '-' || ch == '+' || (ch >= '0' && ch <= '9'))
	{
		/* this statement is not an assign, ignore it */
		passto(';');
		return;
	}
	/* if statement begins with id */
	string tmp;
	cut(tmp);
	/* declaration of variables */
	if (tmp == "int" || tmp == "float")
	{
		strip();
		string id;
		cutid(id);
		if(tmp == "int")
			table.funcs[cur_func_name]->vars[id] = new VarTable(id, INT);
		else 
			table.funcs[cur_func_name]->vars[id] = new VarTable(id, FLOAT);
		return;
	}
	else if (tmp == "if")
	{
		strip();
		ch = buffer[p++];	//this is '('
		strip();
		string left, right;
		OP_TYPE type, another_type;
		cut(left);
		strip();
		if (ch == '<')
		{
			ch = buffer[p++];
			if (ch == '=')
			{
				type = GOTO_LE;
				another_type = GOTO_G;
				ch = buffer[p++];
			}
			else
			{
				type = GOTO_L;
				another_type = GOTO_GE;
			}
		}
		else if (ch == '=')
		{
			ch = buffer[p++];
			if (ch == '=')
			{
				type = GOTO_EQ;
				another_type = GOTO_NE;
				ch = buffer[p++];
			}
		}
		else if (ch == '!')
		{
			ch = buffer[p++];
			if (ch == '=')
			{
				type = GOTO_NE;
				another_type = GOTO_EQ;
				ch = buffer[p++];
			}
		}
		else if (ch == '>')
		{
			ch = buffer[p++];
			if (ch == '=')
			{
				type = GOTO_GE;
				another_type = GOTO_L;
				ch = buffer[p++];
			}
			else
			{
				type = GOTO_G;
				another_type = GOTO_LE;
			}
		}
		strip();
		cut(right);
		strip();
		ch = buffer[p++];	//this is ')'
		strip();
		string cur_tmp;
		string to_true, to_false;
		cut(cur_tmp);
		strip();
		parse_goto(to_true);
		strip();
		cut(cur_tmp);
		strip();
		cut(cur_tmp);
		strip();
		parse_goto(to_false);
		strip();
		/* add another two blocks */
		string left_name = cur_block_name + string("_true"), right_name = cur_block_name + string("_false");
		BlockTable* left_blk = table.funcs[cur_func_name]->add_block(left_name);
		BlockTable* right_blk = table.funcs[cur_func_name]->add_block(right_name);
		table.funcs[cur_func_name]->blocks[cur_block_name]->next.push_back(left_blk->block_name);
		table.funcs[cur_func_name]->blocks[cur_block_name]->next.push_back(right_blk->block_name);
		Exp_stat *left_stat = new Exp_stat();
		Exp_stat *right_stat = new Exp_stat();
		left_stat->op = type;
		right_stat->op = another_type;
		left_stat->arg1 = right_stat->arg1 = left;
		left_stat->arg2 = right_stat->arg2 = right;
		left_blk->statements.push_back(left_stat);
		right_blk->statements.push_back(right_stat);
		left_blk->next.push_back(to_true);
		right_blk->next.push_back(to_false);
		cur_block_name = "0";		// means no current block
		return;
	}
	else if (tmp == "goto")
	{
		strip();
		string to_where;
		parse_goto(to_where);
		table.funcs[cur_func_name]->blocks[cur_block_name]->next.push_back(to_where);
		cur_block_name = "0";	// means no current block
		return;
	}
	else if (tmp == "return")
	{
		strip();
		string result;
		cut(result);
		strip();
		Exp_stat *cur_stat = new Exp_stat();
		cur_stat->op = RETURN;
		cur_stat->result = result;
		table.funcs[cur_func_name]->blocks[cur_block_name]->statements.push_back(cur_stat);
		return;
	}
	else
	{
		strip();
		if (ch != '=')
		{
			/* useless statement without assign */
			passto(';');
			return;
		}
		/* next we only consider about assign */
		OP_TYPE type = ASSIGN;
		Exp_stat *cur_stat = new Exp_stat();
		cur_stat->result = tmp;
		ch = buffer[p++];
		strip();
		if (ch == '(')
		{
			/* this is cast statement */
			ch = buffer[p++];
			strip();
			string cur_type;
			cut(cur_type);
			strip();
			type = FLOAT_CAST;
			if (cur_type == "int")
				type = INT_CAST;
			ch = buffer[p++];
			strip();
			string arg1;
			cut(arg1);
			strip();
			cur_stat->op = type;
			cur_stat->arg1 = arg1;
			table.funcs[cur_func_name]->blocks[cur_block_name]->statements.push_back(cur_stat);
			return;
		}
		else
		{
			string arg1;
			cut(arg1);
			strip();
			if (ch == ';')
			{
				cur_stat->op = type;
				cur_stat->arg1 = arg1;
				table.funcs[cur_func_name]->blocks[cur_block_name]->statements.push_back(cur_stat);
				return;
			}
			else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
			{
				if (ch == '+')
					type = ADD;
				else if (ch == '-')
					type = SUB;
				else if (ch == '*')
					type = MUL;
				else if (ch == '/')
					type = DIV;
				ch = buffer[p++];
				strip();
				string arg2;
				cut(arg2);
				strip();
				cur_stat->op = type;
				cur_stat->arg1 = arg1;
				cur_stat->arg2 = arg2;
				table.funcs[cur_func_name]->blocks[cur_block_name]->statements.push_back(cur_stat);
				return;
			}
			else if (ch == '(')
			{
				/* call of function */
				Call_stat *cur_stat = new Call_stat();
				cur_stat->result = tmp;
				cur_stat->op = CALL;
				cur_stat->func_name = arg1;
				ch = buffer[p++];
				strip();
				while(ch != ')')
				{ 
					string arg;
					cut(arg);
					strip();
					if (ch == ',')
					{
						ch = buffer[p++];
						strip();
					}
					cur_stat->args.push_back(arg);
				}
				ch = buffer[p++];
				table.funcs[cur_func_name]->blocks[cur_block_name]->statements.push_back(cur_stat);
				return;
			}
		}		
	}
}

void SSAHandler::parse(bool draw)
{
	cout << "start parsing..." << endl;
	ch = buffer[p++];
	states.push(GLOBAL);
	while (!file.eof())
	{
		logger << "at line: " << line << " current char is " << ch << endl;
		if (ch == '\0')
		{
			nextline();
		}
		else if ((ch == ' ') || (ch == '\t'))
		{
			strip();
		}
		else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_') || (ch >= '0' && ch <= '9'))
		{
			if (states.top() == GLOBAL)
			{
				/* this should be head of a function */
				states.push(FUNCTION_HEAD);
				parse_function_head();
				states.pop();
			}
			else if (states.top() == FUNCTION_BODY)
			{
				/* this should be def or use */
				parse_statement();
			}
		}
		else if (ch == '{')
		{
			if (states.top() == GLOBAL)
			{
				/* this should be body of current function */
				states.push(FUNCTION_BODY);
				ch = buffer[p++];
			}
		}
		else if (ch == '}')
		{
			if (states.top() == FUNCTION_BODY)
			{
				/* this is end of function */
				states.pop();
				states.push(GLOBAL);
				ch = buffer[p++];
				for (auto i = table.funcs[cur_func_name]->blocks.begin(); i != table.funcs[cur_func_name]->blocks.end(); ++i)
				{
					auto &v = i->second->next;
					for (auto j = v.begin(); j != v.end(); j++)
					{
						table.funcs[cur_func_name]->blocks[*j]->pre.push_back(i->first);
					}
				}
				cur_func_name = cur_block_name = "0";
			}
		}
		else if (ch == ';')
		{
			if (states.top() == GLOBAL)
			{
				ch = buffer[p++];
				if (ch == ';')
				{
					nextline();
				}
				else
				{
					global_error.set(SYNTEXT_ERROR, string("[ERROR] ';' outside function"), line);
					global_error.give_msg();
				}
			}
			else
			{
				ch = buffer[p++];
			}
		}
		else if (ch == '#')
		{
			nextline();
		}
		else if (ch == '<')
		{
			/* this is block */
			char blockname[100];
			int pname = 0;
			ch = buffer[p++];
			while (ch != '>')
			{
				blockname[pname++] = ch;
				ch = buffer[p++];
			}
			blockname[pname] = '\0';
			ch = buffer[p++];	// this is '>'
			ch = buffer[p++];	// this is ':'
			string bname = blockname;
			if (cur_block_name != "0")
			{
				table.funcs[cur_func_name]->blocks[cur_block_name]->next.push_back(bname);
			}
			cur_block_name = bname;
			table.funcs[cur_func_name]->add_block(bname);
		}
	}
	cout << "parse done!" << endl;
	if (draw)
		table.draw();
}

void parse_ssa(fstream &file, bool draw=false)
{
	SSAHandler h(file);
	h.parse(draw);
}

void handle(string filename, bool draw = false)
{
	int length = filename.length();
	string ext = filename.substr(length - 3);
	if (ext != "ssa")
	{
		string msg = "The file given is ";
		msg += filename;
		global_error.set(NOTSSA, msg, 0);
		global_error.give_msg();
		return;
	}
	fstream file;
	file.open(filename.c_str(), ios::in);
	parse_ssa(file, draw);
	file.close();
}

int main()
{
	handle(string("t10.ssa"), true);
	system("pause");
	return 0;
}