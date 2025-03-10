/**
* @file ANTLRData.cpp
* @date April 2015
* Copyright (C) 2015-2022 Altair Engineering, Inc.  
* This file is part of the OpenMatrix Language ("OpenMatrix") software.
* Open Source License Information:
* OpenMatrix is free software. You can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* OpenMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.
* You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
* Commercial License Information: 
* For a copy of the commercial license terms and conditions, contact the Altair Legal Department at Legal@altair.com and in the subject line, use the following wording: Request for Commercial License Terms for OpenMatrix.
* Altair's dual-license business model allows companies, individuals, and organizations to create proprietary derivative works of OpenMatrix and distribute them - whether embedded or bundled with other software - under a commercial license agreement.
* Use of Altair's trademarks and logos is subject to Altair's trademark licensing policies.  To request a copy, email Legal@altair.com and in the subject line, enter: Request copy of trademark and logo usage policy.
*/

#include "ANTLRData.h"
#include <iostream>

ANTLRData::ANTLRData(pANTLR3_INPUT_STREAM in_input, bool delay_parser)
{
	input  = in_input; // we own this now
	lex    = ExprCppTreeLexerNew(input);
	tokens = antlr3CommonTokenStreamSourceNew(8192+1, TOKENSOURCE(lex));

	if (!delay_parser)
		parser = ExprCppTreeParserNew(tokens);
	else
		parser = NULL;
}

void ANTLRData::CreateParser(pANTLR3_COMMON_TOKEN_STREAM in_tokens)
{
	tokens = in_tokens;
	parser = ExprCppTreeParserNew(tokens);
}

ANTLRData::~ANTLRData()
{
	if (parser) // we won't always have a parser
		parser->free(parser);
	tokens->free(tokens);
	lex->free(lex);
	input->close(input);
}

pANTLR3_INPUT_STREAM ANTLRData::InputFromFilename(const std::string& filename)
{
	return antlr3FileStreamNew((pANTLR3_UINT8)filename.c_str(), ANTLR3_ENC_UTF8);
}

pANTLR3_INPUT_STREAM ANTLRData::InputFromExpression(const std::string& instring, const std::string& use_filename)
{
	const unsigned char* temp_str = reinterpret_cast<const unsigned char *> (instring.c_str());
	return antlr3StringStreamNew((pANTLR3_UINT8)temp_str, ANTLR3_ENC_UTF8, (ANTLR3_UINT32)instring.size(), (pANTLR3_UINT8)use_filename.c_str());
}

void ANTLRData::DumpTokenStream(bool include_hidden)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int num_tokens = vec->size(vec);

	for (int j=0; j<num_tokens; ++j)
	{
		pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, j);
		pANTLR3_STRING str = tok->getText(tok);

		if ((tok->getChannel(tok) == HIDDEN) && !include_hidden)
			continue;

		if (tok->getType(tok) == NEWLINE)
			std::cout << "[[NEWLINE]]" << std::endl;
		else if (tok->getType(tok) == WS)
			std::cout << "[[" << str->chars << "]]" << std::endl;
		else
			std::cout << str->chars << "    " << tok->getType(tok) << std::endl;
	}
}

//! Utility to get string token
const char* GetStringToken(pANTLR3_VECTOR vec, int index)
{
	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, index);
	pANTLR3_STRING       str = tok->getText(tok);
	return (const char*)str->chars;
}

pANTLR3_COMMON_TOKEN GetPreviousNonWhitespaceToken(pANTLR3_VECTOR vec, int index)
{
	pANTLR3_COMMON_TOKEN prev_tok = NULL;

	for (int j = index - 1; j >= 0; --j)
	{
		prev_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, j);

		if (prev_tok->getType(prev_tok) != WS)
			break;
	}

	return prev_tok;
}

