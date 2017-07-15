CXX:=					clang++
CXXFLAGS+=		-Weverything -std=c++14 -Wno-c++98-compat \
							-Wno-c++98-c++11-compat-pedantic					\
							-Wno-exit-time-destructors -Wno-padded		\
							-Wno-switch
LDFLAGS+=			-lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system

DEBUG:=				no
ifeq ($(DEBUG),no)
CXXFLAGS+=		-Werror -O2 -fomit-frame-pointer
else
CXXFLAGS+=		-g -O0
LDFLAGS+=			-rdynamic -g -fsanitize=address
endif

NAME:=				c8emu

SRC_FILES:=		main.cpp		\
							Screen.cpp	\
							Chip8.cpp		\
							CPU.cpp

SRC:=					$(addprefix src/, $(SRC_FILES))

OBJ:=					$(SRC:%.cpp=%.o)

$(NAME):		$(OBJ)
			$(CXX) $(LDFLAGS) $(OBJ) -o $(NAME)

all:			$(NAME)

clean:
			$(RM) $(OBJ)

fclean:			clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:			clean fclean re
