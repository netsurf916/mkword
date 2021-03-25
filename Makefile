all: mkword.cpp
	g++ -o mkword mkword.cpp

clean:
	-rm mkword

