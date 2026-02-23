
NAME := codexion

BUILD_DIR := .build

SRC_DIR := src

RM := rm -f

CC := cc

CFLAGS := -Wall -Werror -Wextra

FLAGS := -pthread -I. -g

DEPFLAGS := -MMD -MP

SRCS := $(addprefix $(SRC_DIR)/, \
	codexion.c\
	coder.c\
	dongle.c\
	monitor.c\
	parser.c\
	schedule.c\
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
	./$(NAME) 2 5000 1 1 1 2 1 fifo

debug: re
	gdb --args $(NAME) 2 5000 1 1 1 2 1 fifo

clean:
	$(RM) $(OBJS) $(DEPS)
	rm -rf $(BUILD_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)

