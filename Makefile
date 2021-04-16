all: minishell program1 program2 program3

minishell: minishell.cpp
	g++ -o minishell minishell.cpp

program1: program1.cpp
	g++ -o program1 program1.cpp

program1: program2.cpp
	g++ -o program2 program2.cpp

program3: program3.cpp
	g++ -pthread -o program3 program3.cpp

clean:
	rm minishell program1 program2 program3