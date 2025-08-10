NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincs

SRCS = main.cpp \
    	srcs/Server.cpp \
    	srcs/Channel.cpp \
    	srcs/Client.cpp \
    	srcs/ServerConfig.cpp \
		srcs/signal.cpp \
    	srcs/handler/handler.cpp \
    	srcs/handler/handle_join.cpp \
    	srcs/handler/handle_authentication.cpp \
    	srcs/handler/handle_topic.cpp \
    	srcs/handler/handle_kick.cpp \
    	srcs/handler/handle_pmsg.cpp \
    	srcs/handler/handle_mode.cpp \
    	srcs/handler/handle_mode_utils.cpp \
    	srcs/handler/handle_invite.cpp \
    	srcs/handler/handle_part.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^



clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME) 6667 pass

val: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --log-file=memory_exhaustion.log ./$(NAME) 6667 pass

.PHONY: all clean fclean re run
