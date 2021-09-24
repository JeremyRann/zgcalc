#include "TokenCollection.hpp"
#include <memory>
#include <sstream>
#include <stack>
#include <string>

using std::string;
using std::stringstream;
using std::stack;
using std::shared_ptr;
using std::make_shared;

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
	builder << "Substitutables:\n";
	for (unsigned int i = 0; i < substitutables.size(); i++)
	{
		builder
			<< i
			<< ": ("
			<< substitutables[i]->Start
			<< ", "
			<< substitutables[i]->End
			<< ") "
			<< expression.substr(substitutables[i]->Start, substitutables[i]->End - substitutables[i]->Start + 1)
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
		Unknown,
		// All Unary operators are potentially binary operators
		UnaryOperator,
		BinaryOperator,
		Number,
		Letter,
		Period,
		OpenGrouping,
		CloseGrouping,
		WhiteSpace
	};
	
	ParserState state = ParserState::Blank;
	CharacterType type = CharacterType::Unknown;

	shared_ptr<ExpressionRef> currentExpressionRef = NULL;
	shared_ptr<Token> currentToken = NULL;
	bool expEncountered;
	bool dotEncountered;
	bool prevCharExp = false;
	bool prevCharExpSign = false;
	stack<unsigned int> openGroupChars;

	for (unsigned int i = 0; i < expression.length(); i++)
	{
		char current = expression[i];
		type = CharacterType::Unknown;

		if (current >= 0x30 && current <= 0x39)
		{
			type = CharacterType::Number;
		}
		else if (current >= 0x41 && current <= 0x5a || current >= 0x61 && current <= 0x7a)
		{
			type = CharacterType::Letter;
		}
		else if (current == '.')
		{
			type = CharacterType::Period;
		}
		else if (current == '-' || current == '+')
		{
			type = CharacterType::UnaryOperator;
		}
		else if (current == '*' || current == '/' || current == '^')
		{
			type = CharacterType::BinaryOperator;
		}
		else if (current == '(' || current == '[' || current == '{')
		{
			type = CharacterType::OpenGrouping;
		}
		else if (current == ')' || current == ']' ||  current == '}')
		{
			type = CharacterType::CloseGrouping;
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

		if ((prevCharExp && type != CharacterType::Number && type != CharacterType::UnaryOperator) ||
			(prevCharExpSign && type != CharacterType::Number))
		{
			SetError("Invalid character encountered while parsing exponent", i);
			return;
		}

		switch (type)
		{
			case CharacterType::Number:
			case CharacterType::Letter:
			case CharacterType::Period:
				switch (state)
				{
					case ParserState::Blank:
					case ParserState::BlankWithLHS:
						switch (type)
						{
							case CharacterType::Number:
							case CharacterType::Period:
								state = ParserState::Literal;
								currentToken = make_shared<Token>(Token::TokenType::Literal, substitutables.size());
								dotEncountered = type == CharacterType::Period;
								expEncountered = false;
								break;
							default:
								state = ParserState::Substitutable;
								currentToken = make_shared<Token>(Token::TokenType::Substitutable, substitutables.size());
								break;
						}
						currentExpressionRef = make_shared<ExpressionRef>(i);
						substitutables.push_back(currentExpressionRef);
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
						if (type == CharacterType::Period)
						{
							if (state == ParserState::Substitutable)
							{
								SetError("Unexpected period encountered while parsing variable or function name", i);
								return;
							}
							else if (expEncountered)
							{
								SetError("Invalid period encountered in numeric value exponent", i);
								return;
							}
							else if (dotEncountered)
							{
								SetError("Period encountered twice while parsing numeric value", i);
								return;
							}
							dotEncountered = true;
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
				if (prevCharExp && type == CharacterType::UnaryOperator)
				{
					prevCharExpSign = true;
					currentExpressionRef->End = i;
				}
				else
				{
					if (state == ParserState::Blank && type == CharacterType::BinaryOperator)
					{
						SetError("Unexpected binary operator found; expected left operand", i);
						return;
					}
					currentToken = make_shared<Token>(Token::TokenType::Operator, (int)current);
					tokens.push_back(currentToken);
					state = ParserState::Blank;
				}
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
			case CharacterType::OpenGrouping:
				currentToken = make_shared<Token>(Token::TokenType::OpenGrouping, (int)current);
				tokens.push_back(currentToken);
				openGroupChars.push(i);
				state = ParserState::Blank;
				break;
			case CharacterType::CloseGrouping:
				currentToken = make_shared<Token>(Token::TokenType::CloseGrouping, (int)current);
				tokens.push_back(currentToken);
				if (openGroupChars.empty())
				{
					SetError("Unmatched close grouping character", i);
					return;
				}
				else
				{
					bool mismatch = true;
					switch (expression[openGroupChars.top()])
					{
						case '(':
							mismatch = current != ')';
							break;
						case '[':
							mismatch = current != ']';
							break;
						case '{':
							mismatch = current != '}';
							break;
					}
					if (mismatch)
					{
						SetError("Mismatched close grouping character", i);
						return;
					}
				}
				openGroupChars.pop();
				state = ParserState::BlankWithLHS;
				break;
		}
		prevCharExpSign = prevCharExp && type == CharacterType::UnaryOperator;
		prevCharExp = state == ParserState::Literal && (current == 'e' || current == 'E');
	}

	if (!openGroupChars.empty())
	{
		SetError("Ummatched open grouping indicator", openGroupChars.top());
		return;
	}

	if (prevCharExp || (prevCharExpSign))
	{
		SetError("Unexpected end of expression while parsing exponent", expression.length() - 1);
		return;
	}

	if (!tokens.empty() && tokens[tokens.size() - 1]->Type == Token::TokenType::Operator)
	{
		SetError("Unexpected end of expression while waiting for right-hand side of operation", expression.length() - 1);
		return;
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

// Private methods
void TokenCollection::SetError(const char *msg, int pos)
{
	success = false;
	stringstream builder;
	builder << msg << " (" << pos + 1 << ")";
	errorMessage = builder.str();
}

