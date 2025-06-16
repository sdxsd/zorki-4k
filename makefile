##
# Zorki
#
# @file
# @version 0.1

CC = gcc
CFLAGS = -Wall -Wextra -Werror -g
NAME_LOADER = loader
NAME_INFECTOR = infector

CFILES_LOADER = \
		src/loader.c \
		src/lists.c \
		src/utils.c

CFILES_INFECTOR = \
		src/encoder.c \
		src/utils.c \
		src/lists.c \
		src/infection.c

HEADERS = \
	includes/lists.h \
	includes/infection.h \
	includes/utils.h

OFILES_LOADER = $(addprefix obj/,$(CFILES_LOADER:.c=.o))
OFILES_INFECTOR = $(addprefix obj/,$(CFILES_INFECTOR:.c=.o))

all: loader infector

obj/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

loader: $(OFILES_LOADER)
	$(CC) $(CFLAGS) $(OFILES_LOADER) -o $(NAME_LOADER)

infector: $(OFILES_INFECTOR)
	$(CC) $(CFLAGS) $(OFILES_INFECTOR) -o $(NAME_INFECTOR)

re: fclean all

fclean: clean
	@rm -f $(NAME_INFECTOR)
	@rm -f $(NAME_LOADER)
	@echo "DEEP CLEANING"

clean:
	@rm -rf obj/
	@echo "CLEANED UP"

.PHONY: all re fclean clean


# end
