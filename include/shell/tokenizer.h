#ifndef TOKENIZER_H
#define TOKENIZER_H

#define MAX_TOKENS 128

struct token_list {
    int count;
    char *tokens[MAX_TOKENS];
};

int tokenize_line(char *line, struct token_list *out);
void free_tokens(struct token_list *t);

#endif /* TOKENIZER_H */