#include "TokenCollection.hpp"
#include <unordered_set>
#include <string>
#include <exception>
#include <sstream>

using std::string;
using std::stringstream;

TokenCollection::TokenCollection()
{
	success = false;
}

string TokenCollection::ErrorMessage()
{
	return errorMessage;
}

string TokenCollection::Info()
{
	stringstream builder;
	builder << "Literals:\n";
	for (unsigned int i = 0; i < literals.size(); i++)
	{
		builder
			<< i
			<< ": ("
			<< literals[i]->Start
			<< ", "
			<< literals[i]->End
			<< ") "
			<< expression.substr(literals[i]->Start, literals[i]->End - literals[i]->Start + 1)
			<< "\n";
	}

	builder << "\nTokens:\n";

	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		builder << Token::TypeName(tokens[i]->Type) << ": " << tokens[i]->Value << "\n";
	}
	return builder.str();
}

void TokenCollection::ParseExpression(const string expression)
{
	success = true;
	TokenCollection::expression = expression;

	enum class ParserState
	{
		Blank,
		// Not accumulating means "blank", but track if previously there was a left-hand side
		BlankWithLHS,
		Literal,
		Substitutable
	};

	enum class CharacterType
	{
		// All Unary operators are potentially binary operators
		Unknown,
		UnaryOperator,
		BinaryOperator,
		Number,
		Letter,
		Grouping,
		WhiteSpace
	};
	
	ParserState state = ParserState::Blank;

	ExpressionRef *currentExpressionRef = NULL;
	Token *currentToken = NULL;
	bool expEncountered;
	bool dotEncountered;

	for (unsigned int i = 0; i < expression.length(); i++)
	{
		char current = expression[i];
		CharacterType type = CharacterType::Unknown;

		if (current >= 0x30 && current <= 0x39)
		{
			type = CharacterType::Number;
		}
		else if (current >= 0x41 && current <= 0x5a || current >= 0x61 && current <= 0x7a)
		{
			type = CharacterType::Letter;
		}
		else if (current == '-' || current == '+')
		{
			type = CharacterType::UnaryOperator;
		}
		else if (current == '*' || current == '/')
		{
			type = CharacterType::BinaryOperator;
		}
		else if (current == '(' || current == ')' || current == '[' || current == ']' || current == '{' || current == '}')
		{
			type = CharacterType::Grouping;
		}
		else if (current == ' ' || current == '\t' || current == '\r' || current == '\n')
		{
			type = CharacterType::WhiteSpace;
		}
		else
		{
			SetError("Unrecognized character", i);
			return;
		}

		switch (type)
		{
			case CharacterType::Number:
			case CharacterType::Letter:
				switch (state)
				{
					case ParserState::Blank:
					case ParserState::BlankWithLHS:
						if (type == CharacterType::Number)
						{
							state = ParserState::Literal;
							currentToken = new Token(Token::TokenType::Literal, literals.size());
							dotEncountered = false;
							expEncountered = false;
						}
						else
						{
							state = ParserState::Substitutable;
							currentToken = new Token(Token::TokenType::Substitutable, literals.size());
						}
						currentExpressionRef = new ExpressionRef(i);
						literals.push_back(currentExpressionRef);
						tokens.push_back(currentToken);
						break;
					case ParserState::Literal:
					case ParserState::Substitutable:
						if (currentExpressionRef == NULL)
						{
							SetError("Internal Error: Unable to locate existing expression to update", i);
							return;
						}
						if (type == CharacterType::Letter && state == ParserState::Literal)
						{
							bool err = false;
							if (current == 'e' || current == 'E')
							{
								err = expEncountered;
								expEncountered = true;
							}
							else
							{
								err = true;
							}
							if (err)
							{
								SetError("Unexpected character encountered while parsing numeric value", i);
								return;
							}
						}
						currentExpressionRef->End = i;
						break;
					default:
						SetError("Internal Error: Unrecognized parser state", i);
						return;
				}
				break;
			case CharacterType::UnaryOperator:
			case CharacterType::BinaryOperator:
				if (state == ParserState::Blank && type == CharacterType::BinaryOperator)
				{
					SetError("Unexpected binary operator found; expected left operand", i);
					return;
				}
				currentToken = new Token(Token::TokenType::Operator, (int)current);
				tokens.push_back(currentToken);
				state = ParserState::Blank;
				break;
			case CharacterType::WhiteSpace:
				switch (state)
				{
					case ParserState::Blank:
						state = ParserState::Blank;
						break;
					case ParserState::BlankWithLHS:
					case ParserState::Literal:
					case ParserState::Substitutable:
						state = ParserState::BlankWithLHS;
						break;
				}
				break;
		}
	}

	// Sanity check: If there was a call to SetError, it should ALWAYS return immediately afterwards
	if (!success)
	{
		SetError(("CRITICAL: Failed to property return from error condition. Original error: " + errorMessage).c_str(), 0);
	}
}

bool TokenCollection::Success()
{
	return success;
}

TokenCollection::~TokenCollection()
{
	for (int i = 0; i < literals.size(); i++)
	{
		delete literals[i];
	}

	for (int i = 0; i < tokens.size(); i++)
	{
		delete tokens[i];
	}
}

// Private methods
void TokenCollection::SetError(const char *msg, int pos)
{
	success = false;
	stringstream builder;
	builder << msg << " (" << pos + 1 << ")";
	errorMessage = builder.str();
}
