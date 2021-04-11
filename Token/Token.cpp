#include "Token.hpp"
#include <string>

using std::string;

Token::Token(const Token::TokenType type, const int value)
{
	Type = type;
	Value = value;
}

string Token::TypeName(const TokenType type)
{
	switch (type)
	{
		case TokenType::Literal:
			return "Literal";
		case TokenType::Substitutable:
			return "Substitutable";
		case TokenType::Operator:
			return "Operator";
		case TokenType::Grouping:
			return "Grouping";
		case TokenType::Comma:
			return "Comma";
		default:
			return "(unknown)";
	}
}
