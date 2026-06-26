
NAME := codexion

BUILD_DIR := .build
SRC_DIR := src

RM := rm -f
CC := cc

CFLAGS := -Wall -Werror -Wextra
FLAGS := -pthread -Iinclude
DEBUG_FLAGS := -g -O0 #-fsanitize=thread
DEPFLAGS := -MMD -MP

SRCS := $(addprefix $(SRC_DIR)/, \
	codexion.c\
	coder.c\
	dongle.c\
	heapq.c\
	monitor.c\
	parsing.c\
	utils.c\
	init.c\
	)

OBJS := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRCS:.c=.o))
DEPS := $(patsubst %.o, %.d, $(OBJS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(FLAGS) $(OBJS) -o $(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(FLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

t: re
	$(CC) $(CFLAGS) $(FLAGS) $(OBJS) -o $(NAME)
	./$(NAME) 4 600 300 100 100 10 100 fifo

debug: $(OBJS)
	$(CC) $(FLAGS) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)
	./$(NAME) 4 600 300 100 100 10 100 fifo

gdb: $(OBJS)
	$(CC) $(CFLAGS) $(FLAGS) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)
	gdb --args $(NAME) 4 600 300 100 100 10 100 fifo

valgrind: $(OBJS)
	$(CC) $(CFLAGS) $(FLAGS) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all --tool=memcheck ./$(NAME) 4 600 300 100 100 10 100 fifo

clean:
	$(RM) $(OBJS) $(DEPS)
	rm -rf $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

test:
	bash tester.sh

test2:
	bash tester2.sh

re: fclean all

.PHONY: all clean fclean re test test2
-include $(DEPS)

