#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "Token\TokenCollection.hpp"

using std::string;
using std::cout;
using std::cin;
using std::getline;
using std::endl;
using boost::to_lower_copy;
using boost::trim;

int main()
{
	cout << "Enter an expression to evaluate, or \"exit\" to exit\n";
	string command = "";
	while (command != "exit")
	{
		if (command != "")
		{
			TokenCollection coll;
			coll.ParseExpression(command);
			if (coll.Success())
			{
				cout << coll.Info() << "\n";
			}
			else
			{
				cout << "Error: " << coll.ErrorMessage() << "\n";
			}
		}

		string rawCommand;
		getline(cin, rawCommand);
		trim(rawCommand);
		command = to_lower_copy(rawCommand);
	}
}

