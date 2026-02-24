
NAME := codexion

BUILD_DIR := .build

SRC_DIR := src

RM := rm -f

CC := cc

CFLAGS := -Wall -Werror -Wextra

FLAGS := -pthread -I.

DEBUG_FLAGS := -g -fsanitize=thread

DEPFLAGS := -MMD -MP

SRCS := $(addprefix $(SRC_DIR)/, \
	codexion.c\
	coder.c\
	dongle.c\
	monitor.c\
	parser.c\
	utils.c\
	init.c\
	)

OBJS := $(subst $(SRC_DIR), $(BUILD_DIR), $(SRCS:.c=.o))
DEPS := $(patsubst %.o, %.d, $(OBJS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(FLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

t: re
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)
	./$(NAME) 4 2000 5 1.5 1 2 1 fifo

debug:
	$(CC) $(FLAGS) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)
	./$(NAME) 4 2000 5 1.5 1 2 1 fifo
# 	gdb --args $(NAME) 4 5000 1 1 1 2 1 fifo

clean:
	$(RM) $(OBJS) $(DEPS)
	rm -rf $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)

