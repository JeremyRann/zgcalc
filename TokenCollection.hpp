#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Token.hpp"
#include "ExpressionRef.hpp"

using std::string;
using std::vector;
using std::shared_ptr;

class TokenCollection
{
private:
	string expression;
	vector<shared_ptr<Token>> tokens;
	vector<shared_ptr<ExpressionRef>> substitutables;
	bool success;
	string errorMessage;
	void SetError(const char *msg, int pos);
public:
	TokenCollection();
	string ErrorMessage();
	string Info();
	void ParseExpression(string expression);
	bool Success();
};

