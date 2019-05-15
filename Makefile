TARGET = coconutman

LEXER_IN = cfg.l
LEXER_OUT = coconutman.lexer.c
PARSER_IN = cfg.y
PARSER_OUT = coconutman.parser.c

INCLUDE_DIR = include/

LIB_C_DIR = src/lib/
LIB_SRC = $(filter-out $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT),$(foreach dir,$(LIB_C_DIR),$(wildcard $(dir)*.c)))

SRC_DIR = src/
SRC = $(foreach dir,$(SRC_DIR),$(wildcard $(dir)*.c))

coconutman: $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o)
	$(CC) -o $(TARGET) $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o) $(LDFLAGS)

$(SRC_DIR)$(LEXER_OUT:.c=.o): $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT:.c=.h)
	$(CC) -I $(INCLUDE_DIR) -o $@ -c $< $(LDFLAGS)

$(SRC_DIR)$(PARSER_OUT:.c=.o): $(SRC_DIR)$(PARSER_OUT) $(SRC_DIR)$(LEXER_OUT:.c=.h)
	$(CC) -I $(INCLUDE_DIR) -o $@ -c $< $(LDFLAGS)

$(SRC_DIR)$(LEXER_OUT:.c=.h): $(SRC_DIR)$(LEXER_OUT)
$(SRC_DIR)$(PARSER_OUT:.c=.h): $(SRC_DIR)$(PARSER_OUT)

$(SRC_DIR)$(LEXER_OUT): $(SRC_DIR)$(LEXER_IN)
	flex -o $@ --header-file=$(@:.c=.h) $<

$(SRC_DIR)$(PARSER_OUT): $(SRC_DIR)$(PARSER_IN)
	bison -d -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -I $(INCLUDE_DIR) -o $@ -c $<

clean:
	rm -f $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.h) $(SRC_DIR)$(PARSER_OUT:.c=.h)
	rm -f $(TARGET)