void ANTLRData::PreprocessTokenStream(pANTLR3_COMMON_TOKEN_STREAM& tokens)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	for (int j = 0; j < num_tokens; ++j)
	{
		pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, j);
		int token_type = tok->getType(tok);

		if (token_type == QUOTE)
		{
			if (j > 0)
			{
				pANTLR3_COMMON_TOKEN prev_tok = GetPreviousNonWhitespaceToken(vec, j);
				int                  prev_tok_type = 0;
				
				if (prev_tok)
					prev_tok_type = prev_tok->getType(prev_tok);

				if ((prev_tok_type != RBRACKET) && (prev_tok_type != RPAREN) && (prev_tok_type != RCURLY) && (prev_tok_type != IDENT) && (prev_tok_type != DOT) && (prev_tok_type != NUMBER))
					j = PreprocessString(tokens, j);
			}
			else
			{
				j = PreprocessString(tokens, j);
			}
		}
		else if (token_type == EQUOTE)
		{
			j = PreprocessEquote(tokens, j);
		}
		else if ((token_type == PERCENT) || (token_type == PCTLCURLY))
		{
			j = PreprocessComment(tokens, j);
		}
		else if (token_type == CONT)
		{
			j = PreprocessContinue(tokens, j);
		}
		else if (token_type == LBRACKET)
		{
			j = PreprocessMatrixOrCell(tokens, j);
		}
		else if (token_type == LCURLY)
		{
			j = PreprocessMatrixOrCell(tokens, j);
		}
		else if (token_type == NUMBERHACK)
		{
			j = PreprocessNumberHack(tokens, j);
		}
		else if (token_type == LPAREN)
		{
			j = PreprocessParentheses(tokens, j);
		}
		else if (token_type == IDENT)
		{
			if (j < (num_tokens - 2))
			{
				pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, j + 1);
				pANTLR3_COMMON_TOKEN tok3 = (pANTLR3_COMMON_TOKEN)vec->get(vec, j + 2);

				// What I'd like to do here is replace tok3 with a LPAREN
				// and then insert an RPAREN after tokn.  However, there is no insert
				// method on the ANTLR3_VECTOR, so the best I can do for now is to remove
				// the quotes. This does have a functional impact on a case like:
				// a 'b c' unfortunately.
				if ((tok2->getType(tok2) == WS) && (tok3->getType(tok3) == QUOTE))
				{
					tok3->setChannel(tok3, HIDDEN);

					for (int k = j + 3; k < num_tokens; ++k)
					{
						pANTLR3_COMMON_TOKEN tokn = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);

						if (tokn->getType(tokn) == QUOTE)
						{
							tokn->setChannel(tokn, HIDDEN);
							j = k + 1;
							break;
						}
					}
				}
			}
		}
		else if (token_type == PROPERTIES)
		{
			if (j < (num_tokens - 1))
			{
				pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, j + 1);

				if (tok2->getType(tok2) == LPAREN)
				{
					tok->setType(tok, IDENT);
					// the text for tok should already by properties
				}
			}
		}
		else if (token_type == METHODS)
		{
			bool skip = false;

			if (j < (num_tokens - 1))
			{
				pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, j + 1);

				if (tok2->getType(tok2) == LPAREN)
				{
					for (int k = j + 2; k < num_tokens; ++k)
					{
						pANTLR3_COMMON_TOKEN tokn = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);

						if (tokn->getType(tokn) == RPAREN)
							break;

						if (tokn->getType(tokn) == ASSIGN)
						{
							skip = true;
							break;
						}
					}

					if (!skip)
						tok->setType(tok, IDENT);
					// the text for tok should already by methods
				}
			}
		}
	}
}

