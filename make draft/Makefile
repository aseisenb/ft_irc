NAME     = ircserv
CC       = c++
DEP_FLAG = -MMD
C98      = -std=c++98
FLAGS    = -Wall -Wextra -Werror ${C98} ${DEP_FLAG} -g3
INCL     = -I includes
SRCS	=	$(addsuffix .cpp,		\
			$(addprefix srcs/,		\
			$(addprefix parsing/,	\
				init				\
				parsing				\
				input				\
			)						\
			$(addprefix server/,	\
				actions				\
				commands_channel	\
				commands			\
			)						\
			$(addprefix utils/,		\
				utils				\
				clear_data			\
			)))
CLAS	= 	$(addsuffix .cpp,		\
			$(addprefix classes/,	\
				User				\
				Channel				\
			))
BONUS	= 	$(addsuffix .cpp,		\					\
			$(addprefix srcs/,		\
			$(addprefix parsing/,	\
				parsing				\
			)))
MAIN_OBJ = srcs/main.o

OBJS    = ${SRCS:.cpp=.o} ${CLAS:.cpp=.o}

DEPS    = ${OBJS:.o=.d} ${MAIN_OBJ:.o=.d}

RM		:= rm -rf
RED		:= \033[0;31m
NC		:= \033[0m
LGREEN	:= \033[0;32m
CYAN	:= \033[0;36m

%.o: %.cpp
	@echo "\033[1A                                                              "
	@echo -n "\033[1A"
	@echo "${CYAN}Compiling $< ${NC}"
	@${CC} ${FLAGS} -o $@ -c $< ${INCL}

${NAME}: ${OBJS} ${MAIN_OBJ}
	@${CC} ${FLAGS} ${OBJS} ${MAIN_OBJ} ${INCL} -o $@ 
	@echo "${LGREEN}Successfully created ${NC}${CYAN}${NAME}${NC}${LGREEN}!${NC}"

all: ${NAME}

clean:
	@echo "${CYAN}Cleaned ${NAME}${NC}"
	@${RM} ${OBJS} ${MAIN_OBJ} ${DEPS}

fclean:
	@echo "${CYAN}FCleaned ${NAME}${NC}"
	@${RM} ${OBJS} ${NAME} ${MAIN_OBJ} ${DEPS}

re: fclean all

-include ${DEPS}

.PHONY: all clean fclean re bonus