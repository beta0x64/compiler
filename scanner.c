#include "headers/openfile.h"
#include "headers/scanner.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/* from stack overflow http://stackoverflow.com/a/16151299 */
void toUpper(char *text, char *nText){
    int i;
    for(i=0; i<=strlen(text); i++){
        if( (text[i] > 96 ) && (text[i] < 123) ) // is the char lower case
            nText[i] = text[i] - 'a' + 'A';   //make upper
        else
            nText[i] = text[i]; //do nothing
    }   
}

void scanner(openfile_data * of_d_ptr) {
    openfile_data of_d = *of_d_ptr;
    FILE * in = of_d.input;
    FILE * listing_file = of_d.listing_file;
    scanner_data s;
    scanner_data * s_p = &s;
    // priming the loop
    s.line_buffer[0] = '\0';
    while (fgets(s.line_buffer, MAX_SIZE, in) != NULL) {
        s.line_index = 0;
        match_code_to_token(s_p);
        int source_eof = feof(in);
        //process tokens in t array
        int i;
        for (i = 0; i < s.t_index; i++) {
            //format from token and print to temp file
            token t = s.t[i];
            if (t.token_number != -1) {
                printf("%s %s %d\n", t.token_type, t.buffer, t.token_number);
            }
            if (t.token_number == ERROR) {
                // put this in the listing file via of_d_ptr
                printf("\nERROR: %s\n", t.buffer);
            }
        }
        //watch out for accidentally setting feof on this in there
        if (source_eof) {
            break;
        }
    }
    return;
}

void match_code_to_token(scanner_data * s_p) {
    token t;
    
    
    // while we still have characters..
    int t_index = 0;
    short unsigned int line_buffer_len = strlen(s_p->line_buffer);
    while (s_p->line_index < line_buffer_len) { 
        switch (s_p->line_buffer[s_p->line_index]) {
            case ' ':
            case '\t':
            case '\n':
                s_p->line_index++;
                break;
            case '=':
                s_p->t[t_index] = match_error(s_p);
                t_index++;
                break;
            case '(':
            case ')':
            case ';':
            case ',':
            case ':':
            case '+':
            case '-':
                s_p->t[t_index] = match_symbol(s_p);
                t_index++;
                break;
            default:
                if (isalpha(s_p->line_buffer[s_p->line_index])) {
                    s_p->t[t_index] = match_alpha(s_p); 
                }
                else if (isdigit(s_p->line_buffer[s_p->line_index])) {
                    //s_p->t[t_index] = match_numeric(s_p);
                }
                else {
                    //s_p->t[t_index] = match_error(s_p);
                }
                t_index++;
                break;
        // if starts with symbol, go there..
        // if starts with alpha..
        // if starts with int..
    		//all characters non-alpha numeric should get sent
    		//allows to catch invalid characters in symbol match too
        }
    }
    s_p->t_index = t_index;
    return;
}

token match_alpha(scanner_data * sp) {
    token t;
    memset(t.buffer, '\0', MAX_SIZE);
    // get the index
    short unsigned int x = 0;
    int i = sp->line_index;
    // put this letter into the buffer
    if (isalpha(sp->line_buffer[i])) {
        strncpy(t.buffer + x, sp->line_buffer + i, 1);
        strcat(t.buffer, "\0");
        i++; x++;
    }
    // assign its token number
    t.token_number = ID;
    strcpy(t.token_type, "ID");
    // while we aren't at EOL,
    while (sp->line_buffer[i] != '\0' || sp->line_buffer[i] != '\n') {
    //      get another character
                
    //      if it's alphanumeric,
        if (isalnum(sp->line_buffer[i])) {
    //          put it in the buffer
            strncat(t.buffer + x, sp->line_buffer + i, 1);
            x++;
        }
        else {
    //          it's a terminal. break out
            break;
        }
        i++;
    }
    sp->line_index = i;
  
    // handle if it is begin
    char * burner_buffer = (char *) malloc(strlen(t.buffer) + 1);
    toUpper(t.buffer, burner_buffer);
    if (strcmp(burner_buffer, "BEGIN") == 0) {
        t.token_number = BEGIN;
        strcpy(t.token_type, "BEGIN");
    }
    // handle if it is end
    else if (strcmp(burner_buffer, "END") == 0) {
        t.token_number = END;
        strcpy(t.token_type, "END");
    }
    return t;
}

token match_numeric(scanner_data * sp) {
    token t;
    return t;
}

token match_symbol(scanner_data * sp)
{
    token t;
    memset(t.buffer, '\0', MAX_SIZE);
    strncat(t.buffer, sp->line_buffer + sp->line_index, 1);
	if (t.buffer[0] == '(')
	{
		t.token_number=6;
		strcpy(t.token_type,"LPAREN");
	}
	else if (t.buffer[0] == '(')
	{
		t.token_number=7;
		strcpy(t.token_type,"RPAREN");
	}	
	else if (t.buffer[0] == ';')
	{
		t.token_number=8;
		strcpy(t.token_type,"SEMICOLON");
	}
	else if (t.buffer[0] == ',')
	{
		t.token_number=9;
		strcpy(t.token_type,"COMMA");
	}
	else if (t.buffer[0] == ':')
	{
		//get next character from line buffer
		if (sp->line_buffer[sp->line_index+1] == '=')
		{
			t.token_number=10;
			strcpy(t.token_type,"ASSIGNOP");
            strncpy(t.buffer + 1, sp->line_buffer + sp->line_index + 1, 1);
            sp->line_index++;
		}
		else
		{
			//lexical error
			t = match_error(sp);
            sp->line_index--; //must do this because match_error shouldnt
		}	
	}
	else if (t.buffer[0] == '+')
	{
		t.token_number=11;
		strcpy(t.token_type,"PLUSOP");
	}
	else if (t.buffer[0] == '-')
	{
		//get next character from line buffer
		if (sp->line_buffer[sp->line_index+1] == '-')
		{
			//place rest of line into listing file
			//just skip it
			while (sp->line_buffer[sp->line_index] != '\n') {
                sp->line_index++;
            }
            t.token_number = -1; //ignore this in processing
		}
		else
		{	
			//putchar back
			t.token_number=12;
			strcpy(t.token_type,"MINUSOP");
		}
	}
	else
	{
		//lexical error or potentially whitespace
		t = match_error(sp);
	}
    sp->line_index++;
	return t;
}

token match_error(scanner_data * sp) {
    token t;
    t.token_number = ERROR;
    strcpy(t.token_type, "ERROR");
    strncpy(t.buffer, sp->line_buffer + sp->line_index, 1);
    sp->line_index++;
    return t;
}