int ANTLRData::PreprocessMatrixOrCell(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);

	int k = start;

	int exit_token_type = 0;

	if (token_type == LBRACKET)
		exit_token_type = RBRACKET;
	else if (token_type == LCURLY)
		exit_token_type = RCURLY;

	if (exit_token_type)
	{
		int brace_token = start;
		int start_count = 1;

		for (k = start + 1; k < num_tokens; k++)
		{
			pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);
			int token_type = tok->getType(tok);

			pANTLR3_COMMON_TOKEN prev_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, k - 1);

			// There's a string in the cell or matrix, so skip that whole thing
			// but make sure we don't have a transpose operator
			bool prev_tok_check = true;

			if (prev_tok->getType(prev_tok) == RPAREN)
				prev_tok_check = false;

			if (prev_tok->getType(prev_tok) == RCURLY)
				prev_tok_check = false;

			if (prev_tok->getType(prev_tok) == IDENT)
				prev_tok_check = false;

			if ((token_type == QUOTE) && prev_tok_check)
			{
				int  q;
				bool true_string = true;

				for (q = k + 1; q < num_tokens; q++)
				{
					pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, q);

					if (tok->getType(tok) == NEWLINE)
					{
						true_string = false;
						break;
					}

					if (tok->getType(tok) == QUOTE)
						break;
				}

				if (true_string)
					k = PreprocessString(tokens, k);
			}
			else if (token_type == LPAREN)
			{
				k = PreprocessParentheses(tokens, k);
			}
			else if (token_type == LBRACKET)
			{
				k = PreprocessMatrixOrCell(tokens, k);
			}
			else if (token_type == LCURLY)
			{
				k = PreprocessMatrixOrCell(tokens, k);
			}
			else if (token_type == NUMBERHACK)
			{
				k = PreprocessNumberHack(tokens, k);
			}
			else if (token_type == END)
			{
				tok->setType(tok, IDENT);
				pANTLR3_STRING str = tok->getText(tok);
				str->set(str, "end");
				tok->setText(tok, str);
			}
			else if (token_type == CONT)
			{
				k = PreprocessContinue(tokens, k);
			}
			else if (token_type == EQUOTE)
			{
				k = PreprocessEquote(tokens, k);
			}
			else if (token_type == exit_token_type)
			{
				return k;
			}
			else if ((token_type == WS) && (k != brace_token + 1))
			{
				pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k + 1);

				if (!tok2)
					continue;

				int tok2_type = tok2->getType(tok2);

				pANTLR3_COMMON_TOKEN tok_prev = (pANTLR3_COMMON_TOKEN)vec->get(vec, k - 1);

				if (tok_prev->getType(tok_prev) == CONT)
				{
					tok_prev = (pANTLR3_COMMON_TOKEN)vec->get(vec, k - 2);

					if (tok_prev->getType(tok_prev) == WS)
						tok_prev = (pANTLR3_COMMON_TOKEN)vec->get(vec, k - 3);
				}

				if (tok2_type == CONT)
				{
					k = PreprocessContinue(tokens, k + 1);
					tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);
				}

				int prev_type = tok_prev->getType(tok_prev);
				int next_type = tok2_type;

				bool prev_token_flag = false;
				bool next_token_flag = false;

				if ((prev_type != COMMA) && (prev_type != COLON) && (prev_type != SEMIC) && (prev_type != NEWLINE) && (prev_type != TIMES) && (prev_type != DIV) && (prev_type != ASSIGN) && (prev_type != EQUAL))
					prev_token_flag = true;

				if ((next_type == MINUS) || (next_type == PLUS) || (next_type == QUOTE) || (next_type == EQUOTE) || (next_type == LPAREN) || (next_type == LCURLY) || (next_type == CONT))
					next_token_flag = true;

				if ((next_type == LPAREN) && ((prev_type == PLUS) || (prev_type == MINUS)))
					next_token_flag = false;

				if (((next_type == PLUS) || (next_type == MINUS)) && ((prev_type == PLUS) || (prev_type == MINUS)))
					next_token_flag = false;

				if ((next_type == LPAREN) && (prev_type == RPAREN))
				{
					// need to find the character before the previous (
					// if it's an @, then this is an anonymous function definition
					// and we don't want the comma

					for (int m = k - 3; m > 0; m--)
					{
						pANTLR3_COMMON_TOKEN test_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, m);
						int test_tok_type = test_tok->getType(test_tok);

						if (test_tok_type == LPAREN)
						{
							pANTLR3_COMMON_TOKEN test_tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, m - 1);
							int test_tok2_type = test_tok2->getType(test_tok2);

							if (test_tok2_type == AMP)
								next_token_flag = false;

							break;
						}
					}
				}

				if (prev_token_flag && next_token_flag)
				{
					if (k + 2 < (int)vec->count)
					{
						pANTLR3_COMMON_TOKEN tok3 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k + 2);
						int tok3_type = tok3->getType(tok3);
			
						if (tok3_type != WS) 
						{
							tok->setChannel(tok, 0);
							tok->setType(tok, COMMA);
						}
						else
						{
							if (tok2_type == QUOTE) // special case is for strings that start with a space
							{
								tok->setChannel(tok, 0);
								tok->setType(tok, COMMA);
							}
							else if (tok2_type == LCURLY) // special case is for cells that start with a space
							{
								tok->setChannel(tok, 0);
								tok->setType(tok, COMMA);
							}
						}
					}
				}
			}
		}
	}

	return k;
}

int ANTLRData::PreprocessString(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);

	int k = start;

	if (token_type == QUOTE)
	{
		for (k = start + 1; k < num_tokens; ++k)
		{
			pANTLR3_COMMON_TOKEN tok3 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);

			// not sure if we need these lines to force an evaluation or not
			pANTLR3_STRING str = tok3->getText(tok3);
			char* c_str3 = (char*)str->chars;

			if (tok3->getType(tok3) == QUOTE)
				break;
		}
	}

	return k;
}

