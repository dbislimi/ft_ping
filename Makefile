# --- VÉRIFICATION DOCKER ---
ifeq ("$(wildcard /.dockerenv)","")
    $(error "🚨 ERREUR : Tu n'es pas dans Docker ! Lance ton conteneur avant de faire make.")
endif
# ---------------------------

NAME        = ft_ping

CC          = cc
CFLAGS      = -g3

SRC_DIR     = src
INC_DIR     = inc
OBJ_DIR     = .obj

SRCS_FILES  = ping.c utils.c

SRCS        = $(addprefix $(SRC_DIR)/, $(SRCS_FILES))
OBJS        = $(addprefix $(OBJ_DIR)/, $(SRCS_FILES:.c=.o))

INCLUDES    = -I$(INC_DIR)


all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "\033[32m[✓] $(NAME) créé avec succès\033[0m"


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)
	@echo "\033[33m[!] Objets supprimés\033[0m"

fclean: clean
	@rm -f $(NAME)
	@echo "\033[31m[!] $(NAME) supprimé\033[0m"

re: fclean all

.PHONY: all clean fclean re