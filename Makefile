TARGET = coconutman

LEXER_IN = cfg.l
LEXER_OUT = coconutman.lexer.c
PARSER_IN = cfg.y
PARSER_OUT = coconutman.parser.c

INCLUDE = -I CoCoNut-lib/include

LIB_C_DIR = src/lib/
LIB_SRC = $(foreach dir,$(LIB_C_DIR),$(wildcard $(dir)*.c))

SRC_DIR = src/
SRC = $(filter-out $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT),$(foreach dir,$(SRC_DIR),$(wildcard $(dir)*.c)))

LDFLAGS = -lcoconut -L CoCoNut-lib/bin

DEBUG = 

coconutman: $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o)
	$(CC) $(DEBUG) -o $(TARGET) $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o) $(LDFLAGS)

$(SRC_DIR)$(LEXER_OUT:.c=.o): $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT:.c=.h)
	$(CC) $(DEBUG) $(INCLUDE) -o $@ -c $< $(LDFLAGS)

$(SRC_DIR)$(PARSER_OUT:.c=.o): $(SRC_DIR)$(PARSER_OUT) $(SRC_DIR)$(LEXER_OUT:.c=.h)
	$(CC) $(DEBUG) $(INCLUDE) -o $@ -c $< $(LDFLAGS)

$(SRC_DIR)$(LEXER_OUT:.c=.h): $(SRC_DIR)$(LEXER_OUT)
$(SRC_DIR)$(PARSER_OUT:.c=.h): $(SRC_DIR)$(PARSER_OUT)

$(SRC_DIR)$(LEXER_OUT): $(SRC_DIR)$(LEXER_IN)
	flex -o $@ --header-file=$(@:.c=.h) $<

$(SRC_DIR)$(PARSER_OUT): $(SRC_DIR)$(PARSER_IN)
	bison -d -o $@ $<

%.o: %.c
	$(CC) $(DEBUG) $(CFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	rm -f $(LIB_SRC:.c=.o) $(SRC:.c=.o) $(SRC_DIR)$(LEXER_OUT) $(SRC_DIR)$(PARSER_OUT) $(SRC_DIR)$(LEXER_OUT:.c=.o) $(SRC_DIR)$(PARSER_OUT:.c=.o) $(SRC_DIR)$(LEXER_OUT:.c=.h) $(SRC_DIR)$(PARSER_OUT:.c=.h)
	rm -f $(TARGET)
