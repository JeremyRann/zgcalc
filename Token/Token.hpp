#pragma once
#include <string>

using std::string;

class Token
{
public:
	enum class TokenType
	{
		Literal,
		Substitutable,
		Operator,
		Grouping,
		Comma
	};

	Token(TokenType type, int value);

	static string TypeName(TokenType type);

	TokenType Type;
	/* Value is:
	* Literal: Index from Literals collection
	* Substitutable: Index from Substitutables collection
	* Operator: Operator enum value
	* Grouping or Comma: Character
	*/
	int Value;
};

