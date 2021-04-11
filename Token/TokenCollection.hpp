#pragma once
#include <string>
#include <vector>
#include "Token.hpp"
#include "ExpressionRef.hpp"

using std::string;
using std::vector;

class TokenCollection
{
private:
	string expression;
	vector<Token*> tokens;
	// TODO: Maybe literals really is substitutables (shared between variables, functions, and literals)
	vector<ExpressionRef*> literals;
	bool success;
	string errorMessage;
	void SetError(const char *msg, int pos);
public:
	TokenCollection();
	~TokenCollection();
	string ErrorMessage();
	string Info();
	void ParseExpression(string expression);
	bool Success();
};