int ANTLRData::PreprocessParentheses(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);

	int k = start;

	if (token_type == LPAREN)
	{
		for (k = start + 1; k < num_tokens; ++k)
		{
			pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);
			int tok2_type = tok2->getType(tok2);

			if (tok2_type == END)
			{
				tok2->setType(tok2, IDENT);
				pANTLR3_STRING str = tok2->getText(tok2);
				str->set(str, "end");
				tok2->setText(tok2, str);
			}
			else if (tok2_type == RPAREN)
			{
				break;
			}
			else if (tok2_type == LBRACKET)
			{
				k = PreprocessMatrixOrCell(tokens, k);
			}
			else if (tok2_type == LCURLY)
			{
				k = PreprocessMatrixOrCell(tokens, k);
			}
			else if (tok2_type == NUMBERHACK)
			{
				k = PreprocessNumberHack(tokens, k);
			}
			else if (tok2_type == LPAREN)
			{
				k = PreprocessParentheses(tokens, k);
			}
			else if (tok2_type == QUOTE)
			{
				if (k > 0)
				{
					pANTLR3_COMMON_TOKEN prev_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, k - 1);
					int                  prev_tok_type = prev_tok->getType(prev_tok);

					if ((prev_tok_type != RBRACKET) && (prev_tok_type != RPAREN) && (prev_tok_type != RCURLY) && (prev_tok_type != IDENT) && (prev_tok_type != DOT) && (prev_tok_type != NUMBER))
						k = PreprocessString(tokens, k);
				}
			}
			else if (tok2_type == EQUOTE)
			{
				k = PreprocessEquote(tokens, k);
			}
			else if (tok2_type == CONT)
			{
				k = PreprocessContinue(tokens, k);
			}
		}
	}

	return k;
}

int ANTLRData::PreprocessComment(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);
	int k = start;

	if (token_type == PCTLCURLY)
	{
		for (k = start; k < num_tokens; ++k)
		{
			pANTLR3_COMMON_TOKEN next_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);

			next_tok->setChannel(next_tok, HIDDEN);

			// If the first token is on a hidden channel, ANTLR doesn't know
			// what to do and generates a syntax error.  So don't let that happen.
			if (k == 0)
				next_tok->setType(next_tok, NEWLINE);

			if (next_tok->getType(next_tok) == RCURLYPCT)
				break;
		}
	}
	else if (token_type == PERCENT)
	{
		for (k = start + 1; k < num_tokens; ++k)
		{
			pANTLR3_COMMON_TOKEN tok3 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);

			if (tok3->getType(tok3) == NEWLINE)
				break;
		}
	}

	return k;
}

int ANTLRData::PreprocessContinue(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);
	
	int k = start;

	if (token_type == CONT)
	{
		for (k = start; k < num_tokens; ++k)
		{
			pANTLR3_COMMON_TOKEN next_tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);
			next_tok->setChannel(next_tok, HIDDEN);

			if (next_tok->getType(next_tok) == NEWLINE)
				break;
		}
	}

	return k;
}

int ANTLRData::PreprocessEquote(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);
	int            num_tokens = vec->size(vec);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);

	int token_type = tok->getType(tok);

	int k = start;

	if (token_type == EQUOTE)
	{
		pANTLR3_COMMON_TOKEN tok2 = NULL;
		int tok2_type = 0;

		// allow for stacked EQUOTES
		for (k = start + 1; k < num_tokens; ++k)
		{
			tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, k);
			tok2_type = tok2->getType(tok2);

			if (tok2_type != EQUOTE)
				break;
		}

		if (tok2_type == QUOTE)
		{
			tok->setType(tok, QUOTE);
			pANTLR3_STRING str = tok->getText(tok);
			str->set(str, "'");
			tok->setText(tok, str);

			tok2->setType(tok2, EQUOTE);
			str = tok2->getText(tok2);
			str->set(str, "''");
			tok2->setText(tok2, str);

			k = PreprocessString(tokens, start);
		}
		else
		{
			--k;
		}
	}

	return k;
}

int ANTLRData::PreprocessNumberHack(pANTLR3_COMMON_TOKEN_STREAM& tokens, int start)
{
	pANTLR3_VECTOR vec = tokens->getTokens(tokens);

	pANTLR3_COMMON_TOKEN tok = (pANTLR3_COMMON_TOKEN)vec->get(vec, start);
	pANTLR3_COMMON_TOKEN tok2 = (pANTLR3_COMMON_TOKEN)vec->get(vec, start + 1);

	if (tok && tok2)
	{
		if (tok2->getType(tok2) == TIMES)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, ETIMES);
		}
		else if (tok2->getType(tok2) == DIV)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, EDIV);
		}
		else if (tok2->getType(tok2) == POW)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, DOTPOW);
		}
		else if (tok2->getType(tok2) == LDIV)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, ELDIV);
		}
		else if (tok2->getType(tok2) == QUOTE)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, QUOTE);
		}
		else if (tok2->getType(tok2) == DOTDOT)
		{
			tok->setType(tok, NUMBER);
			tok2->setType(tok2, NEWLINE);
			tok2->setChannel(tok2, 0);
		}
	}
	return start + 2;
}
