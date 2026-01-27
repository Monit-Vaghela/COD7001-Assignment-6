#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include<stdio.h>
#include "shell/tokenizer.h"
#include "shell/utils.h"

int add_token(struct token_list * output, char *token)
{
    if(output->count == MAX_TOKENS)
    {
        printf("Tokenizer Error: Can't add token, max limit reached\n");
        return 1;
    }
    output->tokens[output->count] = token;
    output->count++;
    return 0;
}
int tokenize_line(char *line, struct token_list *output) {
    output->count = 0;
    int i = 0;

    while(line[i]!='\0')
    {
        while(line[i] == ' ')  // whitespace handling
        {
            i++;
        }

        if(line[i] == '|')   // operators handling 
        {
            add_token(output,strdup("|"));
            i++;
            continue;
        }
        else if(line[i] == '<')
        {
            add_token(output,strdup("<"));
            i++;
            continue;
        }
        else if(line[i] == '>')
        {
            if(line[i+1] == '>')
            {
                add_token(output,strdup(">>"));
                i+=2;
                continue;
            }
            else{
                add_token(output,strdup(">"));
                i++;
                continue;
            }
        }
        else if(line[i] == '&')
        {
            add_token(output,strdup("&"));
            i++;
            continue;
        }

        if(line[i] == '"')      // Double quotes handling
        {
            i++; 
            int start = i;
            while(line[i] != '"' && line[i] != '\0')
            {
                i++;
            }
            if(line[i] == '\0') // didnt find the closing quote 
            {
                printf("Tokenizer Error: Quotes were not closed\n");
                return 1;
            }
            int end = i;
            int length = end - start;
            char *substr = malloc(length + 1);  
            strncpy(substr, line + start, length);
            substr[length] = '\0';
            add_token(output,substr);
            if(line[i] == '"')
            {
                i++;
            }
            continue;
        }

        if(line[i] == '\'')     // Single quote handling
        {
            i++;
            int start = i;
            while(line[i] != '\'' && line[i] != '\0')
            {
                i++;
            } 
            if(line[i] == '\0') // didnt find the closing quote 
            {
                printf("Tokenizer Error: Quotes were not closed\n");
                return 1;
            }
            int end = i;
            int length = end - start;
            char *substr = malloc(length + 1);  
            strncpy(substr, line + start, length);
            substr[length] = '\0';
            add_token(output,substr);
            if(line[i] == '\'')
            {
                i++;
            }
            continue;
        }

        int start = i;         // normal word handling
        while(line[i] !='\0' && line[i]!= ' ' && line[i] != '>' && line[i] != '|' && line[i] != '&' && line[i] != '<' && line[i] != '"' && line[i] != '\'')
        {
            i++;
        }
        int end = i;
        if(start == end)
        {
            continue;
        }
        int length = end - start;
        char *substr = malloc(length + 1);  
        strncpy(substr, line + start, length);
        substr[length] = '\0';
        add_token(output,substr);
        continue;
    }
    return 0;
}
void free_tokens(struct token_list *t) {
    for (int i = 0; i < t->count; i++)
    {
        free(t->tokens[i]);
    }
    t->count = 0;
}